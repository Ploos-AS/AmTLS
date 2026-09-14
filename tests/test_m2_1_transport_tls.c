#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <bearssl.h>

#include "backends/bearssl_client.h"
#include "platform/allocator.h"
#include "platform/transport.h"

typedef struct AnchorHolder {
    br_x509_decoder_context decoder;
    br_x509_trust_anchor anchor;
    unsigned char dn[1024];
    size_t dn_len;
    int overflow;
} AnchorHolder;

typedef struct SocketTransport {
    int fd;
} SocketTransport;

static void append_dn(void *ctx, const void *buf, size_t len)
{
    AnchorHolder *holder = (AnchorHolder *)ctx;

    if (holder->dn_len + len > sizeof(holder->dn)) {
        holder->overflow = 1;
        return;
    }
    memcpy(holder->dn + holder->dn_len, buf, len);
    holder->dn_len += len;
}

static unsigned char *read_file(const char *path, size_t *length)
{
    FILE *f;
    long n;
    unsigned char *buf;

    f = fopen(path, "rb");
    if (f == 0) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    n = ftell(f);
    if (n <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    buf = (unsigned char *)malloc((size_t)n);
    if (buf == 0) {
        fclose(f);
        return 0;
    }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *length = (size_t)n;
    return buf;
}

static int build_anchor(AnchorHolder *holder,
                        const unsigned char *der, size_t der_len)
{
    br_x509_pkey *pkey;

    memset(holder, 0, sizeof(*holder));
    br_x509_decoder_init(&holder->decoder, append_dn, holder);
    br_x509_decoder_push(&holder->decoder, der, der_len);
    if (holder->overflow || br_x509_decoder_last_error(&holder->decoder) != 0
            || !br_x509_decoder_isCA(&holder->decoder)) {
        return 0;
    }
    pkey = br_x509_decoder_get_pkey(&holder->decoder);
    if (pkey == 0 || holder->dn_len == 0) {
        return 0;
    }
    holder->anchor.dn.data = holder->dn;
    holder->anchor.dn.len = holder->dn_len;
    holder->anchor.flags = BR_X509_TA_CA;
    holder->anchor.pkey = *pkey;
    return 1;
}

static void *host_alloc(void *user, size_t size)
{
    (void)user;
    return malloc(size);
}

static void host_free(void *user, void *ptr, size_t size)
{
    (void)user;
    (void)size;
    free(ptr);
}

static int host_entropy(void *user, unsigned char *buffer, size_t length)
{
    int fd;
    size_t done;

    (void)user;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    done = 0;
    while (done < length) {
        ssize_t n = read(fd, buffer + done, length - done);
        if (n <= 0) {
            close(fd);
            return 0;
        }
        done += (size_t)n;
    }
    close(fd);
    return 1;
}

static long socket_read(void *user, void *buffer, size_t length)
{
    SocketTransport *st = (SocketTransport *)user;
    ssize_t n = recv(st->fd, buffer, length, 0);

    if (n < 0 && errno == EINTR) {
        return -1;
    }
    return (long)n;
}

static long socket_write(void *user, const void *buffer, size_t length)
{
    SocketTransport *st = (SocketTransport *)user;
    ssize_t n = send(st->fd, buffer, length, 0);

    if (n < 0 && errno == EINTR) {
        return -1;
    }
    return (long)n;
}

static int socket_close(void *user)
{
    SocketTransport *st = (SocketTransport *)user;
    int rc = 0;

    if (st->fd >= 0) {
        rc = close(st->fd);
        st->fd = -1;
    }
    return rc;
}

static int connect_loopback(unsigned short port)
{
    struct sockaddr_in sa;
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static void current_bearssl_time(uint32_t *days, uint32_t *seconds)
{
    time_t now = time(0);
    unsigned long long unix_seconds;

    assert(now >= 0);
    unix_seconds = (unsigned long long)now;
    *days = (uint32_t)(719528u + unix_seconds / 86400u);
    *seconds = (uint32_t)(unix_seconds % 86400u);
}

int main(int argc, char **argv)
{
    const char *server_name;
    const char *expected;
    unsigned long port_value;
    unsigned char *ca_der;
    size_t ca_len;
    AnchorHolder anchor;
    SocketTransport socket_state;
    AmTLS_Transport transport;
    AmTLS_Allocator allocator;
    AmTLS_BearSSLClientConfig config;
    AmTLS_Backend backend;
    AmTLS_BackendResult result;
    uint32_t validation_days;
    uint32_t validation_seconds;
    int last_error;
    unsigned steps;

    assert(argc == 5);
    port_value = strtoul(argv[1], 0, 10);
    assert(port_value > 0 && port_value <= 65535u);
    server_name = argv[2];
    expected = argv[4];

    ca_der = read_file(argv[3], &ca_len);
    assert(ca_der != 0);
    assert(build_anchor(&anchor, ca_der, ca_len));

    socket_state.fd = connect_loopback((unsigned short)port_value);
    assert(socket_state.fd >= 0);

    memset(&transport, 0, sizeof(transport));
    transport.user = &socket_state;
    transport.read = socket_read;
    transport.write = socket_write;
    transport.close = socket_close;

    memset(&allocator, 0, sizeof(allocator));
    allocator.alloc = host_alloc;
    allocator.free = host_free;

    current_bearssl_time(&validation_days, &validation_seconds);
    memset(&config, 0, sizeof(config));
    config.server_name = server_name;
    config.entropy_fill = host_entropy;
    config.trust_anchors = &anchor.anchor;
    config.trust_anchor_count = 1;
    config.validation_days = validation_days;
    config.validation_seconds = validation_seconds;

    memset(&backend, 0, sizeof(backend));
    assert(amtls_bearssl_client_bind(&backend, &config) == 0);
    assert(backend.ops != 0);
    assert(backend.ops->init(&backend, &transport, &allocator) == 0);

    result = AMTLS_BACKEND_WANT_READ;
    for (steps = 0; steps < 10000u; steps++) {
        result = backend.ops->handshake(&backend);
        if (result == AMTLS_BACKEND_OK || result == AMTLS_BACKEND_ERROR
                || result == AMTLS_BACKEND_CLOSED) {
            break;
        }
    }
    assert(steps < 10000u);
    last_error = amtls_bearssl_client_last_error(&backend);
    fprintf(stderr, "transport-handshake result=%d error=%d steps=%u\n",
            (int)result, last_error, steps);

    if (strcmp(expected, "ok") == 0) {
        assert(result == AMTLS_BACKEND_OK);
        assert(last_error == BR_ERR_OK);
        puts("PASS: transport TLS handshake trusted chain + matching hostname");
    } else if (strcmp(expected, "bad-name") == 0) {
        assert(result == AMTLS_BACKEND_ERROR);
        assert(last_error == BR_ERR_X509_BAD_SERVER_NAME);
        puts("PASS: transport TLS handshake rejects hostname mismatch");
    } else if (strcmp(expected, "unknown-ca") == 0) {
        assert(result == AMTLS_BACKEND_ERROR);
        assert(last_error == BR_ERR_X509_NOT_TRUSTED);
        puts("PASS: transport TLS handshake rejects unknown CA");
    } else {
        assert(!"unknown expected result");
    }

    backend.ops->destroy(&backend);
    assert(allocator.stats.current_bytes == 0);
    socket_close(&socket_state);
    free(ca_der);
    return 0;
}
