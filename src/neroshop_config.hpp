// Copyright (c) 2021-2024 the neroshop core team
// Distributed under the GPLv3 software license, see the accompanying
// file LICENSE or https://opensource.org/license/gpl-3-0/.

#pragma once

#ifndef CONFIG_HPP_NEROSHOP
#define CONFIG_HPP_NEROSHOP

#include <cstdint>
#include <string>
#include <chrono>
#include <initializer_list>

#if defined(NEROSHOP_USE_QT)
#include <QStandardPaths>
#else
#include "core/tools/device.hpp"
#endif

// Network ports
inline constexpr uint16_t NEROSHOP_IPC_DEFAULT_PORT = 50880;
inline constexpr uint16_t NEROSHOP_P2P_DEFAULT_PORT = 50881; // This port will be used by the daemon to establish connections with p2p network // Use ports between 49152-65535 that are not currently registered with IANA and are rarely used
inline constexpr uint16_t NEROSHOP_RPC_DEFAULT_PORT = 50882; // This port will allow outside clients to interact with neroshop daemon RPC server

// IP Addresses
inline constexpr const char* NEROSHOP_LOOPBACK_ADDRESS = "127.0.0.1";
inline constexpr const char* NEROSHOP_ANY_ADDRESS      = "0.0.0.0";

// Buffer size
inline constexpr size_t NEROSHOP_RECV_BUFFER_SIZE      = 4096;//8192// no IP packet can be above 64000 (64 KB), not even with fragmentation, thus recv on an UDP socket can at most return 64 KB (and what is not returned is discarded for the current packet!)

// DHT constants - timeouts
inline constexpr int NEROSHOP_DHT_RECV_TIMEOUT            = 5000; // Measured in milliseconds
inline constexpr int NEROSHOP_DHT_PING_TIMEOUT            = 5000; // Measured in milliseconds (I2P should use 800-1000ms and Tor should use 5000-10000ms)

// DHT constants
inline constexpr int NEROSHOP_DHT_REPLICATION_FACTOR      = 3;    // 10 to 20 (or even higher) // Usually 3 or 5 but a higher number would improve fault tolerant, mitigating the risk of data loss even if multiple nodes go offline simultaneously. It also helps distribute the load across more nodes, potentially improving read performance by allowing concurrent access from multiple replicas.
inline constexpr int NEROSHOP_DHT_MAX_CLOSEST_NODES       = 20;   // 50 to 100 (or even higher)
inline constexpr int NEROSHOP_DHT_ROUTING_TABLE_BUCKETS   = 256;  // Recommended to use a number of buckets that is equal to the number of bits in the node id (in this case, sha-3-256 so 256 bits)
inline constexpr int NEROSHOP_DHT_NODES_PER_BUCKET        = 20;   // Each bucket should hold up to 20 nodes (same number as k closest nodes)
inline constexpr int NEROSHOP_DHT_MAX_ROUTING_TABLE_NODES = 
    NEROSHOP_DHT_ROUTING_TABLE_BUCKETS * NEROSHOP_DHT_NODES_PER_BUCKET; // = 5120
inline constexpr int NEROSHOP_DHT_MAX_HEALTH_CHECKS       = 3;    // Maximum number of consecutive failed checks before marking the node as dead

// DHT intervals
inline constexpr std::chrono::seconds NEROSHOP_DHT_NODE_HEARTBEAT_INTERVAL{300};  // Number of seconds between each node health check
inline constexpr std::chrono::seconds NEROSHOP_DHT_BUCKET_REFRESH_INTERVAL{3600}; // Number of seconds between each find_node query to neighboring nodes
inline constexpr std::chrono::seconds NEROSHOP_DHT_DATA_REPUBLISH_INTERVAL{3600}; // Number of seconds between each republishing of in-memory hash table data
inline constexpr std::chrono::seconds NEROSHOP_DHT_DATA_REMOVAL_INTERVAL  {1800}; // Number of seconds between each removal of all expired in-memory hash table data

// Search result limit
inline constexpr int NEROSHOP_MAX_SEARCH_RESULTS          = 1000;

// Key filenames
inline constexpr const char* NEROSHOP_PRIVATE_KEY_FILENAME             = "<user_id>.key";
inline constexpr const char* NEROSHOP_PUBLIC_KEY_FILENAME              = "<user_id>.pub";
inline constexpr const char* NEROSHOP_OPENSSL_PRIVATE_KEY_FILENAME     = "<user_id>.pem";
inline constexpr const char* NEROSHOP_PGP_PRIVATE_KEY_FILENAME         = "<user_id>.pgp";
inline constexpr const char* NEROSHOP_PGP_ARMORED_PRIVATE_KEY_FILENAME = "<user_id>.asc";

// RSA key bits
inline constexpr int NEROSHOP_RSA_DEFAULT_BIT_LENGTH = 4096;
inline constexpr int NEROSHOP_RSA_DEFAULT_BITS       = NEROSHOP_RSA_DEFAULT_BIT_LENGTH;

// App and file names
inline constexpr const char* NEROSHOP_APPLICATION_NAME       = "neroshop";
inline constexpr const char* NEROSHOP_DAEMON_CONFIG_FILENAME = "daemon.ini";
inline constexpr const char* NEROSHOP_DATABASE_FILENAME      = "data.sqlite3";
inline constexpr const char* NEROSHOP_SETTINGS_FILENAME      = "settings.json";
inline constexpr const char* NEROSHOP_NODES_FILENAME         = "nodes.lua";
inline constexpr const char* NEROSHOP_LOG_FILENAME           = "neroshop.log";
inline constexpr const char* NEROSHOP_USERDATA_FILENAME      = "client.sqlite3";

// Folder names
inline constexpr const char* NEROSHOP_KEYS_FOLDER_NAME    = "keys";
inline constexpr const char* NEROSHOP_DATA_FOLDER_NAME    = "datastore";
inline constexpr const char* NEROSHOP_CATALOG_FOLDER_NAME = "listings";
inline constexpr const char* NEROSHOP_AVATAR_FOLDER_NAME  = "users";

// Image upload
inline constexpr size_t NEROSHOP_MAX_IMAGE_SIZE = 2 * 1024 * 1024; // 2 MB

// Username limits
inline constexpr int NEROSHOP_MIN_USERNAME_LENGTH = 2;
inline constexpr int NEROSHOP_MAX_USERNAME_LENGTH = 30;

namespace neroshop {

inline std::string get_default_config_path() {
    #if defined(NEROSHOP_USE_QT)
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString(); // or (QStandardPaths::AppLocalDataLocation).toStdString();
    #else // #if !defined(NEROSHOP_USE_QT)
    #if defined(_WIN32)
    return "C:/Users/" + neroshop::device::get_user() + "/AppData/Local/neroshop"; // or "C:/ProgramData/neroshop";
    #endif
    #if defined(__linux__) && !defined(__ANDROID__)
    return "/home/" + neroshop::device::get_user() + "/.config/neroshop"; // or "/etc/xdg/neroshop" or "/home/" + neroshop::device::get_user() + "/.local/share/neroshop";
    #endif
    #if defined(__APPLE__) && defined(__MACH__)
    return "~/Library/Preferences/neroshop";
    #endif
    #if defined(__ANDROID__)
    return "<APPROOT>/files/settings";
    #endif
    #if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
    return "<APPROOT>/Library/Preferences/neroshop";
    #endif
    #endif
    #endif
}

inline std::string get_default_database_path() {
    return get_default_config_path() + "/" + NEROSHOP_DATA_FOLDER_NAME;
}

inline std::string get_default_keys_path() {
    return get_default_config_path() + "/" + NEROSHOP_KEYS_FOLDER_NAME;
}

inline std::string get_default_wallet_path() {
    #if defined(NEROSHOP_USE_QT)
    #if defined(_WIN32)
    return (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/neroshop/wallet").toStdString();
    #else
    return (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop/wallet").toStdString();
    #endif
    #else // #if !defined(NEROSHOP_USE_QT)
    #if defined(_WIN32)
    return "C:/Users/" + neroshop::device::get_user() + "/Documents" + "/neroshop/wallet";
    #endif
    #if defined(__linux__) && !defined(__ANDROID__)
    return "/home/" + neroshop::device::get_user() + "/neroshop/wallet";
    #endif
    #endif
}

struct BootstrapNode {
    std::string address;
    uint16_t port;
};

inline const std::initializer_list<BootstrapNode> BOOTSTRAP_I2P_NODES = {
    { "pbdcncqgmgnan67ej72o7obbuvpy3wd3vhyyh4xty2luxbgmmyaq.b32.i2p", NEROSHOP_P2P_DEFAULT_PORT }
};

inline const std::initializer_list<BootstrapNode> BOOTSTRAP_TOR_NODES = {
    { "testkdb44e3v5bh2svemcwnghh4ns372yzyzmqke65kahryoqb565pid.onion", NEROSHOP_P2P_DEFAULT_PORT },
    { "k33yv63yezwur5n2mbuqrb64iwdovnpw7lsuuursu35hvvj24myx2ryd.onion", NEROSHOP_P2P_DEFAULT_PORT }
};

}

#endif
