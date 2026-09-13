#include <stdio.h>
#include "amtls/amtls.h"

int main(void)
{
    puts(amtls_version_string());
    puts("M0 foundation: no TLS backend enabled");
    return 0;
}
