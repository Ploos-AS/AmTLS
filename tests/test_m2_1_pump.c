#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "backends/handshake_pump.h"

typedef struct FakeEngine {
    AmTLS_EngineState state;
    unsigned char tx[8];
    unsigned char rx[8];
    size_t tx_len;
    size_t rx_len;
    size_t tx_acked;
    size_t rx_acked;
} FakeEngine;

typedef struct FakeTransport {
    long read_result;
    long write_result;
    size_t reads;
    size_t writes;
} FakeTransport;

static AmTLS_EngineState fake_state(void *engine)
{
    return ((FakeEngine *)engine)->state;
}

static unsigned char *fake_send_buffer(void *engine, size_t *length)
{
    FakeEngine *e = (FakeEngine *)engine;
    *length = e->tx_len;
    return e->tx;
}

static void fake_send_ack(void *engine, size_t length)
{
    ((FakeEngine *)engine)->tx_acked += length;
}

static unsigned char *fake_recv_buffer(void *engine, size_t *length)
{
    FakeEngine *e = (FakeEngine *)engine;
    *length = e->rx_len;
    return e->rx;
}

static void fake_recv_ack(void *engine, size_t length)
{
    ((FakeEngine *)engine)->rx_acked += length;
}

static long fake_read(void *user, void *buffer, size_t length)
{
    FakeTransport *t = (FakeTransport *)user;
    size_t n;
    t->reads++;
    if (t->read_result <= 0) {
        return t->read_result;
    }
    n = (size_t)t->read_result;
    if (n > length) {
        n = length;
    }
    memset(buffer, 0xA5, n);
    return (long)n;
}

static long fake_write(void *user, const void *buffer, size_t length)
{
    FakeTransport *t = (FakeTransport *)user;
    size_t n;
    (void)buffer;
    t->writes++;
    if (t->write_result <= 0) {
        return t->write_result;
    }
    n = (size_t)t->write_result;
    if (n > length) {
        n = length;
    }
    return (long)n;
}

static const AmTLS_HandshakeEngineOps engine_ops = {
    fake_state,
    fake_send_buffer,
    fake_send_ack,
    fake_recv_buffer,
    fake_recv_ack
};

int main(void)
{
    FakeEngine engine;
    FakeTransport io;
    AmTLS_Transport transport;
    AmTLS_HandshakePump pump;

    memset(&engine, 0, sizeof(engine));
    memset(&io, 0, sizeof(io));

    transport.user = &io;
    transport.read = fake_read;
    transport.write = fake_write;
    transport.close = 0;

    pump.engine = &engine;
    pump.engine_ops = &engine_ops;
    pump.transport = &transport;

    engine.state = AMTLS_ENGINE_APPLICATION;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_OK);

    engine.state = AMTLS_ENGINE_SEND_RECORD;
    engine.tx_len = 6;
    io.write_result = 3;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_WANT_WRITE);
    assert(engine.tx_acked == 3);
    assert(io.writes == 1);

    io.write_result = -1;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_WANT_WRITE);
    assert(engine.tx_acked == 3);

    engine.state = AMTLS_ENGINE_RECV_RECORD;
    engine.rx_len = 5;
    io.read_result = 2;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_WANT_READ);
    assert(engine.rx_acked == 2);
    assert(io.reads == 1);

    io.read_result = -1;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_WANT_READ);
    assert(engine.rx_acked == 2);

    io.read_result = 0;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_CLOSED);

    engine.state = AMTLS_ENGINE_ERROR;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_ERROR);

    engine.state = AMTLS_ENGINE_CLOSED;
    assert(amtls_handshake_pump_step(&pump) == AMTLS_BACKEND_CLOSED);

    return 0;
}
