#ifndef AMTLS_BEARSSL_CLIENT_H
#define AMTLS_BEARSSL_CLIENT_H

#include <stddef.h>

#include "backends/backend.h"

#define AMTLS_BEARSSL_ENTROPY_BYTES 32u

typedef int (*AmTLS_EntropyFill)(void *user,
                                 unsigned char *buffer,
                                 size_t length);

typedef struct AmTLS_BearSSLClientConfig {
    const char *server_name;
    AmTLS_EntropyFill entropy_fill;
    void *entropy_user;
} AmTLS_BearSSLClientConfig;

/* Bind a backend object to the BearSSL client implementation.
 * The config is consumed by backend->ops->init(); callers must keep it valid
 * until init returns. Secure entropy is mandatory; init fails closed without it.
 */
int amtls_bearssl_client_bind(AmTLS_Backend *backend,
                              const AmTLS_BearSSLClientConfig *config);

const char *amtls_bearssl_client_server_name(const AmTLS_Backend *backend);
int amtls_bearssl_client_last_error(const AmTLS_Backend *backend);

#endif
