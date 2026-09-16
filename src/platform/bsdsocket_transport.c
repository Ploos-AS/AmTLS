#include "platform/bsdsocket_transport.h"

#ifdef __amigaos__
/* The AmigaOS NDK socket prototypes use ssize_t but do not make its
 * definition visible through proto/bsdsocket.h on every Bebbo setup. */
#include <sys/types.h>
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
#ifdef __amigaos__
    /* Some classic NDK variants declare send() with a non-const buffer.
     * send() does not modify the payload; keep the generic transport API
     * const-correct and isolate the legacy prototype mismatch here. */
    rc = (long)send(state->socket_fd, (void *)buffer, length, 0);
#else
    rc = (long)send(state->socket_fd, buffer, length, 0);
#endif
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
