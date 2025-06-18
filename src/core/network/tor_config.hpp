#pragma once

#ifndef TOR_CONFIG_HPP_NEROSHOP
#define TOR_CONFIG_HPP_NEROSHOP

#include <cstdint> // uint16_t
#include <filesystem>
#include <iostream>
#include <string>

#include "../../neroshop_config.hpp"

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

class TorConfig {
public:
    static void create_torrc(const std::string& torrc_path, const std::string& hidden_service_dir, uint16_t hidden_service_port);

private:
    static bool is_file_empty(const std::string& file_path);
};

}

#endif
