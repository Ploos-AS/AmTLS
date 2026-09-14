#include "backends/handshake_pump.h"

static int pump_valid(const AmTLS_HandshakePump *pump)
{
    return pump != 0
        && pump->engine != 0
        && pump->engine_ops != 0
        && pump->transport != 0;
}

static AmTLS_BackendResult result_for_state(AmTLS_EngineState state)
{
    switch (state) {
    case AMTLS_ENGINE_APPLICATION:
        return AMTLS_BACKEND_OK;
    case AMTLS_ENGINE_CLOSED:
        return AMTLS_BACKEND_CLOSED;
    case AMTLS_ENGINE_ERROR:
        return AMTLS_BACKEND_ERROR;
    case AMTLS_ENGINE_SEND_RECORD:
        return AMTLS_BACKEND_WANT_WRITE;
    case AMTLS_ENGINE_RECV_RECORD:
        return AMTLS_BACKEND_WANT_READ;
    }
    return AMTLS_BACKEND_ERROR;
}

AmTLS_BackendResult amtls_handshake_pump_step(AmTLS_HandshakePump *pump)
{
    AmTLS_EngineState state;
    unsigned char *buffer;
    size_t length;
    long n;

    if (!pump_valid(pump) || pump->engine_ops->state == 0) {
        return AMTLS_BACKEND_ERROR;
    }

    state = pump->engine_ops->state(pump->engine);

    switch (state) {
    case AMTLS_ENGINE_APPLICATION:
    case AMTLS_ENGINE_CLOSED:
    case AMTLS_ENGINE_ERROR:
        return result_for_state(state);

    case AMTLS_ENGINE_SEND_RECORD:
        if (pump->engine_ops->send_buffer == 0
                || pump->engine_ops->send_ack == 0
                || pump->transport->write == 0) {
            return AMTLS_BACKEND_ERROR;
        }
        length = 0;
        buffer = pump->engine_ops->send_buffer(pump->engine, &length);
        if (buffer == 0 || length == 0) {
            return AMTLS_BACKEND_ERROR;
        }
        n = pump->transport->write(pump->transport->user, buffer, length);
        if (n < 0) {
            return AMTLS_BACKEND_WANT_WRITE;
        }
        if (n == 0) {
            return AMTLS_BACKEND_CLOSED;
        }
        pump->engine_ops->send_ack(pump->engine, (size_t)n);
        return result_for_state(pump->engine_ops->state(pump->engine));

    case AMTLS_ENGINE_RECV_RECORD:
        if (pump->engine_ops->recv_buffer == 0
                || pump->engine_ops->recv_ack == 0
                || pump->transport->read == 0) {
            return AMTLS_BACKEND_ERROR;
        }
        length = 0;
        buffer = pump->engine_ops->recv_buffer(pump->engine, &length);
        if (buffer == 0 || length == 0) {
            return AMTLS_BACKEND_ERROR;
        }
        n = pump->transport->read(pump->transport->user, buffer, length);
        if (n < 0) {
            return AMTLS_BACKEND_WANT_READ;
        }
        if (n == 0) {
            return AMTLS_BACKEND_CLOSED;
        }
        pump->engine_ops->recv_ack(pump->engine, (size_t)n);
        return result_for_state(pump->engine_ops->state(pump->engine));
    }

    return AMTLS_BACKEND_ERROR;
}
