// Copyright (c) 2021-2023 the neroshop core team
// Distributed under the GPLv3 software license, see the accompanying
// file LICENSE or https://opensource.org/license/gpl-3-0/.

#pragma once

#ifndef CONFIG_HPP_NEROSHOP
#define CONFIG_HPP_NEROSHOP

#include <cstdint>
#include <initializer_list>
#include <string>

#if defined(NEROSHOP_USE_QT)
#include <QStandardPaths>
#else
#include "../src/core/tools/device.hpp"
#endif

#define NEROSHOP_IPC_DEFAULT_PORT 57740
//TODO This port will be used by the daemon to establish connections with p2p network
#define NEROSHOP_P2P_DEFAULT_PORT 50881 // Use ports between 49152-65535 that are not currently registered with IANA and are rarely used
//TODO This port will allow outside clients to interact with neroshop daemon RPC server
#define NEROSHOP_RPC_DEFAULT_PORT 50882

#define NEROSHOP_DAEMON_WAIT_TIME 20 // Measured in seconds

#define NEROSHOP_LOOPBACK_ADDRESS            "127.0.0.1"
#define NEROSHOP_ANY_ADDRESS                 "0.0.0.0"

#define NEROSHOP_RECV_BUFFER_SIZE            4096

#define NEROSHOP_DHT_REPLICATION_FACTOR      10 // 10 to 20 (or even higher) // Usually 3 or 5 but a higher number would improve fault tolerant, mitigating the risk of data loss even if multiple nodes go offline simultaneously. It also helps distribute the load across more nodes, potentially improving read performance by allowing concurrent access from multiple replicas.
#define NEROSHOP_DHT_MAX_CLOSEST_NODES       10 // 50 to 100 (or even higher) // Default is 8 in most Kademlia DHTs
#define NEROSHOP_DHT_QUERY_RECV_TIMEOUT      5 // A reasonable timeout value for a DHT node could be between 5 to 30 seconds.
#define NEROSHOP_DHT_PING_MESSAGE_TIMEOUT    2
#define NEROSHOP_DHT_ROUTING_TABLE_BUCKETS   256 // recommended to use a number of buckets that is equal to the number of bits in the node id (in this case, sha-3-256 so 256 bits)
#define NEROSHOP_DHT_MAX_BUCKET_SIZE         25 // Each bucket should hold up to 12-25 or 25-50 nodes
#define NEROSHOP_DHT_MAX_NODES_PER_BUCKET    NEROSHOP_DHT_MAX_BUCKET_SIZE
#define NEROSHOP_DHT_MAX_ROUTING_TABLE_NODES NEROSHOP_DHT_ROUTING_TABLE_BUCKETS * NEROSHOP_DHT_MAX_BUCKET_SIZE
#define NEROSHOP_DHT_MAX_HEALTH_CHECKS       3 // Maximum number of consecutive failed checks before marking the node as dead
#define NEROSHOP_DHT_PERIODIC_CHECK_INTERVAL 60 // Number of seconds between each periodic health check
#define NEROSHOP_DHT_REPUBLISH_INTERVAL      2 // Number of hours between each periodic refresh/republishing
#define NEROSHOP_DHT_MAX_SEARCHES            3

#define NEROSHOP_PUBLIC_KEY_FILENAME              "<user_id>.pub"
#define NEROSHOP_PRIVATE_KEY_FILENAME             "<user_id>.key"
#define NEROSHOP_OPENSSL_PUBLIC_KEY_FILENAME      "<user_id>.pem"
#define NEROSHOP_OPENSSL_PRIVATE_KEY_FILENAME     "<user_id>.pem"
#define NEROSHOP_PGP_PRIVATE_KEY_FILENAME         "<user_id>.pgp"
#define NEROSHOP_PGP_PUBLIC_KEY_FILENAME          "<user_id>.pgp"
#define NEROSHOP_PGP_ARMORED_PRIVATE_KEY_FILENAME "<user_id>.asc"
#define NEROSHOP_PGP_ARMORED_PUBLIC_KEY_FILENAME  "<user_id>.asc"

#define NEROSHOP_RSA_DEFAULT_BIT_LENGTH 4096
#define NEROSHOP_RSA_DEFAULT_BITS NEROSHOP_RSA_DEFAULT_BIT_LENGTH

#define NEROSHOP_APPLICATION_NAME       "neroshop"
#define NEROSHOP_DAEMON_CONFIG_FILENAME "daemon.conf" // This is actually a lua file
#define NEROSHOP_DATABASE_FILENAME      "data.sqlite3"
#define NEROSHOP_SETTINGS_FILENAME      "settings.json"
#define NEROSHOP_NODES_FILENAME         "nodes.lua"
#if defined(_WIN32)
#define NEROSHOP_LOG_FILENAME           "log.txt"
#else
#define NEROSHOP_LOG_FILENAME           "log"
#endif

#if defined(NEROSHOP_USE_QT)
#define NEROSHOP_DATA_DIRECTORY_PATH           QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation).toStdString()//(QStandardPaths::AppLocalDataLocation).toStdString()
#define NEROSHOP_DEFAULT_CONFIGURATION_PATH    NEROSHOP_DATA_DIRECTORY_PATH
#define NEROSHOP_DEFAULT_DATABASE_PATH         NEROSHOP_DATA_DIRECTORY_PATH
#if defined(_WIN32)
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/neroshop").toStdString()
#else
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/neroshop").toStdString()
#endif
#endif // endif NEROSHOP_USE_QT

#if !defined(NEROSHOP_USE_QT)
#if defined(_WIN32)
#define NEROSHOP_DATA_DIRECTORY_PATH "C:/Users/" + neroshop::device::get_user() + "/AppData/Local/neroshop"//"C:/ProgramData/neroshop"
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH "C:/Users/" + neroshop::device::get_user() + "/Documents" + "/neroshop"
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#define NEROSHOP_DATA_DIRECTORY_PATH "/home/" + neroshop::device::get_user() + "/.config/neroshop"//"/etc/xdg/neroshop"//"/home/" + neroshop::device::get_user() + "/.local/share/neroshop"
#define NEROSHOP_DEFAULT_WALLET_DIRECTORY_PATH "/home/" + neroshop::device::get_user() + "/neroshop"
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define NEROSHOP_DATA_DIRECTORY_PATH "~/Library/Preferences/neroshop"
#endif

#if defined(__ANDROID__)
#define NEROSHOP_DATA_DIRECTORY_PATH "<APPROOT>/files/settings"
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define NEROSHOP_DATA_DIRECTORY_PATH "<APPROOT>/Library/Preferences/neroshop"
#endif
#endif

#define NEROSHOP_DEFAULT_CONFIGURATION_PATH            NEROSHOP_DATA_DIRECTORY_PATH
#define NEROSHOP_DEFAULT_DATABASE_PATH                 NEROSHOP_DATA_DIRECTORY_PATH
#endif // endif NOT NEROSHOP_USE_QT

namespace neroshop {
//TODO Add here your network seed or bootstrap nodes
const std::initializer_list<std::string> BOOTSTRAP_NODES = {
    "node.neroshop.org:" + std::to_string(NEROSHOP_P2P_DEFAULT_PORT),
  //"your_seed_ip1.com:8080",
  //"your_seed_ip2.com:8080",
};

}

#endif
