#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../src/platform/allocator.h"
#include "../src/platform/transport.h"

struct TestTransport {
    const unsigned char *input;
    size_t input_len;
    size_t input_pos;
    unsigned char output[32];
    size_t output_len;
};

static long test_read(void *user, void *buffer, size_t length)
{
    struct TestTransport *t = (struct TestTransport *)user;
    size_t available = t->input_len - t->input_pos;
    size_t n = length < available ? length : available;
    memcpy(buffer, t->input + t->input_pos, n);
    t->input_pos += n;
    return (long)n;
}

static long test_write(void *user, const void *buffer, size_t length)
{
    struct TestTransport *t = (struct TestTransport *)user;
    size_t room = sizeof(t->output) - t->output_len;
    size_t n = length < room ? length : room;
    memcpy(t->output + t->output_len, buffer, n);
    t->output_len += n;
    return (long)n;
}

static int test_close(void *user)
{
    (void)user;
    return 0;
}

static void *test_alloc(void *user, size_t size)
{
    (void)user;
    return malloc(size);
}

static void test_free(void *user, void *ptr, size_t size)
{
    (void)user;
    (void)size;
    free(ptr);
}

int main(void)
{
    static const unsigned char input[] = "server";
    struct TestTransport state;
    AmTLS_Transport transport;
    AmTLS_Allocator allocator;
    char buffer[8];
    void *p;

    memset(&state, 0, sizeof(state));
    state.input = input;
    state.input_len = sizeof(input) - 1;
    transport.user = &state;
    transport.read = test_read;
    transport.write = test_write;
    transport.close = test_close;

    memset(buffer, 0, sizeof(buffer));
    assert(transport.read(transport.user, buffer, 3) == 3);
    assert(memcmp(buffer, "ser", 3) == 0);
    assert(transport.write(transport.user, "client", 6) == 6);
    assert(state.output_len == 6);
    assert(memcmp(state.output, "client", 6) == 0);

    memset(&allocator, 0, sizeof(allocator));
    allocator.alloc = test_alloc;
    allocator.free = test_free;
    p = amtls_allocator_alloc(&allocator, 16);
    assert(p != 0);
    assert(allocator.stats.current_bytes == 16);
    assert(allocator.stats.peak_bytes == 16);
    assert(allocator.stats.total_bytes == 16);
    assert(allocator.stats.allocations == 1);
    amtls_allocator_free(&allocator, p, 16);
    assert(allocator.stats.current_bytes == 0);
    assert(allocator.stats.frees == 1);

    return 0;
}
