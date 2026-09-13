#ifndef AMTLS_TRANSPORT_H
#define AMTLS_TRANSPORT_H

#include <stddef.h>

typedef long (*AmTLS_TransportRead)(void *user, void *buffer, size_t length);
typedef long (*AmTLS_TransportWrite)(void *user, const void *buffer, size_t length);
typedef int (*AmTLS_TransportClose)(void *user);

typedef struct AmTLS_Transport {
    void *user;
    AmTLS_TransportRead read;
    AmTLS_TransportWrite write;
    AmTLS_TransportClose close;
} AmTLS_Transport;

#endif
