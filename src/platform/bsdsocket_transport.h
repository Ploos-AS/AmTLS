#ifndef AMTLS_BSDSOCKET_TRANSPORT_H
#define AMTLS_BSDSOCKET_TRANSPORT_H

#include "platform/transport.h"

typedef struct AmTLS_BSDSocketTransport {
    int socket_fd;
    int owns_socket;
} AmTLS_BSDSocketTransport;

/* Bind an already-connected bsdsocket-compatible socket to AmTLS. */
int amtls_bsdsocket_transport_init(AmTLS_BSDSocketTransport *state,
                                   AmTLS_Transport *transport,
                                   int socket_fd,
                                   int owns_socket);

#endif
