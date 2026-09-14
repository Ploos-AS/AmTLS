#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bearssl.h>

#include "backends/bearssl_client.h"

typedef struct AnchorHolder {
    br_x509_decoder_context decoder;
    br_x509_trust_anchor anchor;
    unsigned char dn[1024];
    size_t dn_len;
    int overflow;
} AnchorHolder;

static void append_dn(void *ctx, const void *buf, size_t len)
{
    AnchorHolder *holder = (AnchorHolder *)ctx;

    if (holder->dn_len + len > sizeof(holder->dn)) {
        holder->overflow = 1;
        return;
    }
    memcpy(holder->dn + holder->dn_len, buf, len);
    holder->dn_len += len;
}

static unsigned char *read_file(const char *path, size_t *length)
{
    FILE *f;
    long n;
    unsigned char *buf;

    f = fopen(path, "rb");
    if (f == 0) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    n = ftell(f);
    if (n <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    buf = (unsigned char *)malloc((size_t)n);
    if (buf == 0) {
        fclose(f);
        return 0;
    }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *length = (size_t)n;
    return buf;
}

static int build_anchor(AnchorHolder *holder,
                        const unsigned char *der, size_t der_len)
{
    br_x509_pkey *pkey;

    memset(holder, 0, sizeof(*holder));
    br_x509_decoder_init(&holder->decoder, append_dn, holder);
    br_x509_decoder_push(&holder->decoder, der, der_len);
    if (holder->overflow || br_x509_decoder_last_error(&holder->decoder) != 0
            || !br_x509_decoder_isCA(&holder->decoder)) {
        return 0;
    }
    pkey = br_x509_decoder_get_pkey(&holder->decoder);
    if (pkey == 0 || holder->dn_len == 0) {
        return 0;
    }
    holder->anchor.dn.data = holder->dn;
    holder->anchor.dn.len = holder->dn_len;
    holder->anchor.flags = BR_X509_TA_CA;
    holder->anchor.pkey = *pkey;
    return 1;
}

int main(int argc, char **argv)
{
    unsigned char *server_der;
    unsigned char *ca_der;
    unsigned char *other_ca_der;
    size_t server_len;
    size_t ca_len;
    size_t other_ca_len;
    const unsigned char *chain[1];
    size_t chain_len[1];
    AnchorHolder trusted;
    AnchorHolder untrusted;
    int rc;

    assert(argc == 4);
    server_der = read_file(argv[1], &server_len);
    ca_der = read_file(argv[2], &ca_len);
    other_ca_der = read_file(argv[3], &other_ca_len);
    assert(server_der != 0);
    assert(ca_der != 0);
    assert(other_ca_der != 0);
    assert(build_anchor(&trusted, ca_der, ca_len));
    assert(build_anchor(&untrusted, other_ca_der, other_ca_len));

    chain[0] = server_der;
    chain_len[0] = server_len;

    rc = amtls_bearssl_verify_chain("valid.example",
                                    &trusted.anchor, 1,
                                    chain, chain_len, 1);
    assert(rc == BR_ERR_X509_OK);
    puts("PASS: trusted chain + matching hostname");

    rc = amtls_bearssl_verify_chain("wrong.example",
                                    &trusted.anchor, 1,
                                    chain, chain_len, 1);
    assert(rc == BR_ERR_X509_BAD_SERVER_NAME);
    puts("PASS: hostname mismatch rejected");

    rc = amtls_bearssl_verify_chain("valid.example",
                                    &untrusted.anchor, 1,
                                    chain, chain_len, 1);
    assert(rc == BR_ERR_X509_NOT_TRUSTED);
    puts("PASS: unknown CA rejected");

    free(server_der);
    free(ca_der);
    free(other_ca_der);
    return 0;
}
