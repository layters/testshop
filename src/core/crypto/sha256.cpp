#include "sha256.hpp"

#include "../tools/logger.hpp" // neroshop::print

#include <openssl/evp.h>
#include <openssl/err.h>

#include <iomanip> // std::setfill, std::setw

std::string neroshop::crypto::sha256(const std::string& plain_text) {
    // EVP (recommended over legacy "SHA256_" functions which are deprecated in OpenSSL 3.0)
    EVP_MD_CTX * context = EVP_MD_CTX_new();
    if(context == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return ""; }
    
    if(EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    if(EVP_DigestUpdate(context, plain_text.c_str(), plain_text.length()) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    if(EVP_DigestFinal_ex(context, digest, &length) != 1) {
        EVP_MD_CTX_free(context);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    std::stringstream ss;
    for(unsigned int i = 0; i < length; ++i) 
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    std::string output = ss.str();
    
    EVP_MD_CTX_free(context); // renamed from "EVP_MD_CTX_destroy" in 1.1.0, same with EVP_MD_CTX_create => EVP_MD_CTX_new
    return output;
}

/*int main() {
    std::cout << neroshop::crypto::sha256("Turtles are cool") << "\n";
    
    return 0;
} // g++ sha256.cpp -lcrypto -lssl
*/
