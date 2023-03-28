#pragma once
#ifndef VALIDATOR_HPP_NEROSHOP
#define VALIDATOR_HPP_NEROSHOP

#include <openssl/evp.h>
#include <openssl/err.h>

#include <iostream>
#include <string>

namespace neroshop {
class Validator {
public:
    // boolean
    static bool validate_username(const std::string& username);
    static bool validate_password(const std::string& password);
    static bool validate_email(const std::string& email);

    static bool validate_sha256_hash(const std::string& email, const std::string& hash); // emails do not need to be salted since they are not required for logins
    static bool generate_sha256_hash(const std::string& email, std::string& hash_out);
private:
};
}
#endif
