# M2.1c qualification — Trust and verification

Status: PASS

Qualified milestone: M2.1c — Trust and verification

Repository: `Ploos-AS/AmTLS`

Qualified code baseline:

- commit `ffebd09417679d2dc079ac604c5487290aaac8f4`
- GitHub Actions CI run #75
- run ID `34897174642`
- result: 11/11 jobs PASS

## Scope

This qualification proves that AmTLS performs certificate-chain validation and hostname verification through the real transport-driven BearSSL client path, not only through a standalone X.509 helper.

The transport fixture uses a loopback TCP connection to an OpenSSL TLS 1.2 server, AmTLS transport callbacks, the M2.1a handshake pump, the M2.1b BearSSL client binding, explicit entropy, explicit validation time and explicit trust anchors.

## Qualified transport cases

### Trusted chain + matching hostname

The client connects through AmTLS transport using the Test CA trust anchor and the server name `valid.example`.

Expected and observed result:

- handshake reaches `AMTLS_BACKEND_OK`
- BearSSL last error is `BR_ERR_OK`
- certificate chain is accepted
- hostname is accepted

Result: PASS

### Hostname mismatch

The client trusts the issuing CA but requests `wrong.example` while the certificate is valid for `valid.example`.

Expected and observed result:

- handshake reaches `AMTLS_BACKEND_ERROR`
- BearSSL reports `BR_ERR_X509_BAD_SERVER_NAME`

Result: PASS

### Unknown CA

The client requests the matching server name but is configured with a different CA trust anchor.

Expected and observed result:

- handshake reaches `AMTLS_BACKEND_ERROR`
- BearSSL reports `BR_ERR_X509_NOT_TRUSTED`

Result: PASS

## Supporting qualification

The same CI run also passed:

- `bearssl-x509-security`
- `bearssl-client-host`
- `bearssl-client-m68k`
- `bearssl-host-qualification`
- `bearssl-vendor-host`
- `bearssl-vendor-m68k`
- `bearssl-vendor-policy`
- `bearssl-probe`
- `m68k-smoke`
- `host`

The transport qualification exposed and fixed two state-progression defects before this PASS:

1. the handshake pump originally returned the pre-ack I/O direction after `sendrec_ack()` / `recvrec_ack()` instead of following the engine's new state;
2. BearSSL application readiness (`BR_SSL_SENDAPP` / `BR_SSL_RECVAPP`) must take precedence over record readiness when both are reported, otherwise a completed handshake may be mistaken for a request for more network input.

The final qualified implementation follows the post-ack engine state and recognizes application readiness before record readiness.

## Security properties qualified by M2.1c

- non-empty trust anchors are mandatory
- explicit certificate-validation time is mandatory
- server name is mandatory
- trusted certificate chains are accepted
- hostname mismatches fail closed
- unknown certificate authorities fail closed
- validation is exercised through the actual AmTLS transport and handshake path

No insecure skip-verification path is introduced.

## Milestone verdict

M2.1c — Trust and verification: PASS.

The next implementation phase is M3 — Amiga network integration. The separate unresolved M2.0 vendor-source policy item (`commit reviewed source subset under third_party/vendor/BearSSL`) remains open and is not implicitly closed by this M2.1c qualification.
