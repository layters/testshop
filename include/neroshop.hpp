#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

#define APP_NAME      "neroshop"
#define APP_AUTHOR    "larteyoh"
#define APP_LICENSE   "MIT License" 
#define APP_COPYRIGHT "Copyright (C) 2021-present larteyoh@protonmail.com"
#define APP_VERSION_MAJOR "0"
#define APP_VERSION_MINOR "1"
#define APP_VERSION_PATCH "0"
#define APP_VERSION APP_VERSION_MAJOR "." APP_VERSION_MINOR "." APP_VERSION_PATCH

#include "debug.hpp"
#include "wallet.hpp" // causes error depending on where you place this header
#include "config.hpp"
#include "database.hpp"
#include "qr.hpp"
#include "icon.hpp"
#include "validator.hpp"
#include "converter.hpp"
#include "user.hpp"
#include "buyer.hpp"
#include "seller.hpp"
#include "item.hpp"
#include "cart.hpp"
#include "order.hpp"
#include "message.hpp"
#include "catalog.hpp"
#include "server.hpp"
#include "client.hpp"
#include "encryptor.hpp"
//#include "carrier.hpp" // not currently in use
#include "util.hpp"
#include "process.hpp"

#endif
