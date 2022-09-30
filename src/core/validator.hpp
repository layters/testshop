#ifndef VALIDATOR_HPP_NEROSHOP
#define VALIDATOR_HPP_NEROSHOP

#if defined(NEROSHOP_USE_LIBBCRYPT)
#include "bcrypt.h"
#endif
#include <openssl/evp.h>
#include <openssl/err.h>

#include <iostream>
#include <string>
#include <regex> // std::regex
#include <cctype> // isspace, isalpha, isalnum, etc.
//#include <fstream> // std::ofstream, std::ifstream
#include <chrono> // std::chrono
#include <iomanip> //std::put_time, std::setfill, std::setw

#include "database.hpp"
#include "config.hpp"
#include "util.hpp"

namespace neroshop {
class Validator {// Authenticator {
public:
    static bool register_user(const std::string& username, const std::string& password, const std::string& confirm_pw, std::string opt_email = "");
    static bool login(const std::string& username, const std::string& password);
    static bool login_with_email(const std::string& email, const std::string& password);
    static void save_user(const std::string& username, const char * pw_hash/*const char pw_hash[BCRYPT_HASHSIZE]*/, std::string email_hash = "");
    static void change_pw(const std::string& old_pw, const std::string& new_pw, const std::string& confirm_new_pw);
    // boolean
    static bool validate_username(const std::string& username);
    static bool validate_password(const std::string& password);
    static bool validate_email(const std::string& email);
    #if defined(NEROSHOP_USE_LIBBCRYPT)
    static bool validate_bcrypt_hash(const std::string& password, const std::string& hash); // bycrypt only takes 55-72 character strings // See https://security.stackexchange.com/questions/6623/pre-hash-password-before-applying-bcrypt-to-avoid-restricting-password-length
    static bool generate_bcrypt_salt(unsigned int workfactor, char salt[BCRYPT_HASHSIZE]);
    static bool generate_bcrypt_hash(const std::string& password, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE]);
    #endif
    static bool validate_sha256_hash(const std::string& email, const std::string& hash); // emails do not need to be salted since they are not required for logins
    static bool generate_sha256_hash(const std::string& email, std::string& hash_out);
private:
};
}
#endif
