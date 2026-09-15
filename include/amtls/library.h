#ifndef AMTLS_LIBRARY_H
#define AMTLS_LIBRARY_H

#include <stddef.h>

#define AMTLS_LIBRARY_NAME "amtls.library"
#define AMTLS_LIBRARY_VERSION 0
#define AMTLS_LIBRARY_REVISION 1

typedef struct AmTLS_LibraryState {
    unsigned long open_count;
    int initialized;
} AmTLS_LibraryState;

int amtls_library_init(AmTLS_LibraryState *state);
int amtls_library_open(AmTLS_LibraryState *state);
int amtls_library_close(AmTLS_LibraryState *state);
int amtls_library_can_expunge(const AmTLS_LibraryState *state);
void amtls_library_expunge(AmTLS_LibraryState *state);
const char *amtls_library_id(void);

#endif
