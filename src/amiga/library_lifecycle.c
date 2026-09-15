#include "amtls/library.h"

static const char library_id[] =
    "amtls.library 0.1 (15.09.2026)\r\n";

int amtls_library_init(AmTLS_LibraryState *state)
{
    if (state == NULL) {
        return -1;
    }
    state->open_count = 0;
    state->initialized = 1;
    return 0;
}

int amtls_library_open(AmTLS_LibraryState *state)
{
    if (state == NULL || !state->initialized) {
        return -1;
    }
    state->open_count++;
    return 0;
}

int amtls_library_close(AmTLS_LibraryState *state)
{
    if (state == NULL || !state->initialized || state->open_count == 0) {
        return -1;
    }
    state->open_count--;
    return 0;
}

int amtls_library_can_expunge(const AmTLS_LibraryState *state)
{
    return state != NULL && state->initialized && state->open_count == 0;
}

void amtls_library_expunge(AmTLS_LibraryState *state)
{
    if (state != NULL && state->open_count == 0) {
        state->initialized = 0;
    }
}

const char *amtls_library_id(void)
{
    return library_id;
}
