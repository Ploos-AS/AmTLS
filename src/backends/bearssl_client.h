#ifndef AMTLS_BEARSSL_CLIENT_H
#define AMTLS_BEARSSL_CLIENT_H

#include <stddef.h>
#include <bearssl.h>

#include "backends/backend.h"

#define AMTLS_BEARSSL_ENTROPY_BYTES 32u

typedef int (*AmTLS_EntropyFill)(void *user,
                                 unsigned char *buffer,
                                 size_t length);

typedef struct AmTLS_BearSSLClientConfig {
    const char *server_name;
    AmTLS_EntropyFill entropy_fill;
    void *entropy_user;
    const br_x509_trust_anchor *trust_anchors;
    size_t trust_anchor_count;
} AmTLS_BearSSLClientConfig;

/* Bind a backend object to the BearSSL client implementation.
 * The config is consumed by backend->ops->init(); callers must keep it valid
 * until init returns. Secure entropy and at least one trust anchor are
 * mandatory; init fails closed without either.
 */
int amtls_bearssl_client_bind(AmTLS_Backend *backend,
                              const AmTLS_BearSSLClientConfig *config);

const char *amtls_bearssl_client_server_name(const AmTLS_Backend *backend);
int amtls_bearssl_client_last_error(const AmTLS_Backend *backend);

#endif
