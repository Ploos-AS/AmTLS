#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/bsdsocket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "platform/bsdsocket_transport.h"

struct Library *SocketBase = NULL;

int main(void)
{
    const unsigned char *host_name = (const unsigned char *)"example.com";
    const struct hostent * const *host_result;
    const struct hostent *host;
    struct sockaddr_in address;
    int fd = -1;
    AmTLS_BSDSocketTransport state;
    AmTLS_Transport transport;
    const char request[] = "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";

    SocketBase = OpenLibrary("bsdsocket.library", 4);
    if (SocketBase == NULL) {
        PutStr("AmTLS M3.3b: FAIL bsdsocket.library\n");
        return 20;
    }

    host_result = (const struct hostent * const *)&(const struct hostent *){ gethostbyname(host_name) };
    host = *host_result;
    if (host == NULL || host->h_addr_list == NULL || host->h_addr_list[0] == NULL) {
        PutStr("AmTLS M3.3b: FAIL DNS\n");
        CloseLibrary(SocketBase);
        return 20;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        PutStr("AmTLS M3.3b: FAIL socket\n");
        CloseLibrary(SocketBase);
        return 20;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(80);
    memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        PutStr("AmTLS M3.3b: FAIL connect\n");
        CloseSocket(fd);
        CloseLibrary(SocketBase);
        return 20;
    }

    if (amtls_bsdsocket_transport_init(&state, &transport, fd, 1) != 0 ||
        transport.write(transport.user, request, sizeof(request) - 1) <= 0) {
        PutStr("AmTLS M3.3b: FAIL transport\n");
        CloseSocket(fd);
        CloseLibrary(SocketBase);
        return 20;
    }

    transport.close(transport.user);
    CloseLibrary(SocketBase);
    PutStr("AMTLS_M3_3B_PASS\n");
    PutStr("AmTLS M3.3b: PASS DNS/TCP\n");
    return 0;
}
