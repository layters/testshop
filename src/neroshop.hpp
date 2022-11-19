#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

#define NEROSHOP_APPLICATION_NAME  "neroshop"
#define NEROSHOP_AUTHOR    "larteyoh"
#define NEROSHOP_LICENSE   "3-Clause BSD License"
#define NEROSHOP_COPYRIGHT "Copyright (C) 2021-present larteyoh@protonmail.com"
#define NEROSHOP_VERSION_MAJOR "0"
#define NEROSHOP_VERSION_MINOR "1"
#define NEROSHOP_VERSION_PATCH "0"
#define NEROSHOP_VERSION NEROSHOP_VERSION_MAJOR "." NEROSHOP_VERSION_MINOR "." NEROSHOP_VERSION_PATCH
// These include files constitute the main neroshop API
// neroshop (core)
#include "core/debug.hpp"
#include "core/wallet.hpp" // causes error depending on where you place this header
#include "core/config.hpp"
#include "core/database.hpp"
#include "core/qr.hpp"
#include "core/validator.hpp"
#include "core/currency_converter.hpp"
#include "core/user.hpp"
#include "core/buyer.hpp"
#include "core/seller.hpp"
#include "core/item.hpp"
#include "core/cart.hpp"
#include "core/order.hpp"
#include "core/catalog.hpp"
#include "core/server.hpp"
#include "core/client.hpp"
#include "core/encryptor.hpp"
#include "core/util.hpp"
#include "core/process.hpp"
// neroshop-daemon
// ...
// neroshop (gui)
#if defined(NEROSHOP_BUILD_GUI)
#include "gui/backend.hpp"
#include "gui/script_proxy.hpp"
#include "gui/wallet_proxy.hpp"
#endif

#endif
