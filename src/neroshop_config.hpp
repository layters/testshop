// Copyright (c) 2021-2024 the neroshop core team
// Distributed under the GPLv3 software license, see the accompanying
// file LICENSE or https://opensource.org/license/gpl-3-0/.

#pragma once

#ifndef CONFIG_HPP_NEROSHOP
#define CONFIG_HPP_NEROSHOP

#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility> // std::pair

#if defined(NEROSHOP_USE_QT)
#include <QStandardPaths>
#else
#include "core/tools/device.hpp"
#endif

#define NEROSHOP_IPC_DEFAULT_PORT 50880
// This port will be used by the daemon to establish connections with p2p network
#define NEROSHOP_P2P_DEFAULT_PORT 50881 // Use ports between 49152-65535 that are not currently registered with IANA and are rarely used
// This port will allow outside clients to interact with neroshop daemon RPC server
#define NEROSHOP_RPC_DEFAULT_PORT 50882

#define NEROSHOP_LOOPBACK_ADDRESS            "127.0.0.1"
#define NEROSHOP_ANY_ADDRESS                 "0.0.0.0"

#define NEROSHOP_RECV_BUFFER_SIZE            4096//8192// no IP packet can be above 64000 (64 KB), not even with fragmentation, thus recv on an UDP socket can at most return 64 KB (and what is not returned is discarded for the current packet!)

#define NEROSHOP_DHT_REPLICATION_FACTOR      3    // 10 to 20 (or even higher) // Usually 3 or 5 but a higher number would improve fault tolerant, mitigating the risk of data loss even if multiple nodes go offline simultaneously. It also helps distribute the load across more nodes, potentially improving read performance by allowing concurrent access from multiple replicas.
#define NEROSHOP_DHT_MAX_CLOSEST_NODES       20   // 50 to 100 (or even higher)
#define NEROSHOP_DHT_RECV_TIMEOUT            2000 // Measured in milliseconds
#define NEROSHOP_DHT_PING_TIMEOUT            800  // Measured in milliseconds
#define NEROSHOP_DHT_ROUTING_TABLE_BUCKETS   256  // Recommended to use a number of buckets that is equal to the number of bits in the node id (in this case, sha-3-256 so 256 bits)
#define NEROSHOP_DHT_NODES_PER_BUCKET        20   // Each bucket should hold up to 20 nodes (same number as k closest nodes)
#define NEROSHOP_DHT_MAX_ROUTING_TABLE_NODES NEROSHOP_DHT_ROUTING_TABLE_BUCKETS * NEROSHOP_DHT_NODES_PER_BUCKET // = 5120
#define NEROSHOP_DHT_MAX_HEALTH_CHECKS          3    // Maximum number of consecutive failed checks before marking the node as dead
#define NEROSHOP_DHT_NODE_HEARTBEAT_INTERVAL    300  // Number of seconds between each node health check
#define NEROSHOP_DHT_BUCKET_REFRESH_INTERVAL    900  // Number of seconds between each find_node query to neighboring nodes
#define NEROSHOP_DHT_DATA_REPUBLISH_INTERVAL    3600 // Number of seconds between each republishing of in-memory hash table data
#define NEROSHOP_DHT_DATA_REMOVAL_INTERVAL      1800 // Number of seconds between each removal of all expired in-memory hash table data

#define NEROSHOP_MAX_SEARCH_RESULTS          1000

#define NEROSHOP_PRIVATE_KEY_FILENAME             "<user_id>.key"
#define NEROSHOP_PUBLIC_KEY_FILENAME              "<user_id>.pub"
#define NEROSHOP_OPENSSL_PRIVATE_KEY_FILENAME     "<user_id>.pem"
#define NEROSHOP_PGP_PRIVATE_KEY_FILENAME         "<user_id>.pgp"
#define NEROSHOP_PGP_ARMORED_PRIVATE_KEY_FILENAME "<user_id>.asc"

#define NEROSHOP_RSA_DEFAULT_BIT_LENGTH 4096
#define NEROSHOP_RSA_DEFAULT_BITS       NEROSHOP_RSA_DEFAULT_BIT_LENGTH

#define NEROSHOP_APPLICATION_NAME       "neroshop"
#define NEROSHOP_DAEMON_CONFIG_FILENAME "daemon.ini"
#define NEROSHOP_DATABASE_FILENAME      "data.sqlite3"
#define NEROSHOP_SETTINGS_FILENAME      "settings.json"
#define NEROSHOP_NODES_FILENAME         "nodes.lua"
#define NEROSHOP_LOG_FILENAME           "neroshop.log"
#define NEROSHOP_USERDATA_FILENAME      "user.sqlite3"

#define NEROSHOP_KEYS_FOLDER_NAME    "keys"
#define NEROSHOP_DATA_FOLDER_NAME    "datastore"
#define NEROSHOP_CATALOG_FOLDER_NAME "listings"
#define NEROSHOP_AVATAR_FOLDER_NAME  "avatars"

#define NEROSHOP_MAX_IMAGE_SIZE      2097152 // 2 MB
#define NEROSHOP_MIN_USERNAME_LENGTH 2
#define NEROSHOP_MAX_USERNAME_LENGTH 30

#if defined(NEROSHOP_USE_QT)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH    QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()//(QStandardPaths::AppLocalDataLocation).toStdString()
#define NEROSHOP_DEFAULT_DATABASE_PATH         NEROSHOP_DEFAULT_CONFIGURATION_PATH + "/" + NEROSHOP_DATA_FOLDER_NAME
#define NEROSHOP_DEFAULT_KEYS_PATH             NEROSHOP_DEFAULT_CONFIGURATION_PATH + "/" + NEROSHOP_KEYS_FOLDER_NAME
#if defined(_WIN32)
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/neroshop/wallet").toStdString()
#else
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop/wallet").toStdString()
#endif
#endif // endif NEROSHOP_USE_QT

#if !defined(NEROSHOP_USE_QT)
#if defined(_WIN32)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "C:/Users/" + neroshop::device::get_user() + "/AppData/Local/neroshop"//"C:/ProgramData/neroshop"
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH "C:/Users/" + neroshop::device::get_user() + "/Documents" + "/neroshop/wallet"
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "/home/" + neroshop::device::get_user() + "/.config/neroshop"//"/etc/xdg/neroshop"//"/home/" + neroshop::device::get_user() + "/.local/share/neroshop"
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH "/home/" + neroshop::device::get_user() + "/neroshop/wallet"
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "~/Library/Preferences/neroshop"
#endif

#if defined(__ANDROID__)
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "<APPROOT>/files/settings"
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH "<APPROOT>/Library/Preferences/neroshop"
#endif
#endif

#define NEROSHOP_DEFAULT_DATABASE_PATH                 NEROSHOP_DEFAULT_CONFIGURATION_PATH + "/" + NEROSHOP_DATA_FOLDER_NAME
#define NEROSHOP_DEFAULT_KEYS_PATH                     NEROSHOP_DEFAULT_CONFIGURATION_PATH + "/" + NEROSHOP_KEYS_FOLDER_NAME
#endif // endif NOT NEROSHOP_USE_QT

namespace neroshop {

static const std::initializer_list<std::string> BOOTSTRAP_I2P_NODES = {
    //{"wq344fz2wgevifcrkf5uiplywlp35ufoomhebxk5qh7obv7ephnqb.32.i2p"},
    {"pbdcncqgmgnan67ej72o7obbuvpy3wd3vhyyh4xty2luxbgmmyaq.b32.i2p"}
};

}

#endif
