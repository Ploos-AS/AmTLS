#ifndef AMTLS_BACKEND_H
#define AMTLS_BACKEND_H

#include <stddef.h>

struct AmTLS_Backend;
struct AmTLS_Transport;
struct AmTLS_Allocator;

typedef enum AmTLS_BackendResult {
    AMTLS_BACKEND_OK = 0,
    AMTLS_BACKEND_WANT_READ = 1,
    AMTLS_BACKEND_WANT_WRITE = 2,
    AMTLS_BACKEND_CLOSED = 3,
    AMTLS_BACKEND_ERROR = -1
} AmTLS_BackendResult;

typedef struct AmTLS_BackendOps {
    const char *name;
    int (*init)(struct AmTLS_Backend *backend,
                struct AmTLS_Transport *transport,
                struct AmTLS_Allocator *allocator);
    AmTLS_BackendResult (*handshake)(struct AmTLS_Backend *backend);
    long (*read)(struct AmTLS_Backend *backend, void *buffer, size_t length);
    long (*write)(struct AmTLS_Backend *backend, const void *buffer, size_t length);
    void (*destroy)(struct AmTLS_Backend *backend);
} AmTLS_BackendOps;

typedef struct AmTLS_Backend {
    const AmTLS_BackendOps *ops;
    void *state;
} AmTLS_Backend;

#endif
