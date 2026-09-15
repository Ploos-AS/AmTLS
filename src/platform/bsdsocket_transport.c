#include "platform/bsdsocket_transport.h"

#ifdef __amigaos__
#include <proto/bsdsocket.h>
#else
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

static long bsdsocket_read(void *user, void *buffer, size_t length)
{
    AmTLS_BSDSocketTransport *state = (AmTLS_BSDSocketTransport *)user;
    long rc;

    if (state == NULL || state->socket_fd < 0 || buffer == NULL) {
        return -1;
    }
    rc = (long)recv(state->socket_fd, buffer, length, 0);
    return rc;
}

static long bsdsocket_write(void *user, const void *buffer, size_t length)
{
    AmTLS_BSDSocketTransport *state = (AmTLS_BSDSocketTransport *)user;
    long rc;

    if (state == NULL || state->socket_fd < 0 || buffer == NULL) {
        return -1;
    }
    rc = (long)send(state->socket_fd, buffer, length, 0);
    return rc;
}

static int bsdsocket_close(void *user)
{
    AmTLS_BSDSocketTransport *state = (AmTLS_BSDSocketTransport *)user;
    int rc = 0;

    if (state == NULL) {
        return -1;
    }
    if (state->socket_fd >= 0 && state->owns_socket) {
#ifdef __amigaos__
        rc = CloseSocket(state->socket_fd);
#else
        rc = close(state->socket_fd);
#endif
    }
    state->socket_fd = -1;
    return rc;
}

int amtls_bsdsocket_transport_init(AmTLS_BSDSocketTransport *state,
                                   AmTLS_Transport *transport,
                                   int socket_fd,
                                   int owns_socket)
{
    if (state == NULL || transport == NULL || socket_fd < 0) {
        return -1;
    }

    state->socket_fd = socket_fd;
    state->owns_socket = owns_socket ? 1 : 0;
    transport->user = state;
    transport->read = bsdsocket_read;
    transport->write = bsdsocket_write;
    transport->close = bsdsocket_close;
    return 0;
}
