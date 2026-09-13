#include <stdlib.h>
#include "amtls/amtls.h"

struct AmTLS_Context {
    unsigned long reserved;
};

struct AmTLS_Connection {
    AmTLS_Context *context;
    AmTLS_Config config;
};

const char *amtls_version_string(void)
{
    return "AmTLS 0.0.0-m0";
}

AmTLS_Result amtls_context_create(AmTLS_Context **out_context)
{
    AmTLS_Context *context;

    if (!out_context) {
        return AMTLS_ERR_ARGUMENT;
    }

    context = (AmTLS_Context *)calloc(1, sizeof(*context));
    if (!context) {
        return AMTLS_ERR_BACKEND;
    }

    *out_context = context;
    return AMTLS_OK;
}

void amtls_context_destroy(AmTLS_Context *context)
{
    free(context);
}

AmTLS_Result amtls_connection_create(AmTLS_Context *context,
                                     const AmTLS_Config *config,
                                     AmTLS_Connection **out_connection)
{
    AmTLS_Connection *connection;

    if (!context || !config || !config->server_name || !out_connection) {
        return AMTLS_ERR_ARGUMENT;
    }

    connection = (AmTLS_Connection *)calloc(1, sizeof(*connection));
    if (!connection) {
        return AMTLS_ERR_BACKEND;
    }

    connection->context = context;
    connection->config = *config;
    *out_connection = connection;
    return AMTLS_OK;
}

void amtls_connection_destroy(AmTLS_Connection *connection)
{
    free(connection);
}
