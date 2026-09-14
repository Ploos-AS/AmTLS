#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "backends/bearssl_client.h"
#include "platform/allocator.h"
#include "platform/transport.h"

typedef struct CaptureTransport {
    unsigned char output[4096];
    size_t output_len;
} CaptureTransport;

/* Structurally present anchor for pre-certificate ClientHello tests. The
 * X.509 path is qualified separately; this fixture is never used to validate
 * a peer certificate. */
static const br_x509_trust_anchor test_anchor = { 0 };

static long capture_read(void *user, void *buffer, size_t length)
{
    (void)user;
    (void)buffer;
    (void)length;
    return -1;
}

static long capture_write(void *user, const void *buffer, size_t length)
{
    CaptureTransport *capture = (CaptureTransport *)user;
    size_t room = sizeof(capture->output) - capture->output_len;
    size_t n = length < room ? length : room;

    memcpy(capture->output + capture->output_len, buffer, n);
    capture->output_len += n;
    return (long)n;
}

static int capture_close(void *user)
{
    (void)user;
    return 0;
}

static void *test_alloc(void *user, size_t size)
{
    (void)user;
    return malloc(size);
}

static void test_free(void *user, void *ptr, size_t size)
{
    (void)user;
    (void)size;
    free(ptr);
}

static int deterministic_entropy(void *user,
                                 unsigned char *buffer,
                                 size_t length)
{
    size_t i;
    unsigned char seed = *(const unsigned char *)user;

    for (i = 0; i < length; i++) {
        buffer[i] = (unsigned char)(seed + (unsigned char)i);
    }
    return 1;
}

static int contains_bytes(const unsigned char *haystack, size_t haystack_len,
                          const unsigned char *needle, size_t needle_len)
{
    size_t i;

    if (needle_len == 0 || haystack_len < needle_len) {
        return 0;
    }
    for (i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(haystack + i, needle, needle_len) == 0) {
            return 1;
        }
    }
    return 0;
}

static void setup_allocator(AmTLS_Allocator *allocator)
{
    memset(allocator, 0, sizeof(*allocator));
    allocator->alloc = test_alloc;
    allocator->free = test_free;
}

static void setup_transport(AmTLS_Transport *transport,
                            CaptureTransport *capture)
{
    memset(capture, 0, sizeof(*capture));
    memset(transport, 0, sizeof(*transport));
    transport->user = capture;
    transport->read = capture_read;
    transport->write = capture_write;
    transport->close = capture_close;
}

static void test_entropy_is_mandatory(void)
{
    AmTLS_Backend backend;
    AmTLS_BearSSLClientConfig config;
    AmTLS_Allocator allocator;
    AmTLS_Transport transport;
    CaptureTransport capture;

    memset(&backend, 0, sizeof(backend));
    memset(&config, 0, sizeof(config));
    setup_allocator(&allocator);
    setup_transport(&transport, &capture);

    config.server_name = "example.com";
    config.trust_anchors = &test_anchor;
    config.trust_anchor_count = 1;
    assert(amtls_bearssl_client_bind(&backend, &config) == 0);
    assert(backend.ops->init(&backend, &transport, &allocator) != 0);
    assert(backend.state == 0);
    assert(allocator.stats.current_bytes == 0);
    assert(capture.output_len == 0);
}

static void test_trust_anchor_is_mandatory(void)
{
    unsigned char seed = 0x22;
    AmTLS_Backend backend;
    AmTLS_BearSSLClientConfig config;
    AmTLS_Allocator allocator;
    AmTLS_Transport transport;
    CaptureTransport capture;

    memset(&backend, 0, sizeof(backend));
    memset(&config, 0, sizeof(config));
    setup_allocator(&allocator);
    setup_transport(&transport, &capture);

    config.server_name = "example.com";
    config.entropy_fill = deterministic_entropy;
    config.entropy_user = &seed;
    assert(amtls_bearssl_client_bind(&backend, &config) == 0);
    assert(backend.ops->init(&backend, &transport, &allocator) != 0);
    assert(backend.state == 0);
    assert(allocator.stats.current_bytes == 0);
    assert(capture.output_len == 0);
}

static void test_tls12_clienthello_with_sni(void)
{
    static const unsigned char host[] = "example.com";
    unsigned char seed = 0x31;
    AmTLS_Backend backend;
    AmTLS_BearSSLClientConfig config;
    AmTLS_Allocator allocator;
    AmTLS_Transport transport;
    CaptureTransport capture;
    AmTLS_BackendResult result;

    memset(&backend, 0, sizeof(backend));
    memset(&config, 0, sizeof(config));
    setup_allocator(&allocator);
    setup_transport(&transport, &capture);

    config.server_name = (const char *)host;
    config.entropy_fill = deterministic_entropy;
    config.entropy_user = &seed;
    config.trust_anchors = &test_anchor;
    config.trust_anchor_count = 1;

    assert(amtls_bearssl_client_bind(&backend, &config) == 0);
    assert(backend.ops != 0);
    assert(strcmp(backend.ops->name, "BearSSL") == 0);
    assert(backend.ops->init(&backend, &transport, &allocator) == 0);
    assert(allocator.stats.current_bytes > 0);
    assert(strcmp(amtls_bearssl_client_server_name(&backend),
                  (const char *)host) == 0);

    result = backend.ops->handshake(&backend);
    assert(result == AMTLS_BACKEND_WANT_WRITE);
    assert(capture.output_len > 11);

    assert(capture.output[0] == 0x16);
    assert(capture.output[5] == 0x01);
    assert(capture.output[9] == 0x03);
    assert(capture.output[10] == 0x03);
    assert(contains_bytes(capture.output, capture.output_len,
                          host, sizeof(host) - 1));

    backend.ops->destroy(&backend);
    assert(backend.state == 0);
    assert(allocator.stats.current_bytes == 0);
}

int main(void)
{
    test_entropy_is_mandatory();
    test_trust_anchor_is_mandatory();
    test_tls12_clienthello_with_sni();
    return 0;
}
