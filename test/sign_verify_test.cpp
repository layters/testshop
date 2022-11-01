#include <fstream>
#include <iostream>
#include <vector>

#include "../src/core/encryptor.hpp"

EVP_PKEY *read_key_pub(const std::string &filepath)
{
    std::ifstream file(filepath);
    std::string str;
    std::string key;
    while (std::getline(file, str)) {
        key += str;
        key += "\n";
    }

    BIO *bio_public = BIO_new(BIO_s_mem());
    if (BIO_write(bio_public, key.c_str(), key.length()) <= 0) {
        return nullptr;
    }
    return PEM_read_bio_PUBKEY(bio_public, nullptr, nullptr, nullptr);
}

EVP_PKEY *read_key_priv(const std::string &filepath)
{
    std::ifstream file(filepath);
    std::string str;
    std::string key;
    while (std::getline(file, str)) {
        key += str;
        key += "\n";
    }

    BIO *bio_public = BIO_new(BIO_s_mem());
    if (BIO_write(bio_public, key.c_str(), key.length()) <= 0) {
        return nullptr;
    }
    return PEM_read_bio_PrivateKey(bio_public, nullptr, nullptr, nullptr);
}

void test_encrypt_decrypt()
{
    auto pkey = neroshop::Encryptor::generate_key_pair_return();
    const std::string msg = "olla!";
    auto enc = neroshop::Encryptor::encrypt_message(pkey, msg);
    auto dec = neroshop::Encryptor::decrypt_message(pkey, enc);
    std::cout << __FUNCTION__ << " result: " << (dec == msg ? "pass" : "fail") << std::endl;
}

void test_encrypt_decrypt_file()
{
    neroshop::Encryptor::generate_key_pair();
    std::ifstream pub("public.pem");
    std::string str;
    std::string pub_key;
    while (std::getline(pub, str)) {
        pub_key += str;
        pub_key += "\n";
    }
    std::ifstream priv("private.pem");
    std::string priv_key;
    while (std::getline(priv, str)) {
        priv_key += str;
        priv_key += "\n";
    }

    const std::string msg = "olla!";
    auto enc = neroshop::Encryptor::public_encrypt(pub_key, msg);
    auto dec = neroshop::Encryptor::private_decrypt(priv_key, enc);
    std::cout << __FUNCTION__ << " result: " << (dec == msg ? "pass" : "fail") << std::endl;
}

void test_sign_verify()
{
    const std::string msg = "hello";
    auto pkey = neroshop::Encryptor::generate_key_pair_return();
    auto signature = neroshop::Encryptor::sign_message(pkey, msg);
    auto result = neroshop::Encryptor::verify_signature(pkey, msg, signature);
    std::cout << __FUNCTION__ << " result: " << (result == 1 ? "pass" : "fail") << std::endl;
}

int main()
{
    test_encrypt_decrypt();
    test_encrypt_decrypt_file();
    test_sign_verify();
    return 0;
}

