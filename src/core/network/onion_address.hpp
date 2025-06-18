#pragma once

#ifndef ONION_ADDRESS_HPP_NEROSHOP
#define ONION_ADDRESS_HPP_NEROSHOP

#include <filesystem>
#include <iostream>
#include <string>

namespace neroshop {

class OnionAddressGenerator {
public:
    OnionAddressGenerator(const std::string& mkp224o_path = "./mkp224o");
    
    bool load(/*const std::string& in_dir*/);
    std::string generate(const std::string& prefix, const std::string& out_dir); // Generate an onion address with the given prefix, output to out_dir. Returns the generated onion address (string)
    
    std::filesystem::path get_onion_dir() const; // Get the path to the generated onion service directory (contains keys)
    std::string get_onion_address() const; // Get the onion address
    std::filesystem::path get_secret_key_path() const; // Get the path to the private key file
    std::filesystem::path get_public_key_path() const; // Get the path to the public key file
    
    bool is_generated() const;

private:
    std::string mkp224o_path_;
    std::filesystem::path onion_dir_;
    std::string onion_addr_;
    bool generated_;
};

}

#endif
