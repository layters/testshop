#pragma once  // works in both C and C++
#ifndef MKP224O_CONFIG_H
#define MKP224O_CONFIG_H

#define ED25519_donna
#define STATISTICS
#define PASSPHRASE
#define VERSION "v1.7.0"

#define CRYPTO_NAMESPACETOP crypto_sign_ed25519_donna
#define _CRYPTO_NAMESPACETOP _crypto_sign_ed25519_donna

#define CRYPTO_NAMESPACE(name) crypto_sign_ed25519_donna_##name
#define _CRYPTO_NAMESPACE(name) _crypto_sign_ed25519_donna_##name

#endif // MKP224O_CONFIG_H
