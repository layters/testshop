// filename: validator.hpp
#ifndef VALIDATOR_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define VALIDATOR_HPP_NEROSHOP

#include <iostream>
#include <string>
#include <regex> // std::regex
#include <cctype> // isspace, isalpha, isalnum, etc.
//#include <fstream> // std::ofstream, std::ifstream
#include <chrono> // std::chrono
#include <iomanip> //std::put_time, std::setfill, std::setw
// neroshop
#include "database.hpp"
// dokun
#include <string.hpp>
////extern "C" {
// libbcrypt
#include "bcrypt.h"
//#include "crypt_blowfish/ow-crypt.h"
// openssl - monero uses openssl so we just have to link this code to monero
#include <openssl/evp.h>
#include <openssl/err.h>
////}

namespace neroshop {
class Validator {
public:
    static bool register_user(const std::string& username, const std::string& password, const std::string& confirm_pw, std::string opt_email = "");
    static bool login(const std::string& username, const std::string& password);
    static bool login_with_email(const std::string& email, const std::string& password);
    static void save_user(const std::string& username, const char pw_hash[BCRYPT_HASHSIZE], std::string email_hash = "");
    static void change_pw(const std::string& old_pw, const std::string& new_pw/*, const std::string& confirm_new_pw*/);
    // boolean
    static bool validate_username(const std::string& username); // for registration
    static bool validate_password(const std::string& password); // for registration
    static bool validate_email(const std::string& email);
    static bool validate_bcrypt_hash(const std::string& password, const std::string& hash); // for login // bycrypt only takes 55-72 character strings // https://security.stackexchange.com/questions/6623/pre-hash-password-before-applying-bcrypt-to-avoid-restricting-password-length
    static bool generate_bcrypt_salt(unsigned int workfactor, char salt[BCRYPT_HASHSIZE]);
    static bool generate_bcrypt_hash(const std::string& password, const char salt[BCRYPT_HASHSIZE], char hash[BCRYPT_HASHSIZE]);
    static bool validate_sha256_hash(const std::string& email, const std::string& hash); // emails do not need to be salted since they are not required for logins
    static bool generate_sha256_hash(const std::string& email, std::string& hash_out);
private:
};
}
#endif
/*
testing speeds:
$ openssl speed sha256 sha512

usernames:
// 2-30 characters in length
// cannot contain any spaces
// cannot contain any symbols except: -, _, and .
// can only begin with a letter
// can only end with a letter or number

passwords:
// at least one upper case English letter, (?=.*?[A-Z])
// at least one lower case English letter, (?=.*?[a-z])
// at least one digit, (?=.*?[0-9])
// at least one special character, (?=.*?[#?!@$%^&*-])
// minimum eight in length .{8,} (with the anchors) 
*/
