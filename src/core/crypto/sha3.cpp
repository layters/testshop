#include "sha3.hpp"

#include "../tools/logger.hpp" // neroshop::print

#include <openssl/evp.h>
#include <openssl/err.h>

#include <iomanip> // std::setfill, std::setw
#include <vector> // std::vector
#include <memory> // std::unique_ptr

namespace neroshop {

namespace crypto {

std::string sha3_256(const std::string& plain_text) {
    // EVP (recommended over legacy "SHA256_" functions which are deprecated in OpenSSL 3.0)
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    if(context == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return ""; }
    
    if(EVP_DigestInit_ex(context.get(), EVP_sha3_256(), nullptr) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    if(EVP_DigestUpdate(context.get(), plain_text.c_str(), plain_text.length()) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
    unsigned int length = 0;
    if(EVP_DigestFinal_ex(context.get(), &digest[0], &length) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    std::stringstream ss;
    for(unsigned int i = 0; i < length; ++i) 
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    std::string output = ss.str();
    
    return output;
}

std::string sha3_512(const std::string& plain_text) {
    // EVP (recommended over legacy "SHA512_" functions which are deprecated in OpenSSL 3.0)
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> context(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
    if(context == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return ""; }
    
    if(EVP_DigestInit_ex(context.get(), EVP_sha3_512(), nullptr) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    if(EVP_DigestUpdate(context.get(), plain_text.c_str(), plain_text.length()) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
    unsigned int length = 0;
    if(EVP_DigestFinal_ex(context.get(), &digest[0], &length) != 1) {
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return "";
    }
    
    std::stringstream ss;
    for(unsigned int i = 0; i < length; ++i) 
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    std::string output = ss.str();
    
    return output;
}

}

}

/*int main() {
    std::cout << neroshop::crypto::sha3_256("Turtles are cool") << "\n\n";
    std::cout << neroshop::crypto::sha3_512("Turtles are cool") << "\n";
    return 0;
} // g++ sha3.cpp -o sha3 -lcrypto -lssl
*/
