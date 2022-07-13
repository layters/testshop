// filename: encryptor.hpp
#ifndef ENCRYPTOR_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define ENCRYPTOR_HPP_NEROSHOP

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility> // std::pair
// neroshop
#include "debug.hpp"
////extern "C" {
// openssl - monero uses openssl so we just have to link this code to monero
#include <openssl/rsa.h> // for creating key pairs, encrypting and decrypting messages (with RSA)
#include <openssl/pem.h> //  includes "openssl/evp.h" which includes "openssl/bio.h"
#include <openssl/err.h>
////}
//# define AES_BLOCK_SIZE 16

namespace neroshop {
class Encryptor {
public:
    static bool generate_key_pair();
    static bool generate_key_pair_ex(); // requires openssl 3.0 // the _ex suffix is for extended versions of existing functions//static bool generate_public_key_from_private_key();
    static bool save_public_key(const EVP_PKEY * pkey);
    static bool save_private_key(const EVP_PKEY * pkey);
    static bool save_key_pair(const EVP_PKEY * pkey);
    static std::string public_encrypt(const std::string& public_key, const std::string& plain_text); // encrypts plain text with a receiver's public key then returns a cipher text otherwise it returns an empty string on failure
    static std::string private_decrypt(const std::string& private_key, const std::string& cipher_text); // decrypts cipher text using the receiver's private key then returns a plain text otherwise it returns an empty string on failure
    static void public_encrypt_fp(const std::string& public_key, const std::string& plain_text_in, std::ofstream& file);
    static void private_decrypt_fp(const std::string& private_key, std::string& plain_text_out, std::ifstream& file);
    static std::string encrypt_message(const EVP_PKEY * key, const std::string& plain_text); // returns encrypted text
    static std::string decrypt_message(const EVP_PKEY * key, const std::string& cipher_text); // returns decrypted text
    // getters
    static std::string get_public_key(const EVP_PKEY * pkey);
    static std::string get_private_key(const EVP_PKEY * pkey);
    static std::pair<std::string, std::string> get_key_pair(const EVP_PKEY * pkey);
private:
};
}
#endif
// 1. alice sends bob a message that is encrypted using bob's public key
// 2. bob receives the encrypted message and must decrpyt the message using his own private key
/* Usage:
Example 1:
    // the public key
    std::string public_key =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4IcJNHFqOofSOaZaijI8\n"
    "AvDA+cYwUEpUWSXZ+NA2AtVObX2htLa3PhSSToQVUABbwPySgaHWL2xGSr/0b0Z4\n"
    "zauCK/VGidYkx85nDsOm0YjcsDUwc5t3WrRKG5+gzqxJi9g5iLaLjVzK2iPqhCTQ\n"
    "R45XtP7XBEIimZltfFW20TPC/jLIMqqKl/tkVQ1aHxqOP9k2DPPswa2JFqXF1Lnk\n"
    "nmKLKHbmn/34CMbbPBwZssailxM+hJOa+KfTRO9nP03m8z0mvRRnxu0oZFx632L9\n"
    "QX8eTHrYwYs6svbWOWE5wmfPKBAkhr94C8ricZXf8B/PAEQrF+aWrjgsLt0xtMtm\n"
    "QQIDAQAB\n"
    "-----END PUBLIC KEY-----";
    // the private key
    std::string private_key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDghwk0cWo6h9I5\n"
    "plqKMjwC8MD5xjBQSlRZJdn40DYC1U5tfaG0trc+FJJOhBVQAFvA/JKBodYvbEZK\n"
    "v/RvRnjNq4Ir9UaJ1iTHzmcOw6bRiNywNTBzm3datEobn6DOrEmL2DmItouNXMra\n"
    "I+qEJNBHjle0/tcEQiKZmW18VbbRM8L+MsgyqoqX+2RVDVofGo4/2TYM8+zBrYkW\n"
	"pcXUueSeYosoduaf/fgIxts8HBmyxqKXEz6Ek5r4p9NE72c/TebzPSa9FGfG7Shk\n"
	"XHrfYv1Bfx5MetjBizqy9tY5YTnCZ88oECSGv3gLyuJxld/wH88ARCsX5pauOCwu\n"
	"3TG0y2ZBAgMBAAECggEBALWrLTx8o+o16VhyDIITAVGTwWCYBpGAgt0a7lIPDhSe\n"
	"yPV4mHWi/YNCm9rhrmjr0VHGSziOXMJERl/HDx1WFPq80feFXwy580qj6+kbT4fs\n"
	"yDve3ZQ874a5p9jQAQoYhu2bB3ph0WqQ8SUtuFwxeUDcoIS3SfyNEnfbl6XpqKF9\n"
	"TrnDINBlWDxc9clvs/3RauJMMPjlOLmtARbBQaZJQYp3LQGRn82/IFTUFxQLBpAJ\n"
	"Q7iaPthg8Rc0rrLqScejg6sNRJXUVJiTfODCcfOCCDq4hKF9aDmp+YCK6KBN04O4\n"
	"TEc9PL9o10CTrQww2qmt/Ci76w8GxrW0VAN5ZZU2kQ0CgYEA/PYMEM+/snsuX1O3\n"
	"Nd83ubnRzf7Me2i6lRcT2OpQoSicHIKAc0VtKYww1iFL+yrFXsgYFBnmkeP4DsPK\n"
	"ZN5mm4XoHeTC4AaH1SSRVdT7nEg6GVK1NbjFtbbEQaz7eKqnnpVniDK3RE3iOLjB\n"
	"nHVWcsZJZlVJjcKAkw5vacGJ6Q8CgYEA4zmLYGXY1qpkc0EQonzBxXUkI8pwfPal\n"
	"6w2/VUdmBmLD0DVRrugnVHnGNLmNk2QXoMqN40vhgD9tdY3BMVBmoRyZLgDpEhXz\n"
	"0XlLiONRkEWsqgzYoC0IG4fSvAsFlPqrXiTg34H/s74CtJ+nLF3hO/HfSkfulM/C\n"
	"dj9OFPgHm68CgYBr4ct3iAJrbhly0lM6iH5NmTAfOGGg6CNa3kK6qgPFF3qstgNu\n"
	"JdfOdlmFmSG8dptCNvf96qXo5l6ufVXd+vOrtEowJZXu0RoxDq1k+7ZrCmqszhc2\n"
	"WB0JyG6ey9VbuvxNp85FyctbOBQYuMLppSk/Pc2j9Q+vg5ouHWPqqH3WhQKBgCF+\n"
	"8SHjwaRbd/VZiRc65uGx1AMGq7BwN6M/4o2yucKFOrJtub3b8ThNvz80fz9UCPum\n"
	"AGaaYAKk1wD2RZ18abSkX5xde/4ziD6/77edMv/elYZ34FM0cDaGvjUENu1wSmTV\n"
	"cOTh6AzaHNH9mwo6SKKqlC0CD5SWT+dYi60hpxV3AoGBAKJbh7ApcUzTvEKofrS7\n"
	"xhzvnwW44cRdHNF25KMD+xkmw/4nrmifDrt+ZT5Zfa2PPBGRNDLP79mpxBnpaNLs\n"
	"LMl2fZN7vg2xY/WurhSmQjl1OmW+wFbYU2Kfmsej1tmtaO4A9xpE5jsu+L5fmrt+\n"
	"y/gd8YWuIVtUTY/HEOWneR/i\n"
	"-----END PRIVATE KEY-----";
    // cipher text
    std::string cipher_text = Encryptor::public_encrypt(public_key, "Turtles are cool");
    std::cout << "message (encrypted): " << cipher_text << std::endl;
    // plain text
    std::string plain_text = Encryptor::private_decrypt(private_key, cipher_text);
    std::cout << "message (decrypted): " << plain_text << std::endl;


Example 2:
    // write plain text to file in cipher text form
    std::ofstream wfile ("cipher_text.txt", std::ios::binary);
    Encryptor::public_encrypt_fp(public_key, "Monero is the king of privacy coins!", wfile);
    // read cipher text from file in plain text form
    std::ifstream rfile ("cipher_text.txt", std::ios::binary);
    std::string plain_text;
    Encryptor::private_decrypt_fp(private_key, plain_text, rfile);
    std::cout << "message (decrypted): " << plain_text << std::endl;
*/
