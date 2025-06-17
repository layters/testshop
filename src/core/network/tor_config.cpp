#include "tor_config.hpp"

namespace neroshop {

//-----------------------------------------------------------------------------

void TorConfig::create_torrc(const std::string& torrc_path, const std::string& hidden_service_dir, uint16_t hidden_service_port) {
    // Check if file is empty
    bool file_empty = is_file_empty(torrc_path);
    
    std::ofstream torrc_file(torrc_path, std::ios::app); // Open in append mode (in case we want to add more hidden services)
    if (!torrc_file) {
        throw std::runtime_error("Failed to open torrc file for writing");
    }
    // If file is not empty, add a newline before appending new hidden service
    if (!file_empty) {
        torrc_file << "\n";
    }
    torrc_file << "HiddenServiceDir " << hidden_service_dir << "\n";
    torrc_file << "HiddenServicePort " << hidden_service_port << " 127.0.0.1:" << hidden_service_port << "\n";
    torrc_file.close();
    
    // Optionally restart Tor daemon with the following command and args: ./tor -f ~/.config/neroshop/tor/torrc
}

//-----------------------------------------------------------------------------

bool TorConfig::is_file_empty(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file) {
        // File doesn't exist or can't be opened, treat as empty
        return true;
    }
    return (file.peek() == std::ifstream::traits_type::eof());
}

//-----------------------------------------------------------------------------

}
