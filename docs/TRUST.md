# AmTLS trust and verification policy

AmTLS treats certificate verification as mandatory security state, not an optional convenience feature.

## Trust-anchor contract

The BearSSL backend is initialized with an explicit array of `br_x509_trust_anchor` objects. A TLS client configuration with no trust anchors fails before any ClientHello is emitted.

This keeps trust-store loading outside the TLS engine itself and makes the backend independent of filesystem layout. On classic AmigaOS, a platform layer can therefore load and parse a compact CA bundle once, retain the resulting anchors, and share them across connections without teaching the TLS core about DOS paths or a specific TCP/IP stack.

The initial Amiga strategy is:

1. ship or install a curated CA bundle separately from `amtls.library`;
2. parse that bundle into BearSSL trust-anchor objects in the platform/integration layer;
3. pass the immutable anchor array into the backend configuration;
4. fail closed if the bundle cannot be loaded, parsed, or yields zero anchors.

A later optimization may provide a reduced CA set for memory-constrained systems, but it must not disable chain or hostname verification.

## Hostname verification

The configured `server_name` is passed to `br_ssl_client_reset()`. BearSSL's minimal X.509 validator uses that expected server name while validating the peer certificate. The same name is also emitted as SNI in the TLS ClientHello.

Empty server names are rejected by AmTLS before the handshake starts.

## Chain validation

`br_ssl_client_init_full()` is initialized with the explicit trust-anchor array. BearSSL's minimal X.509 validator is therefore the certificate-chain authority for the first backend.

The pinned BearSSL source is already qualified with its upstream X.509 test suite. AmTLS additionally treats missing trust as an initialization failure so applications cannot accidentally obtain an unverified connection by omitting CA configuration.

## Security rules

- no trust anchors: fail closed;
- no secure entropy provider: fail closed;
- empty server name: fail closed;
- TLS 1.2 only for the initial backend policy;
- no "insecure" or skip-verification flag in the v0.1 public contract;
- verification failures must surface as connection failure, never application data readiness.

Runtime AmigaOS qualification and CA-bundle parsing are separate M3 integration gates before release.
