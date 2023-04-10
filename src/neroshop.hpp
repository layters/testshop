#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

#define NEROSHOP_APPLICATION_NAME  "neroshop"
#define NEROSHOP_AUTHOR    "larteyoh"
#define NEROSHOP_LICENSE   "GNU General Public License v3.0"
#define NEROSHOP_COPYRIGHT "Copyright (C) 2021-present larteyoh@protonmail.com"
// These include files constitute the main neroshop API
// neroshop (core)
#include "core/version.hpp"
#include "core/wallet.hpp" // causes error depending on where you place this header
#include "core/config.hpp"
#include "core/database.hpp"
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
#include "core/util.hpp"
#include "core/process.hpp"
// crypto
#include "core/crypto/rsa.hpp"
#include "core/crypto/sha256.hpp"
// database
#include "core/database/sqlite.hpp"
// price
// ...
// protocol
#include "core/protocol/p2p/kademlia.hpp"
#include "core/protocol/p2p/node.hpp"
#include "core/protocol/rpc/json_rpc.hpp"
// network
// ...
// util
#include "core/util/downloader.hpp"
#include "core/util/extractor.hpp"
#include "core/util/logger.hpp"
#include "core/util/regex.hpp"
#include "core/util/updater.hpp"
// neroshop-daemon
// ...
// neroshop (gui)
#if defined(NEROSHOP_BUILD_GUI)
#include "gui/backend.hpp"
#include "gui/currency_exchange_rates_provider.hpp"
#include "gui/script_controller.hpp"
#include "gui/user_controller.hpp"
#include "gui/image_provider.hpp"
#include "gui/wallet_controller.hpp"
#include "gui/wallet_qr_provider.hpp"
#include "gui/table_model.hpp"
#endif

#endif
