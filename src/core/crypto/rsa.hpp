#pragma once

#include <openssl/pem.h> //  includes "openssl/evp.h" which includes "openssl/bio.h"

#include <iostream>
#include <fstream>
#include <string>

namespace neroshop {

namespace crypto {
    EVP_PKEY * rsa_generate_keys_get();
    bool rsa_generate_keys(std::string public_key_filename = "public.pem", std::string private_key_filename = "private.pem");
    bool rsa_generate_keys_ex(); // requires openssl 3.0 // the _ex suffix is for extended versions of existing functions
    
    bool rsa_save_public_key(const EVP_PKEY * pkey, std::string filename = "public.pem");
    bool rsa_save_private_key(const EVP_PKEY * pkey, std::string filename = "private.pem");
    bool rsa_save_keys(const EVP_PKEY * pkey, std::string public_key_file = "public.pem", std::string private_key_file = "private.pem");
    
    std::string rsa_public_encrypt(const std::string& public_key, const std::string& plain_text); // encrypts plain text with a receiver's public key then returns a cipher text otherwise it returns an empty string on failure
    std::string rsa_private_decrypt(const std::string& private_key, const std::string& cipher_text); // decrypts cipher text using the receiver's private key then returns a plain text otherwise it returns an empty string on failure
    
    void rsa_public_encrypt_fp(const std::string& public_key, const std::string& plain_text_in, std::ofstream& file);
    void rsa_private_decrypt_fp(const std::string& private_key, std::string& plain_text_out, std::ifstream& file);
    
    std::string rsa_encrypt_message(const EVP_PKEY * key, const std::string& plain_text); // returns encrypted text
    std::string rsa_decrypt_message(const EVP_PKEY * key, const std::string& cipher_text); // returns decrypted text

    std::string rsa_private_sign(const std::string& private_key, const std::string& message);
    bool rsa_public_verify(const std::string& public_key, const std::string& message, const std::string& signature);
    
    std::string rsa_sign_message(const EVP_PKEY * signing_key, const std::string& message); // sign a message using private key // returns the signature
    bool rsa_verify_signature(const EVP_PKEY * verify_key, const std::string& message, const std::string& signature); // verify signature of a message using public_key

    std::string rsa_get_public_key(const EVP_PKEY * pkey);
    std::string rsa_get_private_key(const EVP_PKEY * pkey);
    std::pair<std::string, std::string> rsa_get_keys(const EVP_PKEY * pkey);
}

}
