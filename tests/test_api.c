#include <assert.h>
#include <string.h>
#include "amtls/amtls.h"

int main(void)
{
    AmTLS_Context *context = 0;
    AmTLS_Connection *connection = 0;
    AmTLS_Config config;

    assert(strstr(amtls_version_string(), "AmTLS") != 0);
    assert(amtls_context_create(0) == AMTLS_ERR_ARGUMENT);
    assert(amtls_context_create(&context) == AMTLS_OK);
    assert(context != 0);

    memset(&config, 0, sizeof(config));
    config.server_name = "example.com";

    assert(amtls_connection_create(context, &config, &connection) == AMTLS_OK);
    assert(connection != 0);

    amtls_connection_destroy(connection);
    amtls_context_destroy(context);
    return 0;
}
