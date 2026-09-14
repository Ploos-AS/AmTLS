#ifndef AMTLS_HANDSHAKE_PUMP_H
#define AMTLS_HANDSHAKE_PUMP_H

#include <stddef.h>

#include "backends/backend.h"
#include "platform/transport.h"

typedef enum AmTLS_EngineState {
    AMTLS_ENGINE_CLOSED = 0,
    AMTLS_ENGINE_SEND_RECORD = 1,
    AMTLS_ENGINE_RECV_RECORD = 2,
    AMTLS_ENGINE_APPLICATION = 3,
    AMTLS_ENGINE_ERROR = 4
} AmTLS_EngineState;

typedef struct AmTLS_HandshakeEngineOps {
    AmTLS_EngineState (*state)(void *engine);
    unsigned char *(*send_buffer)(void *engine, size_t *length);
    void (*send_ack)(void *engine, size_t length);
    unsigned char *(*recv_buffer)(void *engine, size_t *length);
    void (*recv_ack)(void *engine, size_t length);
} AmTLS_HandshakeEngineOps;

typedef struct AmTLS_HandshakePump {
    void *engine;
    const AmTLS_HandshakeEngineOps *engine_ops;
    AmTLS_Transport *transport;
} AmTLS_HandshakePump;

AmTLS_BackendResult amtls_handshake_pump_step(AmTLS_HandshakePump *pump);

#endif
