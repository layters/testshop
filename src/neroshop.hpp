#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

#define NEROSHOP_APPLICATION_NAME  "neroshop"
#define NEROSHOP_AUTHOR    "larteyoh"
#define NEROSHOP_LICENSE   "GNU General Public License v3.0"
#define NEROSHOP_COPYRIGHT "Copyright (C) 2021-present larteyoh@protonmail.com"
// These include files constitute the main neroshop API
// neroshop (core)
#include "core/cart.hpp"
#include "core/category.hpp"
#include "core/config.hpp"
#include "core/enums.hpp"
#include "core/listing.hpp"
#include "core/order.hpp"
#include "core/product.hpp"
#include "core/rating.hpp"
#include "core/script.hpp"
#include "core/seller.hpp"
#include "core/serializer.hpp"
#include "core/user.hpp"
#include "core/version.hpp"
#include "core/wallet.hpp"
// crypto
#include "core/crypto/pgp.hpp"
#include "core/crypto/rsa.hpp"
#include "core/crypto/sha3.hpp"
#include "core/crypto/sha256.hpp"
// database
#include "core/database/database.hpp"
#include "core/database/sqlite.hpp"
// network
#include "core/network/i2p.hpp"
#include "core/network/tor.hpp"
// price
#include "core/price/currency_converter.hpp"
// protocol
#include "core/protocol/messages/msgpack.hpp"
//#include "core/protocol/p2p/gossip.hpp"
#include "core/protocol/p2p/kademlia.hpp"
#include "core/protocol/p2p/node.hpp"
#include "core/protocol/p2p/routing_table.hpp"
#include "core/protocol/rpc/json_rpc.hpp"
#include "core/protocol/transport/client.hpp"
#include "core/protocol/transport/ip_address.hpp"
#include "core/protocol/transport/server.hpp"
// tools (utilities)
#include "core/tools/downloader.hpp"
#include "core/tools/extractor.hpp"
#include "core/tools/logger.hpp"
#include "core/tools/process.hpp"
#include "core/tools/regex.hpp"
#include "core/tools/tools.hpp"
#include "core/tools/updater.hpp"
// neroshop-daemon
// ...
// neroshop (gui)
#if defined(NEROSHOP_BUILD_GUI)
#include "gui/backend.hpp"
#include "gui/currency_rate_provider.hpp"
#include "gui/image_loader.hpp"
#include "gui/image_provider.hpp"
#include "gui/script_controller.hpp"
#include "gui/table_model.hpp"
#include "gui/user_controller.hpp"
#include "gui/wallet_controller.hpp"
#include "gui/wallet_qr_provider.hpp"

#endif

#endif
