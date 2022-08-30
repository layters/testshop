#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

#define NEROSHOP_APPLICATION_NAME  "neroshop"
#define NEROSHOP_AUTHOR    "larteyoh"
#define NEROSHOP_LICENSE   "MIT License" 
#define NEROSHOP_COPYRIGHT "Copyright (C) 2021-present larteyoh@protonmail.com"
#define NEROSHOP_VERSION_MAJOR "0"
#define NEROSHOP_VERSION_MINOR "1"
#define NEROSHOP_VERSION_PATCH "0"
#define NEROSHOP_VERSION NEROSHOP_VERSION_MAJOR "." NEROSHOP_VERSION_MINOR "." NEROSHOP_VERSION_PATCH
// neroshop (core)
#include "debug.hpp"
#include "wallet.hpp" // causes error depending on where you place this header
#include "config.hpp"
#include "database.hpp"
#include "qr.hpp"
#include "validator.hpp"
#include "converter.hpp"
#include "user.hpp"
#include "buyer.hpp"
#include "seller.hpp"
#include "item.hpp"
#include "cart.hpp"
#include "order.hpp"
#include "catalog.hpp"
#include "server.hpp"
#include "client.hpp"
#include "encryptor.hpp"
//#include "carrier.hpp" // not currently in use
#include "util.hpp"
#include "process.hpp"
// neroshop (gui)
#if defined(NEROSHOP_BUILD_GUI)
//#include "main_window.hpp" // already included in gui/main.cpp so there's no need to include this (I think?)
#include "gui/icon.hpp"
#include "gui/main_window.hpp"
#include "gui/message_box.hpp"
#include "gui/wallet_proxy.hpp"
#endif

#endif
