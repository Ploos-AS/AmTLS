#include <bearssl.h>

/*
 * AmTLS deliberately excludes BearSSL's host/CPU probing sources from the
 * classic-Amiga client vendor subset.  The upstream default-selection helpers
 * still reference these capability hooks, so provide the same "not available"
 * answers that BearSSL's unsupported-platform implementations provide.
 *
 * Entropy is supplied explicitly by AmTLS and injected into the SSL engine
 * before reset.  Returning no system seeder prevents an accidental fallback to
 * an OS-specific RNG while keeping ssl_engine.c linkable on Amiga-like targets.
 */
br_prng_seeder
br_prng_seeder_system(const char **name)
{
    if (name != 0) {
        *name = "none";
    }
    return 0;
}

const br_block_cbcenc_class *
br_aes_x86ni_cbcenc_get_vtable(void)
{
    return 0;
}

const br_block_cbcdec_class *
br_aes_x86ni_cbcdec_get_vtable(void)
{
    return 0;
}

const br_block_ctr_class *
br_aes_x86ni_ctr_get_vtable(void)
{
    return 0;
}

const br_block_ctrcbc_class *
br_aes_x86ni_ctrcbc_get_vtable(void)
{
    return 0;
}

br_ghash
br_ghash_pclmul_get(void)
{
    return 0;
}

br_chacha20_run
br_chacha20_sse2_get(void)
{
    return 0;
}
