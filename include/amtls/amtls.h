#ifndef AMTLS_AMTLS_H
#define AMTLS_AMTLS_H

#ifdef __cplusplus
extern "C" {
#endif

#define AMTLS_VERSION_MAJOR 0
#define AMTLS_VERSION_MINOR 0
#define AMTLS_VERSION_PATCH 0

typedef enum AmTLS_Result {
    AMTLS_OK = 0,
    AMTLS_ERR_ARGUMENT = -1,
    AMTLS_ERR_UNSUPPORTED = -2,
    AMTLS_ERR_BACKEND = -3,
    AMTLS_ERR_IO = -4,
    AMTLS_ERR_VERIFY = -5
} AmTLS_Result;

typedef struct AmTLS_Context AmTLS_Context;
typedef struct AmTLS_Connection AmTLS_Connection;

typedef struct AmTLS_Config {
    const char *server_name;
    const char *ca_path;
    unsigned long flags;
} AmTLS_Config;

const char *amtls_version_string(void);
AmTLS_Result amtls_context_create(AmTLS_Context **out_context);
void amtls_context_destroy(AmTLS_Context *context);
AmTLS_Result amtls_connection_create(AmTLS_Context *context,
                                     const AmTLS_Config *config,
                                     AmTLS_Connection **out_connection);
void amtls_connection_destroy(AmTLS_Connection *connection);

#ifdef __cplusplus
}
#endif

#endif
