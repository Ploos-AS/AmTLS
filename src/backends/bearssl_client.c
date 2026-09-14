#include "backends/bearssl_client.h"

#include <bearssl.h>
#include <string.h>

#include "backends/handshake_pump.h"
#include "platform/allocator.h"
#include "platform/transport.h"

typedef struct AmTLS_BearSSLState {
    br_ssl_client_context client;
    br_x509_minimal_context x509;
    AmTLS_Allocator *allocator;
    AmTLS_Transport *transport;
    AmTLS_HandshakePump pump;
    unsigned char *iobuf;
    size_t iobuf_size;
    char *server_name;
    size_t server_name_size;
} AmTLS_BearSSLState;

static AmTLS_EngineState bearssl_engine_state(void *engine)
{
    br_ssl_client_context *client = (br_ssl_client_context *)engine;
    unsigned state;

    if (client == 0) {
        return AMTLS_ENGINE_ERROR;
    }

    state = br_ssl_engine_current_state(&client->eng);
    if ((state & BR_SSL_CLOSED) != 0) {
        return br_ssl_engine_last_error(&client->eng) == BR_ERR_OK
            ? AMTLS_ENGINE_CLOSED : AMTLS_ENGINE_ERROR;
    }
    if ((state & BR_SSL_SENDREC) != 0) {
        return AMTLS_ENGINE_SEND_RECORD;
    }
    if ((state & BR_SSL_RECVREC) != 0) {
        return AMTLS_ENGINE_RECV_RECORD;
    }
    if ((state & (BR_SSL_SENDAPP | BR_SSL_RECVAPP)) != 0) {
        return AMTLS_ENGINE_APPLICATION;
    }
    return AMTLS_ENGINE_ERROR;
}

static unsigned char *bearssl_send_buffer(void *engine, size_t *length)
{
    br_ssl_client_context *client = (br_ssl_client_context *)engine;
    return br_ssl_engine_sendrec_buf(&client->eng, length);
}

static void bearssl_send_ack(void *engine, size_t length)
{
    br_ssl_client_context *client = (br_ssl_client_context *)engine;
    br_ssl_engine_sendrec_ack(&client->eng, length);
}

static unsigned char *bearssl_recv_buffer(void *engine, size_t *length)
{
    br_ssl_client_context *client = (br_ssl_client_context *)engine;
    return br_ssl_engine_recvrec_buf(&client->eng, length);
}

static void bearssl_recv_ack(void *engine, size_t length)
{
    br_ssl_client_context *client = (br_ssl_client_context *)engine;
    br_ssl_engine_recvrec_ack(&client->eng, length);
}

static const AmTLS_HandshakeEngineOps bearssl_engine_ops = {
    bearssl_engine_state,
    bearssl_send_buffer,
    bearssl_send_ack,
    bearssl_recv_buffer,
    bearssl_recv_ack
};

static void bearssl_state_free(AmTLS_BearSSLState *state)
{
    AmTLS_Allocator *allocator;

    if (state == 0) {
        return;
    }
    allocator = state->allocator;
    if (allocator == 0) {
        return;
    }
    if (state->server_name != 0) {
        memset(state->server_name, 0, state->server_name_size);
        amtls_allocator_free(allocator, state->server_name,
                             state->server_name_size);
    }
    if (state->iobuf != 0) {
        memset(state->iobuf, 0, state->iobuf_size);
        amtls_allocator_free(allocator, state->iobuf, state->iobuf_size);
    }
    memset(state, 0, sizeof(*state));
    amtls_allocator_free(allocator, state, sizeof(*state));
}

static int bearssl_init(AmTLS_Backend *backend,
                        AmTLS_Transport *transport,
                        AmTLS_Allocator *allocator)
{
    const AmTLS_BearSSLClientConfig *config;
    AmTLS_BearSSLState *state;
    unsigned char entropy[AMTLS_BEARSSL_ENTROPY_BYTES];
    size_t server_name_len;

    if (backend == 0 || transport == 0 || allocator == 0
            || allocator->alloc == 0 || allocator->free == 0
            || transport->read == 0 || transport->write == 0
            || backend->state != 0 || backend->init_config == 0) {
        return -1;
    }

    config = (const AmTLS_BearSSLClientConfig *)backend->init_config;
    if (config->server_name == 0 || config->server_name[0] == '\0'
            || config->entropy_fill == 0
            || config->trust_anchors == 0
            || config->trust_anchor_count == 0
            || config->validation_days == 0
            || config->validation_seconds > 86400u) {
        return -1;
    }
    server_name_len = strlen(config->server_name);
    if (server_name_len > 253u) {
        return -1;
    }

    state = (AmTLS_BearSSLState *)amtls_allocator_alloc(allocator,
                                                        sizeof(*state));
    if (state == 0) {
        return -1;
    }
    memset(state, 0, sizeof(*state));
    state->allocator = allocator;
    state->transport = transport;

    state->server_name_size = server_name_len + 1u;
    state->server_name = (char *)amtls_allocator_alloc(
        allocator, state->server_name_size);
    if (state->server_name == 0) {
        bearssl_state_free(state);
        return -1;
    }
    memcpy(state->server_name, config->server_name, state->server_name_size);

    state->iobuf_size = BR_SSL_BUFSIZE_MONO;
    state->iobuf = (unsigned char *)amtls_allocator_alloc(allocator,
                                                          state->iobuf_size);
    if (state->iobuf == 0) {
        bearssl_state_free(state);
        return -1;
    }

    memset(entropy, 0, sizeof(entropy));
    if (!config->entropy_fill(config->entropy_user, entropy, sizeof(entropy))) {
        memset(entropy, 0, sizeof(entropy));
        bearssl_state_free(state);
        return -1;
    }

    br_ssl_client_init_full(&state->client, &state->x509,
                            config->trust_anchors,
                            config->trust_anchor_count);
    br_x509_minimal_set_time(&state->x509,
                             config->validation_days,
                             config->validation_seconds);
    br_ssl_engine_set_versions(&state->client.eng, BR_TLS12, BR_TLS12);
    br_ssl_engine_set_buffer(&state->client.eng,
                             state->iobuf, state->iobuf_size, 0);
    br_ssl_engine_inject_entropy(&state->client.eng, entropy, sizeof(entropy));
    memset(entropy, 0, sizeof(entropy));

    if (!br_ssl_client_reset(&state->client, state->server_name, 0)) {
        bearssl_state_free(state);
        return -1;
    }

    state->pump.engine = &state->client;
    state->pump.engine_ops = &bearssl_engine_ops;
    state->pump.transport = transport;
    backend->state = state;
    return 0;
}

static AmTLS_BackendResult bearssl_handshake(AmTLS_Backend *backend)
{
    AmTLS_BearSSLState *state;

    if (backend == 0 || backend->state == 0) {
        return AMTLS_BACKEND_ERROR;
    }
    state = (AmTLS_BearSSLState *)backend->state;
    return amtls_handshake_pump_step(&state->pump);
}

static long bearssl_read(AmTLS_Backend *backend, void *buffer, size_t length)
{
    AmTLS_BearSSLState *state;
    unsigned char *src;
    size_t available;
    size_t n;

    if (backend == 0 || backend->state == 0 || buffer == 0 || length == 0) {
        return -1;
    }
    state = (AmTLS_BearSSLState *)backend->state;
    src = br_ssl_engine_recvapp_buf(&state->client.eng, &available);
    if (src == 0 || available == 0) {
        return 0;
    }
    n = length < available ? length : available;
    memcpy(buffer, src, n);
    br_ssl_engine_recvapp_ack(&state->client.eng, n);
    return (long)n;
}

static long bearssl_write(AmTLS_Backend *backend,
                          const void *buffer,
                          size_t length)
{
    AmTLS_BearSSLState *state;
    unsigned char *dst;
    size_t available;
    size_t n;

    if (backend == 0 || backend->state == 0 || buffer == 0 || length == 0) {
        return -1;
    }
    state = (AmTLS_BearSSLState *)backend->state;
    dst = br_ssl_engine_sendapp_buf(&state->client.eng, &available);
    if (dst == 0 || available == 0) {
        return 0;
    }
    n = length < available ? length : available;
    memcpy(dst, buffer, n);
    br_ssl_engine_sendapp_ack(&state->client.eng, n);
    br_ssl_engine_flush(&state->client.eng, 0);
    return (long)n;
}

static void bearssl_destroy(AmTLS_Backend *backend)
{
    if (backend == 0) {
        return;
    }
    bearssl_state_free((AmTLS_BearSSLState *)backend->state);
    backend->state = 0;
    backend->init_config = 0;
}

static const AmTLS_BackendOps bearssl_backend_ops = {
    "BearSSL",
    bearssl_init,
    bearssl_handshake,
    bearssl_read,
    bearssl_write,
    bearssl_destroy
};

int amtls_bearssl_client_bind(AmTLS_Backend *backend,
                              const AmTLS_BearSSLClientConfig *config)
{
    if (backend == 0 || config == 0 || backend->state != 0) {
        return -1;
    }
    backend->ops = &bearssl_backend_ops;
    backend->init_config = config;
    return 0;
}

int amtls_bearssl_verify_chain(const char *server_name,
                               const br_x509_trust_anchor *trust_anchors,
                               size_t trust_anchor_count,
                               const unsigned char *const *certs,
                               const size_t *cert_lengths,
                               size_t cert_count,
                               uint32_t validation_days,
                               uint32_t validation_seconds)
{
    br_x509_minimal_context x509;
    const br_x509_class **xc;
    size_t i;

    if (server_name == 0 || server_name[0] == '\0'
            || trust_anchors == 0 || trust_anchor_count == 0
            || certs == 0 || cert_lengths == 0 || cert_count == 0
            || validation_days == 0 || validation_seconds > 86400u) {
        return BR_ERR_X509_INVALID_VALUE;
    }

    br_x509_minimal_init_full(&x509, trust_anchors, trust_anchor_count);
    br_x509_minimal_set_time(&x509, validation_days, validation_seconds);
    xc = &x509.vtable;
    (*xc)->start_chain(xc, server_name);
    for (i = 0; i < cert_count; i++) {
        if (certs[i] == 0 || cert_lengths[i] == 0
                || cert_lengths[i] > 0xFFFFFFFFu) {
            return BR_ERR_X509_INVALID_VALUE;
        }
        (*xc)->start_cert(xc, (uint32_t)cert_lengths[i]);
        (*xc)->append(xc, certs[i], cert_lengths[i]);
        (*xc)->end_cert(xc);
    }
    return (int)(*xc)->end_chain(xc);
}

const char *amtls_bearssl_client_server_name(const AmTLS_Backend *backend)
{
    const AmTLS_BearSSLState *state;

    if (backend == 0 || backend->state == 0) {
        return 0;
    }
    state = (const AmTLS_BearSSLState *)backend->state;
    return br_ssl_engine_get_server_name(&state->client.eng);
}

int amtls_bearssl_client_last_error(const AmTLS_Backend *backend)
{
    const AmTLS_BearSSLState *state;

    if (backend == 0 || backend->state == 0) {
        return -1;
    }
    state = (const AmTLS_BearSSLState *)backend->state;
    return br_ssl_engine_last_error(&state->client.eng);
}
