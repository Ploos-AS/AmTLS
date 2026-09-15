#include <string.h>

#include "amtls/library.h"

int main(void)
{
    AmTLS_LibraryState state;

    if (amtls_library_init(&state) != 0) return 1;
    if (!state.initialized || state.open_count != 0) return 1;
    if (!amtls_library_can_expunge(&state)) return 1;
    if (amtls_library_open(&state) != 0) return 1;
    if (state.open_count != 1 || amtls_library_can_expunge(&state)) return 1;
    if (amtls_library_close(&state) != 0) return 1;
    if (state.open_count != 0 || !amtls_library_can_expunge(&state)) return 1;
    if (strstr(amtls_library_id(), "amtls.library 0.1") == NULL) return 1;
    amtls_library_expunge(&state);
    if (state.initialized) return 1;
    if (amtls_library_open(&state) == 0) return 1;
    return 0;
}
