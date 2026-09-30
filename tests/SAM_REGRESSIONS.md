# SAM client regressions (issue #330)

Investigated 2026-09-30 against main `9832cecbc099e601a487d04d5851deb36d237ece`.
Issue #330 was open, unassigned, with zero comments. No open SAM-related PR
was returned by the GitHub search at investigation time.

## Run

On Linux with Python 3.8+, g++ (C++17), and OpenSSL development headers:

```sh
python3 tests/sam_client_regression.py
```

TCP port 7656 must be free. The suite fails rather than connecting to an
existing router. Each subprocess uses a temporary working directory and
synthetic keys; existing `i2p.key` files are not touched. The mock SAM bridge
validates command sequencing; actual UDP receive behavior uses loopback
sockets. These are not live-router integration tests.

To reproduce against an unmodified source file:

```sh
git show 9832cecbc099e601a487d04d5851deb36d237ece:src/core/network/sam_client.cpp > /tmp/sam_client_baseline.cpp
python3 tests/sam_client_regression.py /tmp/sam_client_baseline.cpp
```

## Results

| Test | Baseline | Patch |
| --- | --- | --- |
| Restore null-certificate destination | KEY_NOT_FOUND / exits | Pass |
| Restore key-certificate destination, requiring base64 padding | KEY_NOT_FOUND / exits | Pass |
| Restore destination with certificate extending beyond 1024 bytes | KEY_NOT_FOUND / exits | Pass |
| Generate and save a new destination | Pass | Pass |
| Empty UDP queue, binary packet, drained queue, invalid socket | Throws on first empty read | Pass |
| Truncated fixed destination header | Incorrect lookup / exits | Throws locally |
| Truncated certificate payload | Incorrect lookup / exits | Throws locally |

The certificate fixtures exercise serialization boundaries, not cryptographic
key validity. The router remains responsible for validating complete private
key material during SESSION CREATE. Expected public keys and b32 addresses
are computed independently in Python. The mock rejects every private-key
NAMING LOOKUP and requires the original private key in SESSION CREATE.

## Scope and router-specific findings

| Symptom | Evidence | Effect of this patch / remaining work |
| --- | --- | --- |
| emissary: KEY_NOT_FOUND on restored private key | Existing code passes the whole private destination to NAMING LOOKUP before creating the session. Strict naming-service behavior reproduces the failure. | Extract the public Destination locally using its certificate length; no lookup is issued. Preserve public key/address availability after session_prepare(). Live emissary verification remains outstanding. |
| Any router: receive fails while no packet is queued | Constructor sets O_NONBLOCK; recvfrom returns EAGAIN/EWOULDBLOCK and the original implementation throws. Reproduced with real loopback sockets. | Return an empty vector only for would-block. Actual socket errors still throw. Callers should wait for readiness or pace polling to avoid a busy loop. |
| i2pd: UDP send failure / Identity buffer length 45 | Issue example sends a b32 hostname directly in the UDP header. SAM documentation says i2pd requires a full base64 Destination for this path. However, current i2pd openssl branch SAMSession::SendDatagram now resolves .i2p addresses, so implementation version matters. | NOT fixed or reproduced against a live router. Record the affected router version, compare b32 versus resolved base64 Destination, and capture router logs before changing the send path. The 45-byte error is consistent with a short hostname being treated as encoded identity, but is not proven here. |
| i2pd: receiving datagrams | datagram_parse currently requires FROM_PORT even though it is a SAM 3.2 extension. | NOT changed; a separate SAM 3.1 parsing regression/fix is needed if callers use this parser. |
| Java I2P: tunnel ends after minutes | Client advertises MAX=3.3 but does not service control-socket PING after session creation. Current Java SAMv3Handler uses a three-minute read timeout and closes on missing PONG when SAM >=3.2 is negotiated. | NOT fixed or live-reproduced. This is a separate control-channel lifecycle problem. Capture negotiated version and PING/PONG transcript, then test an appropriate keepalive implementation for over six minutes. Do not attribute the reported 1–5-minute interval conclusively to this path. |
| i2pd: EOF on application exit | session_close shuts down/closes the control connection, which ends the SAM session. | No change; an EOF at intentional shutdown alone is not evidence of send failure. |

This patch deliberately does not close issue #330. No live emissary, i2pd,
or Java I2P router was run, and the full application was not built. The
standalone SAM client compilation and the seven regressions above passed.

## Design and rollback

Private destination serialization begins with a 384-byte keys/padding area,
followed by a three-byte certificate header and its variable-length payload.
Extract and re-encode those bytes using I2P's base64 alphabet; slicing a fixed
number of base64 characters would be incorrect for non-null certificates.
The decoder buffer follows the encoded input size to avoid truncating longer
destinations. Local extraction preserves the session_prepare API; NAMING
LOOKUP NAME=ME would instead require an already-created session.

The only receive behavior change is that would-block produces an empty
vector. The header now documents this existing non-blocking socket contract.
The patch does not rewrite saved keys. Reverting the commit restores the
previous behavior without a key migration.

## Sources

- https://github.com/layters/testshop/issues/330
- https://www.i2p.net/en/docs/api/samv3/ (private destinations, naming, UDP, PING/PONG)
- https://www.i2p.net/en/docs/specs/common-structures/ (KeysAndCert and Destination)
- https://github.com/PurpleI2P/i2pd/blob/openssl/libi2pd_client/SAM.cpp (SendDatagram; retrieved 2026-09-30)
- https://github.com/i2p/i2p.i2p/blob/master/apps/sam/java/src/net/i2p/sam/SAMv3Handler.java (READ_TIMEOUT and PONG timeout; retrieved 2026-09-30)
