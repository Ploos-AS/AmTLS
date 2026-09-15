#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "platform/bsdsocket_transport.h"

int main(void)
{
    int pair[2];
    char buffer[8];
    AmTLS_BSDSocketTransport state;
    AmTLS_Transport transport;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0) return 1;
    if (amtls_bsdsocket_transport_init(&state, &transport, pair[0], 1) != 0) return 1;

    if (write(pair[1], "ping", 4) != 4) return 1;
    memset(buffer, 0, sizeof(buffer));
    if (transport.read(transport.user, buffer, 4) != 4) return 1;
    if (memcmp(buffer, "ping", 4) != 0) return 1;

    if (transport.write(transport.user, "pong", 4) != 4) return 1;
    memset(buffer, 0, sizeof(buffer));
    if (read(pair[1], buffer, 4) != 4) return 1;
    if (memcmp(buffer, "pong", 4) != 0) return 1;

    if (transport.close(transport.user) != 0) return 1;
    if (state.socket_fd != -1) return 1;
    close(pair[1]);

    puts("PASS: M3.1 bsdsocket-compatible transport adapter");
    return 0;
}
