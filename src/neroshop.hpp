// Copyright (c) 2021-2023 the neroshop core team
// Distributed under the GPLv3 software license, see the accompanying
// file LICENSE or https://opensource.org/license/gpl-3-0/.

#ifndef NEROSHOP_HPP
#define NEROSHOP_HPP

// These include files constitute the main neroshop API
// neroshop (core)
#include "core/settings.hpp"
#include "core/version.hpp"
// crypto
#include "core/crypto/rsa.hpp"
#include "core/crypto/sha3.hpp"
#include "core/crypto/sha256.hpp"
// database
#include "core/database/database.hpp"
#include "core/database/sqlite3/sqlite3.hpp"
// market
#include "core/market/cart.hpp"
#include "core/market/category.hpp"
#include "core/market/escrow.hpp"
#include "core/market/image.hpp"
#include "core/market/listing.hpp"
#include "core/market/order.hpp"
#include "core/market/product.hpp"
#include "core/market/rating.hpp"
#include "core/market/seller.hpp"
#include "core/market/user.hpp"
// network
#include "core/network/i2p.hpp"
// price_api
#include "core/price_api/currency_converter.hpp"
// protocol
#include "core/protocol/p2p/dht_rescode.hpp"
#include "core/protocol/p2p/file_piece_hasher.hpp"
#include "core/protocol/p2p/key_mapper.hpp"
#include "core/protocol/p2p/node.hpp"
#include "core/protocol/p2p/routing_table.hpp"
#include "core/protocol/p2p/serializer.hpp"
#include "core/protocol/rpc/json_rpc.hpp"
#include "core/protocol/rpc/msgpack.hpp"
#include "core/protocol/transport/client.hpp"
#include "core/protocol/transport/sam_client.hpp"
#include "core/protocol/transport/server.hpp"
// tools (utilities)
#include "core/tools/base64.hpp"
#include "core/tools/device.hpp"
#include "core/tools/filesystem.hpp"
#include "core/tools/logger.hpp"
#include "core/tools/process.hpp"
#include "core/tools/script.hpp"
#include "core/tools/string.hpp"
#include "core/tools/thread_pool.hpp"
#include "core/tools/timestamp.hpp"
#include "core/tools/uuid.hpp"
// wallet
#include "core/wallet/wallet.hpp"
//#include "core/wallet/monero/monero.hpp"
// neroshop-daemon
// ...
// neroshop (gui)
#if defined(NEROSHOP_BUILD_GUI)
#include "gui/backend.hpp"
#include "gui/currency_rate_provider.hpp"
#include "gui/daemon_manager.hpp"
#include "gui/enum_wrapper.hpp"
#include "gui/image_loader.hpp"
#include "gui/image_provider.hpp"
#include "gui/proxy_manager.hpp"
#include "gui/settings_manager.hpp"
#include "gui/user_manager.hpp"
#include "gui/wallet_manager.hpp"
#include "gui/wallet_qr_provider.hpp"

#endif

#endif
