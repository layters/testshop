#include "protobuf.hpp"

#if defined(NEROSHOP_USE_PROTOBUF)
#include "../../version.hpp"
#include "../p2p/dht_rescode.hpp"
#include "../p2p/node.hpp"
#include "../p2p/routing_table.hpp" // neroshop_config.hpp included here
#include "../../database/database.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#endif

#if defined(__gnu_linux__)
#include <sys/socket.h> // ::send, ::recv
#endif

namespace neroshop {

namespace rpc {

//-----------------------------------------------------------------------------

std::vector<uint8_t> protobuf_process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode) {
    neroshop::DhtMessage request_msg;
    neroshop::DhtMessage response_msg;
    std::vector<uint8_t> response;
    
    // Parse request protobuf message
    if (!request_msg.ParseFromArray(request.data(), static_cast<int>(request.size()))) {
        // Parsing error response
        auto* error = response_msg.mutable_error();
        error->set_code(static_cast<int>(DhtResultCode::ParseError));
        error->set_message("Parse error");
        // Optionally put error data (no exception info here)
        response_msg.mutable_response()->set_version(std::string(NEROSHOP_DHT_VERSION));
        // Serialize (error) response
        int size = response_msg.ByteSizeLong();
        response.resize(size);
        response_msg.SerializeToArray(response.data(), size);
        return response;
    }
    
    // Initialize response's version
    std::string version = std::string(NEROSHOP_DHT_VERSION);

    // Validate query exists and fields
    if (!request_msg.has_query()) {
        // Invalid request - no query message
        auto* error = response_msg.mutable_error();
        error->set_code(static_cast<int>(DhtResultCode::ParseError));
        error->set_message("No query message");
        response_msg.mutable_response()->set_version(version);
        int size = response_msg.ByteSizeLong();
        response.resize(size);
        response_msg.SerializeToArray(response.data(), size);
        return response;
    }

    const auto& query = request_msg.query(); // we're only reading here so no mutable_query()
    if (query.version() != version) {
        auto* error = response_msg.mutable_error();
        error->set_code(static_cast<int>(DhtResultCode::ParseError));
        error->set_message("Version mismatch");
        response_msg.mutable_response()->set_version(version);
        int size = response_msg.ByteSizeLong();
        response.resize(size);
        response_msg.SerializeToArray(response.data(), size);
        return response;
    }

    std::string method = query.query();
    const auto& args_map = query.args();
    std::string requester_node_id;
    if (!ipc_mode) {
        auto it = args_map.find("id");
        if (it == args_map.end()) {
            // No id supplied in args
            auto* error = response_msg.mutable_error();
            error->set_code(static_cast<int>(DhtResultCode::ParseError));
            error->set_message("Missing id in args");
            response_msg.mutable_response()->set_version(version);
            int size = response_msg.ByteSizeLong();
            response.resize(size);
            response_msg.SerializeToArray(response.data(), size);
            return response;
        }
        requester_node_id = it->second;
    }
    else {
        requester_node_id = node.get_id();
    }

    // Transaction ID from query (optional, when in IPC mode)
    std::string tid = query.tid();

    int code = static_cast<int>(DhtResultCode::Success);

    // Display request message for debug purposes
    #ifdef NEROSHOP_DEBUG
    std::string debug_str = request_msg.query().DebugString();
    std::cout << "\033[33m" << debug_str << "\033[0m\n";
    #endif
    //---------------------------------------------------------------
    // [START response]
    // Prepare response message payload
    auto* resp = response_msg.mutable_response(); // DhtMessage.msg (Response)
    // Now resp is guaranteed to exist. Use resp to set fields:
    resp->set_tid(tid);
    resp->set_version(version);
    // Set response's ResponsePayload fields
    auto* data_map = resp->mutable_response()->mutable_data(); // Response.response (ResponsePayload) -> ResponsePayload.data (map<string, string>)
    (*data_map)["id"] = node.get_id();
    
    //---------------------------------------------------------------
    // Handle methods
    if (method == "ping") {
        // Reply "version" and "id", and "tid" set above...
    }
    else if (method == "find_node") {
        auto it = args_map.find("target");
        if (it == args_map.end()) {
            code = static_cast<int>(DhtResultCode::ParseError);
        } else {
            std::string target = it->second;
            auto nodes = node.find_node(target, NEROSHOP_DHT_MAX_CLOSEST_NODES);
            
            // Get mutable ResponsePayload pointer
            auto* response_payload = resp->mutable_response(); // Response.response (ResponsePayload)
            
            // Populate repeated NodeInfo protobuf field
            for (const auto& n : nodes) {
                neroshop::NodeInfo* node_info = response_payload->add_nodes(); // Add new NodeInfo, get mutable pointer to it
                node_info->set_address(n->get_address()); // Set address field on the actual new NodeInfo inside the repeated field
                node_info->set_port(n->get_port()); // Set port field on the actual new NodeInfo inside the repeated field
            }
        }
    }
    else if (method == "get_providers") {
        auto it = args_map.find("key");
        if (it == args_map.end()) {
            code = static_cast<int>(DhtResultCode::ParseError);
        } else {
            std::string key = it->second;
            auto peers = node.get_providers(key);
            if (node.has_key(key) || node.has_key_cached(key)) {
                peers.push_front(Peer{node.get_address(), node.get_port()});
            }
            auto* response_payload = resp->mutable_response(); // Response.response (ResponsePayload)
            for (const auto& p : peers) {
                neroshop::NodeInfo* node_info = response_payload->add_nodes();
                node_info->set_address(p.address);
                node_info->set_port(p.port);
            }
        }
    }
    else if (method == "get") {
        auto it = args_map.find("key");
        if (it == args_map.end()) {
            code = static_cast<int>(DhtResultCode::ParseError);
        } else {
            std::string key = it->second;
            if (!ipc_mode) {
                std::string value;
                if (!node.has_key(key)) {
                    if (node.has_key_cached(key)) {
                        value = node.get_cached(key);
                    }
                } else {
                    value = node.get(key);
                }
                if (value.empty()) {
                    code = static_cast<int>(DhtResultCode::RetrieveFailed);
                    auto* error = response_msg.mutable_error();
                    error->set_code(code);
                    error->set_message("Key not found");
                    error->set_data("");
                    response_msg.mutable_response()->set_version(version);
                    response_msg.mutable_error()->set_code(code);
                } else {
                    (*data_map)["value"] = value;
                }
            } else {
                if (key == "status") {
                    // Reply "id" already set above...
                    (*data_map)["connected_peers"] = std::to_string(node.get_peer_count());
                    (*data_map)["active_peers"] = std::to_string(node.get_active_peer_count());
                    (*data_map)["idle_peers"] = std::to_string(node.get_idle_peer_count());
                    (*data_map)["data_count"] = std::to_string(node.get_data_count());
                    (*data_map)["data_ram_usage"] = std::to_string(node.get_data_ram_usage());
                    (*data_map)["host"] = node.get_address();
                    (*data_map)["port"] = std::to_string(node.get_port());
                    (*data_map)["network_type"] = node.get_network_type_as_string();

                    // Populate repeated NodeInfo protobuf field for peers
                    auto* response_payload = resp->mutable_response(); // Response.response (ResponsePayload)
                    auto peers_list = node.get_peers();
                    for (const auto& peer : peers_list) {
                        neroshop::NodeInfo* node_info = response_payload->add_nodes();
                        node_info->set_address(peer.address);
                        node_info->set_port(peer.port);
                        node_info->set_id(peer.id);
                        node_info->set_status(static_cast<int>(peer.status));
                        node_info->set_distance(peer.distance);
                    }
                    // The peer list is now in response_payload->nodes()
                } else {
                    std::string value = node.send_get(key);
                    if (value.empty()) {
                        code = static_cast<int>(DhtResultCode::RetrieveFailed);
                        auto* error = response_msg.mutable_error();
                        error->set_code(code);
                        error->set_message("Key not found");
                        error->set_data("");
                        response_msg.mutable_response()->set_version(version);
                    } else {
                        (*data_map)["value"] = value;
                    }
                }
            }
        }
    }
    else if (method == "put") {
        auto it_key = args_map.find("key");
        auto it_val = args_map.find("value");
        if (it_key == args_map.end() || it_val == args_map.end()) {
            code = static_cast<int>(DhtResultCode::ParseError);
        } else {
            std::string key = it_key->second;
            std::string value = it_val->second;
            if (!ipc_mode) {
                bool success = node.store(key, value);
                code = success ? static_cast<int>(DhtResultCode::Success) : static_cast<int>(DhtResultCode::StoreFailed);
                if (code != static_cast<int>(DhtResultCode::Success)) {
                    auto* error = response_msg.mutable_error();
                    error->set_code(code);
                    error->set_message(get_dht_result_code_as_string(static_cast<DhtResultCode>(code)));
                    response_msg.mutable_response()->set_version(version);
                } else {
                    // ...
                }
            } else {
                // ipc_mode true put/send_put scenario
                int put_messages_sent = node.send_put(key, value);
                if (put_messages_sent <= 0) {
                    code = static_cast<int>(DhtResultCode::StoreFailed);
                } else {
                    code = static_cast<int>(DhtResultCode::Success);
                }

                if ((put_messages_sent < NEROSHOP_DHT_REPLICATION_FACTOR) && (put_messages_sent > 0)) {
                    code = static_cast<int>(DhtResultCode::StorePartial);
                }

                if (node.store(key, value)) {
                    if (node.cache(key, value)) {
                        if (put_messages_sent == 0) {
                            code = static_cast<int>(DhtResultCode::StoreToSelf);
                        }
                        if (!Node::is_value_publishable(value)) {
                            code = static_cast<int>(DhtResultCode::Success);
                        }
                        node.map(key, value);
                    }
                }

                if (code == static_cast<int>(DhtResultCode::Success) || code == static_cast<int>(DhtResultCode::StorePartial)) {
                    // ...
                } else {
                    auto* error = response_msg.mutable_error();
                    error->set_code(code);
                    error->set_message(get_dht_result_code_as_string(static_cast<DhtResultCode>(code)));
                    response_msg.mutable_response()->set_version(version);
                }
            }
        }
    }
    else if (method == "map") {
        auto it_key = args_map.find("key");
        auto it_val = args_map.find("value");
        if (it_key != args_map.end() && it_val != args_map.end()) {
            std::string key = it_key->second;
            std::string value = it_val->second;
            if (node.validate(key, value)) {
                node.map(key, value);
            }
        } else {
            code = static_cast<int>(DhtResultCode::ParseError);
        }
    }
    else if (method == "remove" && ipc_mode) {
        auto it_key = args_map.find("key");
        if (it_key == args_map.end()) {
            code = static_cast<int>(DhtResultCode::ParseError);
        } else {
            std::string key = it_key->second;
            if (node.has_key(key)) {
                code = (node.remove(key)) ? static_cast<int>(DhtResultCode::Success) : static_cast<int>(DhtResultCode::RemoveFailed);
            }

            db::Sqlite3* database = neroshop::get_database();
            if (!database) throw std::runtime_error("database is not opened");
            if (database->get_integer_params("SELECT EXISTS(SELECT key FROM hash_table WHERE key = ?1)", {key}) == 1) {
                code = (database->execute_params("DELETE FROM hash_table WHERE key = ?1", {key}) == SQLITE_OK)
                       ? static_cast<int>(DhtResultCode::Success)
                       : static_cast<int>(DhtResultCode::RemoveFailed);
            }
            if (database->get_integer_params("SELECT EXISTS(SELECT key FROM mappings WHERE key = ?1)", {key}) == 1) {
                code = (database->execute_params("DELETE FROM mappings WHERE key = ?1", {key}) == SQLITE_OK)
                       ? static_cast<int>(DhtResultCode::Success)
                       : static_cast<int>(DhtResultCode::RemoveFailed);
            }

            if (code != static_cast<int>(DhtResultCode::Success)) {
                auto* error = response_msg.mutable_error();
                error->set_code(code);
                error->set_message(get_dht_result_code_as_string(static_cast<DhtResultCode>(code)));
                response_msg.mutable_response()->set_version(version);
            } else {
                // ...
            }
        }
    }
    else if (method == "clear" && ipc_mode) {
        code = (node.remove_all()) ? static_cast<int>(DhtResultCode::Success) : static_cast<int>(DhtResultCode::RemoveFailed);
        if (code != static_cast<int>(DhtResultCode::Success)) {
            auto* error = response_msg.mutable_error();
            error->set_code(code);
            error->set_message(get_dht_result_code_as_string(static_cast<DhtResultCode>(code)));
            response_msg.mutable_response()->set_version(version);
        } else {
            // ...
        }
    }
    else {
        // Unknown method error
        code = static_cast<int>(DhtResultCode::ParseError);
        auto* error = response_msg.mutable_error();
        error->set_code(code);
        error->set_message("Unknown method");
        response_msg.mutable_response()->set_version(version);
    }

    if (code != static_cast<int>(DhtResultCode::Success)) {
        auto* error = response_msg.mutable_error();
        error->set_code(code);
        if (error->message().empty()) error->set_message(get_dht_result_code_as_string(static_cast<DhtResultCode>(code)));
    }

    // Set version in response or error if not set
    if (!response_msg.has_response() && !response_msg.has_error()) {
        response_msg.mutable_response()->set_version(version);
    }

    // After building response_msg...
    
    int size = response_msg.ByteSizeLong();
    response.resize(size);
    response_msg.SerializeToArray(response.data(), size);
    
    //---------------------------------------------------------------
    
    return response;
    // [END response]
}

//-----------------------------------------------------------------------------

} // namespace rpc

} // namespace neroshop

#endif // NEROSHOP_USE_PROTOBUF
