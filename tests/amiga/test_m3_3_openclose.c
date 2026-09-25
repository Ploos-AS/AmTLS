#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <stdio.h>

#define AMTLS_LIBRARY_NAME "amtls.library"

int main(void)
{
    struct Library *base;
    int i;

    base = OpenLibrary(AMTLS_LIBRARY_NAME, 0);
    if (base == NULL) {
        puts("AmTLS M3.3a: FAIL OpenLibrary");
        return 20;
    }

    printf("AmTLS M3.3a: opened %s version %lu.%lu\n",
           AMTLS_LIBRARY_NAME,
           (unsigned long)base->lib_Version,
           (unsigned long)base->lib_Revision);
    CloseLibrary(base);

    for (i = 0; i < 100; ++i) {
        base = OpenLibrary(AMTLS_LIBRARY_NAME, 0);
        if (base == NULL) {
            printf("AmTLS M3.3a: FAIL reopen iteration %d\n", i);
            return 20;
        }
        CloseLibrary(base);
    }

    puts("AmTLS M3.3a: PASS");
    return 0;
}
