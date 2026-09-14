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

/* Validate a DER certificate chain with the same BearSSL minimal X.509 engine
 * used by the client binding. Certificates are supplied in TLS order: end
 * entity first, then intermediates. Returns BR_ERR_X509_OK (0) on success or
 * the BearSSL X.509 error code on failure. The server name is mandatory so
 * hostname verification cannot be accidentally bypassed.
 */
int amtls_bearssl_verify_chain(const char *server_name,
                               const br_x509_trust_anchor *trust_anchors,
                               size_t trust_anchor_count,
                               const unsigned char *const *certs,
                               const size_t *cert_lengths,
                               size_t cert_count);

const char *amtls_bearssl_client_server_name(const AmTLS_Backend *backend);
int amtls_bearssl_client_last_error(const AmTLS_Backend *backend);

#endif
