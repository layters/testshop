#pragma once

#ifndef TOR_MANAGER_HPP_NEROSHOP
#define TOR_MANAGER_HPP_NEROSHOP

#include <cstdint> // uint16_t
#include <filesystem>
#include <iostream>
#include <string>
#include <atomic>

#include <subprocess.h>

#include "../../neroshop_config.hpp" // get_default_config_path(), NEROSHOP_P2P_DEFAULT_PORT

inline const std::filesystem::path TOR_HIDDEN_SERVICE_DIR_FOLDER_NAME{"hidden_service"};
inline constexpr uint16_t TOR_HIDDEN_SERVICE_PORT = NEROSHOP_P2P_DEFAULT_PORT;
inline const std::filesystem::path TOR_TORRC_FILENAME{"torrc"}; // Will be stored in get_default_config_path() + "/tor"

namespace neroshop {

inline std::filesystem::path get_default_tor_path() {
    return std::filesystem::path(get_default_config_path()) / "tor";
}

inline std::filesystem::path get_hidden_service_dir_path() {
    return get_default_tor_path() / TOR_HIDDEN_SERVICE_DIR_FOLDER_NAME;
}

class TorManager {
public:
    explicit TorManager(uint16_t socks_port = 9050);
    ~TorManager();
    
    void start_tor();
    void stop_tor();
    
    void set_socks_port(uint16_t socks_port); // Call BEFORE start_tor()
    
    uint16_t get_socks_port() const;
    int get_bootstrap_progress() const;
    std::string get_onion_address() const;
    
    bool is_tor_running() const;
    bool is_tor_ready() const; // Bootstrap at 100%
private:
    void create_torrc(const std::string& torrc_path, const std::string& hidden_service_dir, uint16_t hidden_service_port);
    void poll_hostname();
    bool is_socks_port_same_as_torrc(uint16_t socks_port);
    std::atomic<int> bootstrap_progress{0};  // 0-100%
    std::string onion_address;
    std::string tor_binary;
    std::string base_dir;
    std::filesystem::path hs_dir;
    std::filesystem::path data_dir;
    uint16_t socks_port;
    uint16_t control_port;
    std::string torrc_path;
    bool is_running = false;
    struct subprocess_s tor_proc;  // Store process handle
};

}

#endif
