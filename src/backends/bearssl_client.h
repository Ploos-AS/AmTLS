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
    uint32_t validation_days;
    uint32_t validation_seconds;
} AmTLS_BearSSLClientConfig;

/* Bind a backend object to the BearSSL client implementation.
 * The config is consumed by backend->ops->init(); callers must keep it valid
 * until init returns. Secure entropy, at least one trust anchor and an
 * explicit certificate-validation time are mandatory; init fails closed
 * without them. validation_days uses BearSSL's epoch (1970-01-01 = 719528).
 */
int amtls_bearssl_client_bind(AmTLS_Backend *backend,
                              const AmTLS_BearSSLClientConfig *config);

/* Validate a DER certificate chain with the same BearSSL minimal X.509 engine
 * used by the client binding. Certificates are supplied in TLS order: end
 * entity first, then intermediates. This exposes BearSSL end_chain() return
 * semantics: 0 means successful validation; failures are returned as non-zero
 * BearSSL X.509 error codes such as BR_ERR_X509_BAD_SERVER_NAME or
 * BR_ERR_X509_NOT_TRUSTED. The server name and explicit validation time are
 * mandatory so hostname and validity checks cannot be accidentally bypassed.
 */
int amtls_bearssl_verify_chain(const char *server_name,
                               const br_x509_trust_anchor *trust_anchors,
                               size_t trust_anchor_count,
                               const unsigned char *const *certs,
                               const size_t *cert_lengths,
                               size_t cert_count,
                               uint32_t validation_days,
                               uint32_t validation_seconds);

const char *amtls_bearssl_client_server_name(const AmTLS_Backend *backend);
int amtls_bearssl_client_last_error(const AmTLS_Backend *backend);

#endif
