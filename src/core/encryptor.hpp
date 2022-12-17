#ifndef ENCRYPTOR_HPP_NEROSHOP
#define ENCRYPTOR_HPP_NEROSHOP

#include <openssl/rsa.h>
#include <openssl/pem.h> //  includes "openssl/evp.h" which includes "openssl/bio.h"
#include <openssl/err.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility> // std::pair
//#include <cstring> // strlen

#include "debug.hpp"
//# define AES_BLOCK_SIZE 16

namespace neroshop {
class Encryptor {
public:
    static bool generate_key_pair();
    static bool generate_key_pair_ex(); // requires openssl 3.0 // the _ex suffix is for extended versions of existing functions//static bool generate_public_key_from_private_key();
    static EVP_PKEY * generate_key_pair_return();
    static bool save_public_key(const EVP_PKEY * pkey, std::string filename = "public.pem");
    static bool save_private_key(const EVP_PKEY * pkey, std::string filename = "private.pem");
    static bool save_key_pair(const EVP_PKEY * pkey, std::string public_key_file = "public.pem", std::string private_key_file = "private.pem");
    // encryption and decrypting functions - messages
    static std::string public_encrypt(const std::string& public_key, const std::string& plain_text); // encrypts plain text with a receiver's public key then returns a cipher text otherwise it returns an empty string on failure
    static std::string private_decrypt(const std::string& private_key, const std::string& cipher_text); // decrypts cipher text using the receiver's private key then returns a plain text otherwise it returns an empty string on failure
    static void public_encrypt_fp(const std::string& public_key, const std::string& plain_text_in, std::ofstream& file);
    static void private_decrypt_fp(const std::string& private_key, std::string& plain_text_out, std::ifstream& file);
    static std::string encrypt_message(const EVP_PKEY * key, const std::string& plain_text); // returns encrypted text
    static std::string decrypt_message(const EVP_PKEY * key, const std::string& cipher_text); // returns decrypted text    
    // signing and verifying functions - login system (not implemented yet)
    static std::string private_sign(const std::string& private_key, const std::string& message);
    static bool public_verify(const std::string& public_key, const std::string& message, const std::string& signature);
    static std::string sign_message(const EVP_PKEY * signing_key, const std::string& message); // sign a message using private key // returns the signature
    static bool verify_signature(const EVP_PKEY * verify_key, const std::string& message, const std::string& signature); // verify signature of a message using public_key
    // getters
    static std::string get_public_key(const EVP_PKEY * pkey);
    static std::string get_private_key(const EVP_PKEY * pkey);
    static std::pair<std::string, std::string> get_key_pair(const EVP_PKEY * pkey);
private:
};
}
#endif
