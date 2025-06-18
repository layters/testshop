#include "onion_address.hpp"

#include "tor_config.hpp"
#include "../tools/string.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <chrono>

namespace neroshop {

//-----------------------------------------------------------------------------

OnionAddressGenerator::OnionAddressGenerator(const std::string& mkp224o_path)
    : mkp224o_path_(mkp224o_path), generated_(false)
{
    if (mkp224o_path_.empty()) {
        throw std::invalid_argument("mkp224o path cannot be empty");
    }
}
        
//-----------------------------------------------------------------------------

bool OnionAddressGenerator::load(/*const std::string& in_dir*/) {
    namespace fs = std::filesystem;
    
    auto base_path = get_default_tor_path(); // ~/.config/neroshop/tor
    auto keys_path = base_path / TOR_HIDDEN_SERVICE_DIR_FOLDER_NAME; // ~/.config/neroshop/tor/hidden_service
    auto last_onion_file = keys_path / "last_onion_address.txt"; // ~/.config/neroshop/tor/hidden_service/last_onion_address.txt
    if (!fs::exists(last_onion_file)) {
        std::cerr << "load: last_onion_address.txt not found\n";
        return false;
    }
    
    // Read onion address from "last_onion_address.txt" file
    std::ifstream infile(last_onion_file);
    if (!infile) {
        std::cerr << "load: Failed to open last_onion_address.txt\n";
        return false;
    }
    std::string last_onion_address;
    std::getline(infile, last_onion_address);
    if (last_onion_address.empty()) {
        std::cerr << "load: last_onion_address.txt is empty\n";
        return false;
    }
    // Trim trailing whitespace
    last_onion_address.erase(last_onion_address.find_last_not_of(" \n\r\t") + 1);
    
    // ~/.config/neroshop/tor/hidden_service/<last_onion_address>
    auto onion_folder = keys_path / last_onion_address;
    // Check if the .onion folder exists
    if (!fs::exists(onion_folder)) {
        std::cerr << "load: Onion keys folder does not exist: " << onion_folder << "\n";
        return false;
    }
    
    // Check if the hostname file exists
    auto hostname_path = onion_folder / "hostname";
    if (!fs::exists(hostname_path)) {
        std::cerr << "load: hostname file missing in onion folder: " << hostname_path << "\n";
        return false;
    }
    
    // Open the hostname file
    std::ifstream hostname_file(hostname_path);
    if (!hostname_file) {
        std::cerr << "load: Failed to open hostname file\n";
        return false;
    }
    // Store the hostname in onion_addr_
    std::getline(hostname_file, this->onion_addr_);
    if (this->onion_addr_.empty()) {
        std::cerr << "load: hostname file is empty\n";
        return false;
    }
    this->onion_addr_.erase(this->onion_addr_.find_last_not_of(" \n\r\t") + 1);
    
    // Store the onion_folder in onion_dir_
    this->onion_dir_ = onion_folder;
    std::cout << "load: Loaded existing onion address: " << this->onion_addr_ << std::endl;
    
    // Set generated_ to true
    this->generated_ = true;
    
    return true;
}

//-----------------------------------------------------------------------------

// Generate an onion address with the given prefix, output to out_dir
// Returns the generated onion address (string)
std::string OnionAddressGenerator::generate(const std::string& prefix, const std::string& out_dir) {
    // Can only generate once
    if(generated_) {
        std::cout << "generate: Onion addresses can only be generated once\n";
        return "";
    }
    
    if (out_dir.empty()) {
        throw std::invalid_argument("Output directory cannot be empty");
    }
    // Make sure output directory exists
    std::filesystem::create_directories(out_dir);

    // Build command with -n 1 to generate exactly one matching address
    std::string cmd = mkp224o_path_ + " -d " + out_dir + " -n 1";
    if (!prefix.empty()) {
        cmd += " " + prefix;
    }
    
    std::cout << "generate: Running command: " << cmd << std::endl;

    // Run mkp224o as a subprocess
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        throw std::runtime_error(
            "mkp224o command failed with exit code " + std::to_string(ret) + ": " + cmd);//"mkp224o failed to run or did not find a matching address.");
    }

    // Find the generated onion directory
    std::filesystem::path newest_dir;
    std::filesystem::file_time_type::clock::time_point newest_time;

    for (const auto& entry : std::filesystem::directory_iterator(out_dir)) {
        if (entry.is_directory()) {
            std::string dir_name = entry.path().filename().string();
            if (neroshop::string_tools::ends_with(dir_name, ".onion")) {
                auto ftime = std::filesystem::last_write_time(entry.path());
                if (newest_dir.empty() || ftime > newest_time) {
                    newest_dir = entry.path();
                    newest_time = ftime;
                }
            }
        }
    }

    if (!newest_dir.empty()) {
        onion_dir_ = newest_dir;
        onion_addr_ = newest_dir.filename().string();
        std::cout << "generate: Found onion address directory: " << onion_dir_
                  << std::endl;
        generated_ = true;
        return onion_addr_;
    }

    throw std::runtime_error("No onion address found in output directory.");
}

//-----------------------------------------------------------------------------

// Get the path to the generated onion service directory (contains keys)
std::filesystem::path OnionAddressGenerator::get_onion_dir() const { return onion_dir_; }

//-----------------------------------------------------------------------------

// Get the onion address
std::string OnionAddressGenerator::get_onion_address() const { return onion_addr_; }

//-----------------------------------------------------------------------------

// Get the path to the private key file
std::filesystem::path OnionAddressGenerator::get_secret_key_path() const {
    return onion_dir_ / "hs_ed25519_secret_key";
}

//-----------------------------------------------------------------------------

// Get the path to the public key file
std::filesystem::path OnionAddressGenerator::get_public_key_path() const {
    return onion_dir_ / "hs_ed25519_public_key";
}

//-----------------------------------------------------------------------------

bool OnionAddressGenerator::is_generated() const { return generated_; }

//-----------------------------------------------------------------------------

} // namespace neroshop
