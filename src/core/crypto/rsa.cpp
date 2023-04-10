#include "rsa.hpp"

#include "../util/logger.hpp" // neroshop::print

#include <openssl/rsa.h>
#include <openssl/err.h>

#include <sstream>

EVP_PKEY * neroshop::crypto::rsa_generate_keys_get() {
    // Generate public/private key pairs
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    EVP_PKEY * pkey = nullptr;
    if(!ctx) { neroshop::print("EVP_PKEY_CTX_new_id failed", 1); return nullptr; }
    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string("EVP_PKEY_keygen_init ") + std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return nullptr;
    }
    if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string("EVP_PKEY_CTX_set_rsa_keygen_bits ") + std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return nullptr;
    }
    if(EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string("EVP_PKEY_keygen ") + std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return nullptr;
    }
    // We need the pkey alive so that we can return it. Do not free it
    // Free the context instead
    EVP_PKEY_CTX_free(ctx);
    return pkey;
}
//------------------
bool neroshop::crypto::rsa_generate_keys(std::string public_key_filename, std::string private_key_filename) {
    // create a context
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr); // https://www.openssl.org/docs/man1.1.1/man3/EVP_PKEY_keygen.html
    EVP_PKEY * pkey = nullptr;
    if(!ctx) { neroshop::print("EVP_PKEY_CTX_new_id failed", 1); return false; }
    // initialize the context
    if(EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // set the RSA key length bits (default: 2048)
    if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // generate the key pair
    if(EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(ERR_error_string(ERR_get_error(), nullptr), 1);
        return false;
    }
    // save key pair
    if(!rsa_save_keys(pkey, public_key_filename, private_key_filename)) return false;
    // free up the pkey and the context
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    return true;
}
//------------------
bool neroshop::crypto::rsa_generate_keys_ex() {
#if !defined(NEROSHOP_OPENSSL_3_0) || !defined(NEROSHOP_OPENSSL_3)
    neroshop::print("error: NEROSHOP_OPENSSL_3_0 not defined", 1);
    return false;
#endif
#if defined(NEROSHOP_OPENSSL_3_0) || defined(NEROSHOP_OPENSSL_3)
    // an RSA key can be generated simply like this:
    EVP_PKEY * pkey = EVP_RSA_gen(2048);//(4096); // https://www.openssl.org/docs/man3.0/man7/EVP_PKEY-RSA.html
    if(!pkey) { neroshop::print("EVP_RSA_gen failed", 1); return false; }
    // save key pair
    if(!rsa_save_keys(pkey)) return false;
    // temporary - will be deleted soon
    /*auto key_pair = get_key_pair(pkey);
    std::cout << key_pair.first << std::endl << key_pair.second << std::endl;*/
    // free the pkey
    EVP_PKEY_free(pkey);
    return true;
#endif
    return false;    
}
//------------------
//------------------
bool neroshop::crypto::rsa_save_public_key(const EVP_PKEY * pkey, std::string filename) {
    BIO	* bio_public = BIO_new_file(filename.c_str(), "w+"); // or .pub
    if(PEM_write_bio_PUBKEY(bio_public, const_cast<EVP_PKEY *>(pkey)) != 1) {
    //if(PEM_write_bio_RSAPublicKey(bp_public, const_cast<RSA *>(rsa)) != 1) { // deprecated in OpenSSL 3.0
        neroshop::print("PEM_write_bio_RSAPublicKey failed", 1);
        BIO_free_all(bio_public);
        return false;
    }
    // free the bio now that we are done writing to it
    BIO_free_all(bio_public);
    neroshop::print(filename + " created", 3);
    return true;
}
//------------------
bool neroshop::crypto::rsa_save_private_key(const EVP_PKEY * pkey, std::string filename) {
    BIO	* bio_private = BIO_new_file(filename.c_str(), "w+"); // or .key
    if(PEM_write_bio_PKCS8PrivateKey(bio_private, const_cast<EVP_PKEY *>(pkey), nullptr, nullptr, 0, nullptr, nullptr) != 1) { // same as PEM_write_bio_PrivateKey - both use PKCS#8 format which supports all algorithms including RSA // TODO: add encryption e.g: EVP_aes_256_cbc() (in arg 3) using a passphrase/password (in arg 4) and passphrase_len (in arg 5)
    //if(PEM_write_bio_RSAPrivateKey(bp_private, const_cast<RSA *>(rsa), nullptr, nullptr, 0, nullptr, nullptr) != 1) { // deprecated in OpenSSL 3.0
        neroshop::print("PEM_write_bio_PKCS8PrivateKey failed", 1);
        BIO_free_all(bio_private);
        return false;
    }
    // free the bio now that we are done writing to it
    BIO_free_all(bio_private);
    neroshop::print(filename + " created", 3);
    return true;
}
//------------------
bool neroshop::crypto::rsa_save_keys(const EVP_PKEY * pkey, std::string public_key_file, std::string private_key_file) {
    if(!rsa_save_public_key(pkey, public_key_file)) return false;
    if(!rsa_save_private_key(pkey, private_key_file)) return false;
    return true;
}
//------------------
//------------------
// refer to: https://www.openssl.org/docs/man3.0/man3/EVP_PKEY_encrypt.html
std::string neroshop::crypto::rsa_encrypt_message(const EVP_PKEY * key, const std::string& in) {
    unsigned char * out;
    size_t outlen, inlen = in.size();
    ENGINE * eng = nullptr;
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new(const_cast<EVP_PKEY *>(key), eng);
    if(!ctx) { neroshop::print("EVP_PKEY_CTX_new failed", 1); return ""; }
    // initialize context
    if(EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // set padding
    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) { // RSA_PKCS1_PADDING for PKCS#1 padding, RSA_SSLV23_PADDING for SSLv23 padding, RSA_NO_PADDING for no padding, RSA_PKCS1_OAEP_PADDING for OAEP padding (encrypt and decrypt only), RSA_X931_PADDING for X9.31 padding (signature operations only) and RSA_PKCS1_PSS_PADDING (sign and verify only)
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // determine buffer length
    if(EVP_PKEY_encrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char *>(in.c_str()), inlen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // malloc
    out = static_cast<unsigned char *>(OPENSSL_malloc(outlen));
    if(!out) { neroshop::print("OPENSSL_malloc failed", 1); return ""; }
    // encrypt plain text
    if(EVP_PKEY_encrypt(ctx, out, &outlen, reinterpret_cast<const unsigned char *>(in.c_str()), inlen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // free context
    EVP_PKEY_CTX_free(ctx);
    // Encrypted data is outlen bytes written to buffer out
    //std::cout << "cipher_text (encrypted): " << out << " (" << outlen << ")" << std::endl << std::endl;
    std::string cipher_text = std::string(reinterpret_cast<const char*>(out), outlen);
    // free output buffer
    OPENSSL_free(out);
    // return cipher text
    return cipher_text;
}
//------------------
// refer to: https://www.openssl.org/docs/man3.0/man3/EVP_PKEY_decrypt.html
std::string neroshop::crypto::rsa_decrypt_message(const EVP_PKEY * key, const std::string& in) {
    unsigned char * out;
    size_t outlen, inlen = in.size();
    ENGINE * eng = nullptr;
    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new(const_cast<EVP_PKEY *>(key), eng);
    if(!ctx) { neroshop::print("EVP_PKEY_CTX_new failed", 1); return ""; }
    // initialize context
    if(EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // set padding
    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) { // RSA_PKCS1_PADDING for PKCS#1 padding, RSA_SSLV23_PADDING for SSLv23 padding, RSA_NO_PADDING for no padding, RSA_PKCS1_OAEP_PADDING for OAEP padding (encrypt and decrypt only), RSA_X931_PADDING for X9.31 padding (signature operations only) and RSA_PKCS1_PSS_PADDING (sign and verify only)
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // determine buffer length
    if(EVP_PKEY_decrypt(ctx, nullptr, &outlen, reinterpret_cast<const unsigned char *>(in.c_str()), inlen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // malloc
    out = static_cast<unsigned char *>(OPENSSL_malloc(outlen));
    if(!out) { neroshop::print("OPENSSL_malloc failed", 1); return ""; }
    // decrypt cipher text - error found here
    if(EVP_PKEY_decrypt(ctx, out, &outlen, reinterpret_cast<const unsigned char *>(in.c_str()), inlen) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string("EVP_PKEY_decrypt ") + std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return "";
    }
    // free context
    EVP_PKEY_CTX_free(ctx);
    // Decrypted data is outlen bytes written to buffer out
    //std::cout << "plain_text (decrypted): " << out << " (" << outlen << ")" << std::endl << std::endl;
    std::string plain_text = std::string(reinterpret_cast<const char*>(out), outlen);
    // free output buffer
    OPENSSL_free(out);
    // return plain text
    return plain_text;
}
//------------------
std::string neroshop::crypto::rsa_public_encrypt(const std::string& public_key, const std::string& plain_text) {
    // write the public key to a BIO
    BIO * bio_public = BIO_new(BIO_s_mem());
    if(BIO_write(bio_public, public_key.c_str(), public_key.length()) <= 0) {
        neroshop::print("BIO_write failed", 1);
        return "";
    }
    // store the public key in a EVP_PKEY*
    EVP_PKEY * pkey = PEM_read_bio_PUBKEY(bio_public, nullptr, nullptr, nullptr); // or // EVP_PKEY * pkey = nullptr; PEM_read_bio_PUBKEY(bio_public, &pkey, nullptr, nullptr);
    if(pkey == nullptr) {
        BIO_free_all(bio_public);
        neroshop::print("PEM_read_bio_PUBKEY failed", 1); 
        return ""; 
    }
    // free the bio since we no longer need it
    BIO_free_all(bio_public);
    // now time for encryption
    std::string cipher_text = rsa_encrypt_message(pkey, plain_text);
#ifdef NEROSHOP_DEBUG0
    std::cout << "message encrypted with public key: " << std::endl << get_public_key(pkey) << std::endl;
#endif    
    // free the pkey now that we are done with it
    EVP_PKEY_free(pkey);
    // return the cipher text
    return cipher_text;
}
//------------------
std::string neroshop::crypto::rsa_private_decrypt(const std::string& private_key, const std::string& cipher_text) {
    // write the private key to a BIO
    BIO * bio_private = BIO_new(BIO_s_mem());
    if(BIO_write(bio_private, private_key.c_str(), private_key.length()) <= 0) {
        neroshop::print("BIO_write failed", 1);
        return "";
    }    
    // store the private key in a EVP_PKEY*
    EVP_PKEY * pkey = PEM_read_bio_PrivateKey(bio_private, nullptr, nullptr, nullptr);
    if(pkey == nullptr) { 
        BIO_free_all(bio_private);
        neroshop::print("PEM_read_bio_PrivateKey failed", 1); 
        return ""; 
    }
    // free the bio since we no longer need it
    BIO_free_all(bio_private);
    // now time for decryption
    std::string plain_text = rsa_decrypt_message(pkey, cipher_text);
#ifdef NEROSHOP_DEBUG0
    std::cout << "message decrypted with private key: " << std::endl << get_private_key(pkey) << std::endl;
#endif    
    // free the pkey now that we are done with it
    EVP_PKEY_free(pkey);
    // return the plain text
    return plain_text;
}
//------------------
//------------------
void neroshop::crypto::rsa_public_encrypt_fp(const std::string& public_key, const std::string& plain_text, std::ofstream& file) {
    // encrypt plain text
    std::string cipher_text = rsa_public_encrypt(public_key, plain_text);
#ifdef NEROSHOP_DEBUG    
    std::cout << "message (encrypted): " << cipher_text << std::endl;
#endif    
    // store cipher text in file
    ////std::ofstream file ("cipher_text.txt", std::ios::binary);
    if(!file.is_open()) {
        neroshop::print("public_encrypt_fp error: failed to open file", 1);
        return;
    }
    // write to file
    file << cipher_text;
    file.close();
}
//------------------
void neroshop::crypto::rsa_private_decrypt_fp(const std::string& private_key, std::string& plain_text, std::ifstream& file) {
    // load cipher text from file    
    ////std::ifstream file ("cipher_text.txt", std::ios::binary);
    if(!file.is_open()) { 
        neroshop::print("private_decrypt_fp error: failed to open file", 1); 
        plain_text.clear();
        return;
    }
    // read from file
    std::stringstream cipher_text_ss;
    cipher_text_ss << file.rdbuf(); // dump file contents
    file.close();
    std::string cipher_text = cipher_text_ss.str();//std::cout << "message (encrypted - file): " << cipher_text << std::endl;
    // decrypt cipher text then set the plain text
    plain_text = rsa_private_decrypt(private_key, cipher_text);
}
//------------------
//------------------
std::string neroshop::crypto::rsa_get_public_key(const EVP_PKEY * pkey) {
    BIO * out = BIO_new(BIO_s_mem());
    if(PEM_write_bio_PUBKEY(out, const_cast<EVP_PKEY *>(pkey)) != 1) {
    //if(PEM_write_bio_RSAPublicKey(out, const_cast<RSA *>(rsa)) != 1) { // deprecated in OpenSSL 3.0
        neroshop::print("PEM_write_bio_RSAPublicKey failed", 1);
        BIO_free_all(out);
        return ""; // return empty string on failure
    }
    // copy the output to the memory buffer
    BUF_MEM * mem = nullptr;
    BIO_get_mem_ptr(out, &mem);
    std::string public_key(mem->data, mem->length);
    // free the bio now that we are done writing to it
    BIO_free_all(out);
    return public_key;
}
//------------------
std::string neroshop::crypto::rsa_get_private_key(const EVP_PKEY * pkey) {
    BIO	* out = BIO_new(BIO_s_mem());
    if(PEM_write_bio_PKCS8PrivateKey(out, const_cast<EVP_PKEY *>(pkey), nullptr, nullptr, 0, nullptr, nullptr) != 1) { // same as PEM_write_bio_PrivateKey - both use PKCS#8 format which supports all algorithms including RSA // to-do: add encryption e.g: EVP_aes_256_cbc() (in arg 3) using a passphrase/password (in arg 4) and passphrase_len (in arg 5)
    //if(PEM_write_bio_RSAPrivateKey(bp_private, const_cast<RSA *>(rsa), nullptr, nullptr, 0, nullptr, nullptr) != 1) { // deprecated in OpenSSL 3.0
        neroshop::print("PEM_write_bio_PKCS8PrivateKey failed", 1);
        BIO_free_all(out);
        return ""; // return empty string on failure
    }    
    // copy the output to the memory buffer
    BUF_MEM * mem = nullptr;
    BIO_get_mem_ptr(out, &mem);
    std::string private_key(mem->data, mem->length);
    // free the bio now that we are done writing to it
    BIO_free_all(out);
    return private_key;
}
//------------------
std::pair<std::string, std::string> neroshop::crypto::rsa_get_keys(const EVP_PKEY * pkey) {
    std::string public_key = rsa_get_public_key(pkey);
    std::string private_key = rsa_get_private_key(pkey);
    auto key_pair = std::make_pair(public_key, private_key);
    return key_pair;
}
//------------------
// refer to: https://www.openssl.org/docs/man3.0/man3/EVP_PKEY_sign.html
//           https://www.openssl.org/docs/man3.0/man3/EVP_PKEY_verify.html
std::string neroshop::crypto::rsa_sign_message(const EVP_PKEY *key, const std::string &message)
{
    // NB: assumes signing_key and md are set up before the next
    // step. signing_key must be an RSA private key and md must
    // point to the SHA-256 digest to be signed.

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(const_cast<EVP_PKEY *>(key), NULL); // NULL=no engine
    if (!ctx) {
        neroshop::print("EVP_PKEY_CTX_new failed", 1);
        return NULL;
    } //"";

    if (EVP_PKEY_sign_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }
    if (EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    unsigned char digest[SHA256_DIGEST_LENGTH];
    unsigned int digest_len = 0;
    EVP_MD_CTX * sha256_ctx = EVP_MD_CTX_new();
    if(sha256_ctx == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return NULL; }
    if(EVP_DigestInit_ex(sha256_ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    if(EVP_DigestUpdate(sha256_ctx, message.c_str(), message.size()) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    if(EVP_DigestFinal_ex(sha256_ctx, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    std::size_t siglen = 0;
    // Determine buffer length (siglen)
    if (EVP_PKEY_sign(ctx, NULL, &siglen, digest, SHA256_DIGEST_LENGTH) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }
    unsigned char *sig = reinterpret_cast<unsigned char *>(OPENSSL_malloc(siglen));
    if (!sig) {
        neroshop::print("OPENSSL_malloc failed", 1);
        return NULL;
    } //""; }

    if (EVP_PKEY_sign(ctx, sig, &siglen, digest, SHA256_DIGEST_LENGTH) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }
    // Free context
    EVP_PKEY_CTX_free(ctx);
    std::string signature = std::string(reinterpret_cast<const char *>(sig), siglen);
    // Free signature output buffer
    OPENSSL_free(sig);
    return signature;
}
//------------------
bool neroshop::crypto::rsa_verify_signature(const EVP_PKEY * verify_key, const std::string& message, const std::string& signature)
{
    // NB: assumes verify_key, sig, siglen md and mdlen are already set up
    // and that verify_key is an RSA public key

    EVP_PKEY_CTX * ctx = EVP_PKEY_CTX_new(const_cast<EVP_PKEY *>(verify_key), NULL); //NULL=no engine
    if(!ctx) {
        neroshop::print("EVP_PKEY_CTX_new failed", 1);
        return false;
    }
    // Error occurred
    if(EVP_PKEY_verify_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return false;
    }
    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return false;
    }
    if(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return false;
    }

    unsigned char digest[SHA256_DIGEST_LENGTH];
    unsigned int digest_len = 0;
    EVP_MD_CTX * sha256_ctx = EVP_MD_CTX_new();
    if(sha256_ctx == nullptr) { neroshop::print("EVP_MD_CTX_new failed", 1); return NULL; }
    if(EVP_DigestInit_ex(sha256_ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    if(EVP_DigestUpdate(sha256_ctx, message.c_str(), message.size()) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    if(EVP_DigestFinal_ex(sha256_ctx, digest, &digest_len) != 1) {
        EVP_MD_CTX_free(sha256_ctx);
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return NULL; //"";
    }

    // Perform operation
    int ret = EVP_PKEY_verify(ctx,
                              reinterpret_cast<const unsigned char *>(signature.c_str()),
                              signature.size(),
                              digest,
                              SHA256_DIGEST_LENGTH);
    // ret == 1 indicates success, 0 verify failure and < 0 for some
    // other error.
    if(ret != 1) {
        EVP_PKEY_CTX_free(ctx);
        neroshop::print(std::string(ERR_error_string(ERR_get_error(), nullptr)), 1);
        return false;
    }
    // Free context
    EVP_PKEY_CTX_free(ctx);
    return true;
}
//------------------
//------------------
std::string neroshop::crypto::rsa_private_sign(const std::string& private_key, const std::string& message) {
    // Write the private key to a BIO
    BIO * bio_private = BIO_new(BIO_s_mem());
    if(BIO_write(bio_private, private_key.c_str(), private_key.length()) <= 0) {
        neroshop::print("BIO_write failed", 1);
        return "";
    }    
    // Store the private key in a EVP_PKEY*
    EVP_PKEY * pkey = PEM_read_bio_PrivateKey(bio_private, nullptr, nullptr, nullptr);
    if(pkey == nullptr) { 
        BIO_free_all(bio_private);
        neroshop::print("PEM_read_bio_PrivateKey failed", 1); 
        return ""; 
    }
    // Free the bio since we no longer need it
    BIO_free_all(bio_private);
    // We will now get the signature
    std::string signature = rsa_sign_message(pkey, message);
    // Free the pkey now that we are done with it
    EVP_PKEY_free(pkey);    
    // Return the signature
    return signature;
}
//------------------
bool neroshop::crypto::rsa_public_verify(const std::string& public_key, const std::string& message, const std::string& signature) {
    // Write the public key to a BIO
    BIO * bio_public = BIO_new(BIO_s_mem());
    if(BIO_write(bio_public, public_key.c_str(), public_key.length()) <= 0) {
        neroshop::print("BIO_write failed", 1);
        return false;
    }
    // Store the public key in a EVP_PKEY*
    EVP_PKEY * pkey = PEM_read_bio_PUBKEY(bio_public, nullptr, nullptr, nullptr); // or // EVP_PKEY * pkey = nullptr; PEM_read_bio_PUBKEY(bio_public, &pkey, nullptr, nullptr);
    if(pkey == nullptr) {
        BIO_free_all(bio_public);
        neroshop::print("PEM_read_bio_PUBKEY failed", 1); 
        return false;
    }
    // Free the bio since we no longer need it
    BIO_free_all(bio_public);
    // We will now verify the signature
    bool verified = rsa_verify_signature(pkey, message, signature);
    // Free the pkey now that we are done with it
    EVP_PKEY_free(pkey);
    // Return the verification result
    return verified;
}

/*int main() {
    EVP_PKEY * pkey = neroshop::crypto::rsa_generate_keys_get();
    std::string public_key = neroshop::crypto::rsa_get_public_key(pkey);
    std::string private_key = neroshop::crypto::rsa_get_private_key(pkey);
    std::cout << "public key: \n" << public_key << "\n\n";
    std::cout << "private key: \n" << private_key << "\n\n";
    
    std::string cipher_text = neroshop::crypto::rsa_public_encrypt(public_key, "Turtles are cool");
    std::cout << "cipher text: " << cipher_text << "\n\n";
    std::string plain_text = neroshop::crypto::rsa_private_decrypt(private_key, cipher_text);
    std::cout << "plain text: " << plain_text << "\n";
    
    return 0;
} // g++ rsa.cpp -lcrypto -lssl
*/
