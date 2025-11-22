#include "msgpack.hpp"

#if defined(NEROSHOP_USE_MSGPACK)
#include "../../version.hpp"
#include "../../tools/logger.hpp"
#include "../p2p/dht_rescode.hpp"
#include "../p2p/node.hpp"
#include "../p2p/routing_table.hpp"
#include "../../database/database.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <cinttypes> // PRIu64, PRId64
#include <cstdio> // printf
#include <random> // std::random_device
#include <sstream>
#include <iomanip>

#if defined(_WIN32)
#include <winsock2.h>
#endif

#if defined(__gnu_linux__)
#include <sys/socket.h> // ::send, ::recv
#endif

namespace neroshop {

namespace rpc {

//-----------------------------------------------------------------------------

// Recursive helper to escape JSON special characters in strings
static std::string json_escape(const char* str, uint32_t len) {
    std::ostringstream oss;
    oss << '"';
    for (uint32_t i = 0; i < len; i++) {
        char c = str[i];
        switch (c) {
            case '\\': oss << "\\\\"; break;
            case '\"': oss << "\\\""; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
                } else {
                    oss << c;
                }
        }
    }
    oss << '"';
    return oss.str();
}

static std::string array_to_json(const msgpack_object_array* arr) {
    std::ostringstream oss;
    oss << "[";
    for (uint32_t i = 0; i < arr->size; i++) {
        oss << msgpack_object_to_json(&arr->ptr[i]);
        if (i + 1 < arr->size) oss << ",";
    }
    oss << "]";
    return oss.str();
}

static std::string map_to_json(const msgpack_object_map* map) {
    std::ostringstream oss;
    oss << "{";
    for (uint32_t i = 0; i < map->size; i++) {
        const msgpack_object_kv* kv = &map->ptr[i];
        if (kv->key.type == MSGPACK_OBJECT_STR) {
            oss << json_escape(kv->key.via.str.ptr, kv->key.via.str.size);
        } else {
            // for simplicity convert non-string keys as numbers or null
            oss << "\"<non-string-key>\"";
        }
        oss << ":";
        oss << msgpack_object_to_json(&kv->val);
        if (i + 1 < map->size) oss << ",";
    }
    oss << "}";
    return oss.str();
}

//-----------------------------------------------------------------------------

std::string msgpack_object_to_json(const msgpack_object* obj) {
    switch (obj->type) {
        case MSGPACK_OBJECT_NIL:
            return "null";
        case MSGPACK_OBJECT_BOOLEAN:
            return obj->via.boolean ? "true" : "false";
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            return std::to_string(obj->via.u64);
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            return std::to_string(obj->via.i64);
        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64: {
            std::ostringstream oss;
            oss << std::fixed << obj->via.f64;
            return oss.str();
        }
        case MSGPACK_OBJECT_STR:
            return json_escape(obj->via.str.ptr, obj->via.str.size);
        case MSGPACK_OBJECT_BIN: {
            // Represent binary data as hex string
            std::ostringstream oss;
            oss << "\"0x";
            for (uint32_t i = 0; i < obj->via.bin.size; i++) {
                oss << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)obj->via.bin.ptr[i];
            }
            oss << "\"";
            return oss.str();
        }
        case MSGPACK_OBJECT_ARRAY:
            return array_to_json(&obj->via.array);
        case MSGPACK_OBJECT_MAP:
            return map_to_json(&obj->via.map);
        case MSGPACK_OBJECT_EXT:
            // Extension types can be represented with type and data hex
        {
            std::ostringstream oss;
            oss << "\"<ext type=" << (int)obj->via.ext.type << " data=0x";
            for (uint32_t i = 0; i < obj->via.ext.size; i++) {
                oss << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)obj->via.ext.ptr[i];
            }
            oss << ">\"";
            return oss.str();
        }
    }
    return "\"<unknown>\""; // fallback
}

//-----------------------------------------------------------------------------

// Helper: find a key in a msgpack_object map, returns pointer to value or NULL if not found
const msgpack_object* msgpack_find(const msgpack_object* map_obj, const char* key) {
    if (map_obj->type != MSGPACK_OBJECT_MAP) return NULL;
    for (uint32_t i = 0; i < map_obj->via.map.size; ++i) {
        const msgpack_object_kv* kv = &map_obj->via.map.ptr[i];
        if (kv->key.type == MSGPACK_OBJECT_STR) {
            if (strncmp(kv->key.via.str.ptr, key, kv->key.via.str.size) == 0 &&
                strlen(key) == kv->key.via.str.size) {
                return &kv->val;
            }
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------

std::vector<uint8_t> msgpack_process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode) {
    /*std::cout << "MSGPACK_VERSION: " << MSGPACK_VERSION << std::endl;

    std::vector<uint8_t> response; // bytes
    msgpack::object_handle oh;////nlohmann::json request_object;
    std::map<std::string, msgpack::object> response_map;////nlohmann::json response_object;
    
    try {
        oh = msgpack::unpack(reinterpret_cast<const char*>(request.data()), request.size());
        msgpack::object request_object = oh.get();
        std::cout << "\033[33m" << request_object << "\033[0m" << std::endl;
    } catch (const msgpack::unpack_error& exception) {
        // Build inner error map
        neroshop::log_error("Error parsing client request");
        std::map<std::string, msgpack::object> error_map;
        {
            msgpack::zone zone; // zone to manage all allocations

            error_map["code"] = msgpack::object(static_cast<int>(DhtResultCode::ParseError), zone);
            error_map["message"] = msgpack::object(std::string("Parse error"), zone);
            error_map["data"] = msgpack::object(std::string(exception.what()), zone);
            // Insert error map into response map, allocating memory with same zone
            response_map["error"] = msgpack::object(error_map, zone);
            // Add the other fields
            response_map["version"] = msgpack::object(std::string(NEROSHOP_DHT_VERSION), zone);
            ////response_map["tid"] = msgpack::object(nullptr, zone); // <- omit "tid"
            
            // Now pack
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, response_map);
            #ifdef NEROSHOP_DEBUG
            msgpack_print(sbuf.data(), sbuf.size());
            msgpack_print(response_map);
            #endif
            
            std::vector<uint8_t> response(sbuf.data(), sbuf.data() + sbuf.size());
            return response;
        }
    }
    
    // TODO: Get data fields from unpacked request (oh)
    msgpack::object request_object = oh.get();*/
    
    //-----------------------------------------------------------
    // msgpack-c packing code replaces msgpack-cxx portion here
    //-----------------------------------------------------------
    msgpack_unpacked result;
    size_t off = 0;
    msgpack_unpacked_init(&result);

    if (msgpack_unpack_next(&result, reinterpret_cast<const char*>(request.data()), request.size(), &off) != MSGPACK_UNPACK_SUCCESS) {
        neroshop::log_error("Error parsing client request");
        msgpack_sbuffer sbuf;
        msgpack_packer pk;
        msgpack_sbuffer_init(&sbuf);
        msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

        // pack error map: { "code":1, "message":"Parse error", "data":"Unpacking failed" }
        msgpack_pack_map(&pk, 3);
        msgpack_pack_str(&pk, 4); msgpack_pack_str_body(&pk, "code", 4);
        msgpack_pack_int(&pk, 1);
        msgpack_pack_str(&pk, 7); msgpack_pack_str_body(&pk, "message", 7);
        msgpack_pack_str(&pk, 11); msgpack_pack_str_body(&pk, "Parse error", 11);
        msgpack_pack_str(&pk, 4); msgpack_pack_str_body(&pk, "data", 4);
        const char* err = "Unpacking failed";
        msgpack_pack_str(&pk, strlen(err)); msgpack_pack_str_body(&pk, err, strlen(err));

        std::vector<uint8_t> response(sbuf.data, sbuf.data + sbuf.size);
        msgpack_sbuffer_destroy(&sbuf);
        msgpack_unpacked_destroy(&result);
        return response;
    }

    msgpack_object* request_object = &result.data;
    msgpack_print_object(request_object, 0);
    
    // Process request object and build response here...
    
    //-----------------------------------------------------------
    // Old nlohmann-json version
    //-----------------------------------------------------------
    /*assert(request_object.is_object());
    assert(request_object["version"].is_string());
    std::string neroshop_version = request_object["version"];
    assert(neroshop_version == std::string(NEROSHOP_DHT_VERSION));
    assert(request_object["query"].is_string());
    std::string method = request_object["query"];
    // "args" must contain the querying node's ID 
    assert(request_object["args"].is_object());
    auto params_object = request_object["args"];
    if(!ipc_mode) assert(params_object["id"].is_string());
    std::string requester_node_id = (ipc_mode) ? node.get_id() : params_object["id"].get<std::string>();
    
    if(!request_object.contains("tid") && !ipc_mode) {
        std::cout << "No tid found, hence a notification that will not receive a response from the server\n";
        return {};
    }
    auto tid = (ipc_mode) ? nullptr : request_object["tid"];
    int code = static_cast<int>(DhtResultCode::Success);*/
    
    //-----------------------------------------------------------
    // Transition from nlohmann-json to msgpack-cxx
    //-----------------------------------------------------------
    /*// Check top-level object is a map (like JSON object)
    assert(request_object.type == msgpack::type::MAP);

    // Helper lambda to find a key in a msgpack map object
    auto find_key = [](const msgpack::object& map_obj, const std::string& key) -> msgpack::object* {
        if (map_obj.type != msgpack::type::MAP) return nullptr;
        for (uint32_t i = 0; i < map_obj.via.map.size; ++i) {
            const auto& kv = map_obj.via.map.ptr[i];
            if (kv.key.type == msgpack::type::STR) {
                std::string k(kv.key.via.str.ptr, kv.key.via.str.size);
                if (k == key) {
                    return const_cast<msgpack::object*>(&kv.val);
                }
            }
        }
        return nullptr; // key not found
    };
    
    // Validate and extract "version"
    msgpack::object* version_obj = find_key(request_object, "version");
    assert(version_obj && version_obj->type == msgpack::type::STR);
    std::string neroshop_version(version_obj->via.str.ptr, version_obj->via.str.size);
    assert(neroshop_version == std::string(NEROSHOP_DHT_VERSION));

    // Validate and extract "query"
    msgpack::object* query_obj = find_key(request_object, "query");
    assert(query_obj && query_obj->type == msgpack::type::STR);
    std::string method(query_obj->via.str.ptr, query_obj->via.str.size);

    // Validate and extract "args" as a sub-map
    msgpack::object* args_obj = find_key(request_object, "args");
    assert(args_obj && args_obj->type == msgpack::type::MAP);

    // Extract requester_node_id from args or node, depending on ipc_mode
    std::string requester_node_id;
    if (!ipc_mode) {
        msgpack::object* id_obj = find_key(*args_obj, "id");
        assert(id_obj && id_obj->type == msgpack::type::STR);
        requester_node_id = std::string(id_obj->via.str.ptr, id_obj->via.str.size);
    } else {
        requester_node_id = node.get_id();
    }

    // Check if "tid" exists if not ipc_mode
    msgpack::object* tid_obj = find_key(request_object, "tid");
    if (!ipc_mode && !tid_obj) {
        std::cout << "No tid found, hence a notification that will not receive a response from the server\n";
        return {};
    }
    
    // Prepare zone for allocations in response building
    msgpack::zone zone;

    int code = static_cast<int>(DhtResultCode::Success);*/
    
    //-----------------------------------------------------------
    // msgpack-c packing code replaces msgpack-cxx portion here
    //-----------------------------------------------------------
    // Validate top-level object is a map
    assert(request_object->type == MSGPACK_OBJECT_MAP);

    // Extract "version"
    const msgpack_object* version_obj = msgpack_find(request_object, "version");
    assert(version_obj && version_obj->type == MSGPACK_OBJECT_STR);
    const char* neroshop_version = version_obj->via.str.ptr;
    size_t neroshop_version_len = version_obj->via.str.size;
    assert(neroshop_version_len == strlen(NEROSHOP_DHT_VERSION));
    assert(strncmp(neroshop_version, NEROSHOP_DHT_VERSION, neroshop_version_len) == 0);

    // Extract "query" -> method string
    const msgpack_object* query_obj = msgpack_find(request_object, "query");
    assert(query_obj && query_obj->type == MSGPACK_OBJECT_STR);
    char method[128] = {0};
    size_t method_len = query_obj->via.str.size;
    memcpy(method, query_obj->via.str.ptr, method_len < sizeof(method) ? method_len : sizeof(method) -1);

    // Extract "args" -> must be map
    const msgpack_object* args_obj = msgpack_find(request_object, "args");
    assert(args_obj && args_obj->type == MSGPACK_OBJECT_MAP);

    // requester_node_id from args or node depending on ipc_mode
    char requester_node_id[256] = {0};
    if (!ipc_mode) {
        const msgpack_object* id_obj = msgpack_find(args_obj, "id");
        assert(id_obj && id_obj->type == MSGPACK_OBJECT_STR);
        size_t id_len = id_obj->via.str.size;
        memcpy(requester_node_id, id_obj->via.str.ptr, id_len < sizeof(requester_node_id) ? id_len : sizeof(requester_node_id) -1);
    } else {
        snprintf(requester_node_id, sizeof(requester_node_id), "%s", node.get_id().c_str());
    }

    // Check optional "tid" key if not ipc_mode
    const msgpack_object* tid_obj = msgpack_find(request_object, "tid");
    std::string tid;
    if (tid_obj && tid_obj->type == MSGPACK_OBJECT_STR) {
        tid.assign(tid_obj->via.str.ptr, tid_obj->via.str.size);
    } else if (!ipc_mode) {
        // No tid in a non-ipc request, treat as notification or error
        printf("No tid found, no response will be sent\n");
        msgpack_unpacked_destroy(&result);
        return {};
    }


    int code = static_cast<int>(DhtResultCode::Success);
    
    //-----------------------------------------------------------
    // Old nlohmann-json version
    //-----------------------------------------------------------
    /*if(method == "ping") {
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
    }*/
    
    //-----------------------------------------------------------
    // Transition from nlohmann-json to msgpack-cxx
    //-----------------------------------------------------------
    /*// Build response depending on method
    if (method == "ping") {
        // Build response map
        response_map["version"] = msgpack::object(std::string(NEROSHOP_DHT_VERSION), zone);
        std::map<std::string, msgpack::object> response_inner;
        response_inner["id"] = msgpack::object(node.get_id(), zone);
        response_map["response"] = msgpack::object(response_inner, zone);
    }*/
    
    //-----------------------------------------------------------
    // msgpack-c packing code replaces msgpack-cxx portion here
    //-----------------------------------------------------------
    if (strcmp(method, "ping") == 0) {
        msgpack_sbuffer sbuf;
        msgpack_packer pk;

        msgpack_sbuffer_init(&sbuf);
        msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

        // Pack a map with three key-value pairs: "tid", "version" and "response"
        msgpack_pack_map(&pk, 3);

        // Key: "tid"
        msgpack_pack_str(&pk, 3);
        msgpack_pack_str_body(&pk, "tid", 3);
        msgpack_pack_str(&pk, tid.size());
        msgpack_pack_str_body(&pk, tid.data(), tid.size());

        // Key: "version"
        msgpack_pack_str(&pk, 7);
        msgpack_pack_str_body(&pk, "version", 7);
        msgpack_pack_str(&pk, strlen(NEROSHOP_DHT_VERSION));
        msgpack_pack_str_body(&pk, NEROSHOP_DHT_VERSION, strlen(NEROSHOP_DHT_VERSION));

        // Key: "response"
        msgpack_pack_str(&pk, 8);
        msgpack_pack_str_body(&pk, "response", 8);

        // Value of "response" is another map with one key "id"
        msgpack_pack_map(&pk, 1);

        const std::string& id = node.get_id();
        // Key: "id"
        msgpack_pack_str(&pk, 2);
        msgpack_pack_str_body(&pk, "id", 2);
        // Value: node id string
        msgpack_pack_str(&pk, id.size());
        msgpack_pack_str_body(&pk, id.data(), id.size());

        // Now sbuf.data contains the packed message, sbuf.size is its length

        // Copy to std::vector<uint8_t> to return
        std::vector<uint8_t> response(sbuf.data, sbuf.data + sbuf.size);

        msgpack_sbuffer_destroy(&sbuf);

        msgpack_unpacked_destroy(&result);

        return response;
    }
    
    //-----------------------------------------------------------
    // Old nlohmann-json version
    //-----------------------------------------------------------
    /*if(method == "find_node") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["target"].is_string());
        std::string target = params_object["target"].get<std::string>();
        
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        auto nodes = node.find_node(target, NEROSHOP_DHT_MAX_CLOSEST_NODES);
        if(nodes.empty()) {
            response_object["response"]["nodes"] = nlohmann::json::array();
        } else {
            std::vector<nlohmann::json> nodes_array;
            for (const auto& n : nodes) {
                nlohmann::json node_object = {
                    {"address", n->get_address()},
                    {"port", n->get_port()}
                };
                nodes_array.push_back(node_object);
            }
            response_object["response"]["nodes"] = nodes_array;
        }
    }*/
    
    //-----------------------------------------------------------
    // Transition from nlohmann-json to msgpack-cxx
    //-----------------------------------------------------------
    /*else if (method == "find_node") {
        msgpack::object* target_obj = find_key(*args_obj, "target");
        assert(target_obj && target_obj->type == msgpack::type::STR);
        std::string target(target_obj->via.str.ptr, target_obj->via.str.size);

        response_map["version"] = msgpack::object(std::string(NEROSHOP_DHT_VERSION), zone);

        std::map<std::string, msgpack::object> response_inner;
        response_inner["id"] = msgpack::object(node.get_id(), zone);

        auto nodes = node.find_node(target, NEROSHOP_DHT_MAX_CLOSEST_NODES);
        std::vector<msgpack::object> nodes_array;

        for (auto& n : nodes) {
            std::map<std::string, msgpack::object> node_obj;
            node_obj["address"] = msgpack::object(n->get_address(), zone);
            node_obj["port"] = msgpack::object(n->get_port(), zone);
            nodes_array.push_back(msgpack::object(node_obj, zone));
        }

        response_inner["nodes"] = msgpack::object(nodes_array, zone);
        response_map["response"] = msgpack::object(response_inner, zone);
    }    */
    
    //-----------------------------------------------------------
    // msgpack-c packing code replaces msgpack-cxx portion here
    //-----------------------------------------------------------
    if (strcmp(method, "find_node") == 0) {
        // Extract "target" string from args map
        const msgpack_object* target_obj = msgpack_find(args_obj, "target");
        assert(target_obj && target_obj->type == MSGPACK_OBJECT_STR);
        std::string target(target_obj->via.str.ptr, target_obj->via.str.size);

        // Get closest nodes matching target from your Node object
        // Assume it returns std::vector<NodeInfo>, where NodeInfo at least has get_address() and get_port()
        auto nodes = node.find_node(target, NEROSHOP_DHT_MAX_CLOSEST_NODES);

        msgpack_sbuffer sbuf;
        msgpack_sbuffer_init(&sbuf);
        msgpack_packer pk;
        msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);

        // Top-level map has three key-value pairs: "tid", "version" and "response"
        msgpack_pack_map(&pk, 3);

        // Key: "tid"
        msgpack_pack_str(&pk, 3);
        msgpack_pack_str_body(&pk, "tid", 3);
        msgpack_pack_str(&pk, tid.size());
        msgpack_pack_str_body(&pk, tid.data(), tid.size());

        // Key: "version"
        msgpack_pack_str(&pk, 7);
        msgpack_pack_str_body(&pk, "version", 7);
        msgpack_pack_str(&pk, strlen(NEROSHOP_DHT_VERSION));
        msgpack_pack_str_body(&pk, NEROSHOP_DHT_VERSION, strlen(NEROSHOP_DHT_VERSION));

        // Key: "response"
        msgpack_pack_str(&pk, 8);
        msgpack_pack_str_body(&pk, "response", 8);

        // Value of "response" is a map with keys "id" and "nodes"
        msgpack_pack_map(&pk, 2);

        // "id": node.get_id()
        const std::string& id = node.get_id();
        msgpack_pack_str(&pk, 2);
        msgpack_pack_str_body(&pk, "id", 2);
        msgpack_pack_str(&pk, id.size());
        msgpack_pack_str_body(&pk, id.data(), id.size());

        // "nodes": array of node objects
        msgpack_pack_str(&pk, 5);
        msgpack_pack_str_body(&pk, "nodes", 5);

        // Pack array size
        msgpack_pack_array(&pk, static_cast<uint32_t>(nodes.size()));

        for (const auto& n : nodes) {
            // Each node is packed as a map with "address" and "port"
            msgpack_pack_map(&pk, 2);

            // "address"
            const std::string& addr = n->get_address();
            msgpack_pack_str(&pk, 7);
            msgpack_pack_str_body(&pk, "address", 7);
            msgpack_pack_str(&pk, addr.size());
            msgpack_pack_str_body(&pk, addr.data(), addr.size());

            // "port"
            msgpack_pack_str(&pk, 4);
            msgpack_pack_str_body(&pk, "port", 4);
            msgpack_pack_int(&pk, n->get_port());
        }

        // Build vector for response from sbuf data
        std::vector<uint8_t> response(sbuf.data, sbuf.data + sbuf.size);

        msgpack_sbuffer_destroy(&sbuf);
        msgpack_unpacked_destroy(&result);

        return response;
    }
    
    //-----------------------------------------------------
    /*if(method == "get_providers") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
        auto peers = node.get_providers(key);
        if(node.has_key(key) || node.has_key_cached(key)) { peers.push_front(Peer{ node.get_address(), node.get_port() }); }
        if(peers.empty()) {
            response_object["response"]["values"] = nlohmann::json::array();
        } else {
            std::vector<nlohmann::json> peers_array;
            for (const auto& p : peers) {
                nlohmann::json peer_object = {
                    {"address", p.address},
                    {"port", p.port}
                };
                peers_array.push_back(peer_object);
            }
            response_object["response"]["values"] = peers_array;
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "get" && ipc_mode == false) { // For Processing Get Requests From Other Nodes
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        
        std::string value;
        if(!node.has_key(key)) {
            if(node.has_key_cached(key)) {
                value = node.get_cached(key);
            }
        } else {
            value = node.get(key);
        }
        
        if (value.empty()) {
            code = static_cast<int>(DhtResultCode::RetrieveFailed);
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = "Key not found";
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["value"] = value;
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "get" && ipc_mode == true) { // For Sending Get Requests to Other Nodes
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        
        // To get network status
        if(key == "status") {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["connected_peers"] = node.get_peer_count();
            response_object["response"]["active_peers"] = node.get_active_peer_count();
            response_object["response"]["idle_peers"] = node.get_idle_peer_count();
            response_object["response"]["data_count"] = node.get_data_count();
            response_object["response"]["data_ram_usage"] = node.get_data_ram_usage();
            response_object["response"]["host"] = node.get_address();
            response_object["response"]["port"] = node.get_port();
            response_object["response"]["network_type"] = node.get_network_type_as_string();
            auto peers_list = node.get_peers();
            if(!peers_list.empty()) {
                std::vector<nlohmann::json> peers_array;
                for (const auto& peer : peers_list) {
                    nlohmann::json peestd::cout << r_object = {
                        {"address", peer.address},
                        {"port", peer.port},
                        {"id", peer.id},
                        {"status", static_cast<int>(peer.status)},
                        {"distance", peer.distance},
                    };
                    peers_array.push_back(peer_object);
                }
                response_object["response"]["peers"] = peers_array;
            }
            response_object["tid"] = tid;
            response = nlohmann::json::to_msgpack(response_object);
            return response;
        }
                        
        std::string value = node.send_get(key);
            
        if (value.empty()) {
            code = static_cast<int>(DhtResultCode::RetrieveFailed);
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = "Key not found";
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
            response_object["response"]["value"] = value;
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "put" && ipc_mode == false) { // For Processing Put Requests from Other Nodes
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        code = (node.store(key, value) == false) 
               ? static_cast<int>(DhtResultCode::StoreFailed) 
               : static_cast<int>(DhtResultCode::Success);
            
        if(code != static_cast<int>(DhtResultCode::Success)) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "map") {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        if(node.validate(key, value)) {
            node.map(key, value);
        }
    
        response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
        response_object["response"]["id"] = node.get_id();
    }*/
    //-----------------------------------------------------
    /*if((method == "set" || method == "put") && ipc_mode == true) { // For Sending Put Requests to Other Nodes
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        assert(params_object["value"].is_string());
        std::string value = params_object["value"].get<std::string>();
        
        int put_messages_sent = node.send_put(key, value);
        code = (put_messages_sent <= 0) 
               ? static_cast<int>(DhtResultCode::StoreFailed) 
               : static_cast<int>(DhtResultCode::Success);
        log_info("Number of nodes you've sent a PUT message to: {}", put_messages_sent);
        
        if((put_messages_sent < NEROSHOP_DHT_REPLICATION_FACTOR) && (put_messages_sent > 0)) {
            code = static_cast<int>(DhtResultCode::StorePartial);
        }
        
        if(node.store(key, value)) {
            if(node.cache(key, value)) {
                if(put_messages_sent == 0) { 
                    code = static_cast<int>(DhtResultCode::StoreToSelf);
                }
                if(!Node::is_value_publishable(value)) {
                    code = static_cast<int>(DhtResultCode::Success);
                }
                
                node.map(key, value);
            }
        }
        
        if((code == static_cast<int>(DhtResultCode::Success)) || (code == static_cast<int>(DhtResultCode::StorePartial))) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "remove" && ipc_mode == true) {
        assert(request_object["args"].is_object());
        auto params_object = request_object["args"];
        assert(params_object["key"].is_string());
        std::string key = params_object["key"].get<std::string>();
        
        if(node.has_key(key)) {
            code = (node.remove(key) == false) 
                ? static_cast<int>(DhtResultCode::RemoveFailed) 
                : static_cast<int>(DhtResultCode::Success);
        }
        
        db::Sqlite3 * database = neroshop::get_database();
        if(!database) throw std::runtime_error("database is not opened");
        if(database->get_integer_params("SELECT EXISTS(SELECT key FROM hash_table WHERE key = ?1)", { key }) == 1) {
            code = (database->execute_params("DELETE FROM hash_table WHERE key = ?1", { key }) != SQLITE_OK)
                ? static_cast<int>(DhtResultCode::RemoveFailed) 
                : static_cast<int>(DhtResultCode::Success);
        }
        
        if(database->get_integer_params("SELECT EXISTS(SELECT key FROM mappings WHERE key = ?1)", { key }) == 1) {
            code = (database->execute_params("DELETE FROM mappings WHERE key = ?1", { key }) != SQLITE_OK)
                ? static_cast<int>(DhtResultCode::RemoveFailed) 
                : static_cast<int>(DhtResultCode::Success);
        }
        
        if(code != static_cast<int>(DhtResultCode::Success)) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        }
    }*/
    //-----------------------------------------------------
    /*if(method == "clear" && ipc_mode == true) {
        code = (node.remove_all() == false) 
            ? static_cast<int>(DhtResultCode::RemoveFailed) 
            : static_cast<int>(DhtResultCode::Success);
            
        if(code != static_cast<int>(DhtResultCode::Success)) {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["error"]["id"] = node.get_id();
            response_object["error"]["code"] = code;
            response_object["error"]["message"] = get_dht_result_code_as_string(static_cast<DhtResultCode>(code));
        } else {
            response_object["version"] = std::string(NEROSHOP_DHT_VERSION);
            response_object["response"]["id"] = node.get_id();
        }
    }*/
    //-----------------------------------------------------
    
    //-----------------------------------------------------------
    // Old nlohmann-json version
    //-----------------------------------------------------------
    /*response_object["tid"] = tid;
    response = nlohmann::json::to_msgpack(response_object);
    return response;*/
    
    //-----------------------------------------------------------
    // Transition from nlohmann-json to msgpack-cxx
    //-----------------------------------------------------------
    /*msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, response_map);*/
    /*#ifdef NEROSHOP_DEBUG
    msgpack_print(sbuf.data(), sbuf.size());
    msgpack_print(response_map);
    #endif*/
            
    /*response = std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    return response;*/
    return {};
}

//-----------------------------------------------------------------------------

void msgpack_print_data(const char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        std::cout << "0x" << std::setw(2) << std::setfill('0')
                  << (static_cast<int>(static_cast<unsigned char>(data[i])) & 0xff)
                  << ' ';
    }
    std::cout << std::endl;
}

//-----------------------------------------------------------------------------

void msgpack_print_object(const msgpack_object* obj, int msg_t, bool initial_call) {
    // Set text color based on message type
    if (initial_call) {
        std::cout << ((msg_t == 1) ? "\033[32m" : ((msg_t == 2) ? "\033[91m" : "\033[33m"));
    }
    
    switch (obj->type) {
        case MSGPACK_OBJECT_NIL:
            std::cout << "null";//printf("null");
            break;
        case MSGPACK_OBJECT_BOOLEAN:
            std::cout << (obj->via.boolean ? "true" : "false");//printf(obj->via.boolean ? "true" : "false");
            break;
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            std::cout << obj->via.u64;//printf("%" PRIu64, obj->via.u64);
            break;
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            std::cout << obj->via.i64;//printf("%" PRId64, obj->via.i64);
            break;
        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64:
            std::cout << obj->via.f64;//printf("%f", obj->via.f64);
            break;
        case MSGPACK_OBJECT_STR:
            std::cout << "\"";
            for (size_t i = 0; i < obj->via.str.size; ++i) {
                char c = obj->via.str.ptr[i];
                if (c == '\0') std::cout << "\\0";
                else std::cout << c;
            }
            std::cout << "\"";//printf("\"%.*s\"", (int)obj->via.str.size, obj->via.str.ptr);
            break;
        case MSGPACK_OBJECT_ARRAY:
            std::cout << "[";
            for (uint32_t i = 0; i < obj->via.array.size; ++i) {
                msgpack_print_object(&obj->via.array.ptr[i], msg_t, false);
                if (i + 1 < obj->via.array.size)
                    std::cout << ",";
            }
            std::cout << "]";/*printf("[");
            for (uint32_t i = 0; i < obj->via.array.size; ++i) {
                msgpack_print_object(&obj->via.array.ptr[i], msg_t, false);
                if (i + 1 < obj->via.array.size) printf(",");
            }
            printf("]");*/
            break;
        case MSGPACK_OBJECT_MAP:
            std::cout << "{";
            for (uint32_t i = 0; i < obj->via.map.size; ++i) {
                const msgpack_object_kv* kv = &obj->via.map.ptr[i];
                std::cout << "\"";
                for (size_t j = 0; j < kv->key.via.str.size; ++j) {
                    char c = kv->key.via.str.ptr[j];
                    if (c == '\0') std::cout << "\\0";
                    else std::cout << c;
                }
                std::cout << "\":";
                msgpack_print_object(&kv->val, msg_t, false);
                if (i + 1 < obj->via.map.size)
                    std::cout << ",";
            }
            std::cout << "}";/*printf("{");
            for (uint32_t i = 0; i < obj->via.map.size; ++i) {
                const msgpack_object_kv* kv = &obj->via.map.ptr[i];
                printf("\"%.*s\":", (int)kv->key.via.str.size, kv->key.via.str.ptr);
                msgpack_print_object(&kv->val, msg_t, false);
                if (i + 1 < obj->via.map.size) printf(",");
            }
            printf("}");*/
            break;
        default:
            std::cout << "\"<unknown>\"";//printf("\"<unknown>\"");
    }
    
    // Reset color formatting
    if (initial_call) {
        std::cout << "\033[0m";
        std::cout << "\n";
    }
}

//-----------------------------------------------------------------------------

void msgpack_print(const char* data, size_t len) {
    msgpack_unpacked result;
    size_t offset = 0;

    msgpack_unpacked_init(&result);
    msgpack_unpack_return ret = msgpack_unpack_next(&result, data, len, &offset);

    if (ret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;
        msgpack_print_object(&obj, 0);
        printf("\n");
    } else {
        fprintf(stderr, "Failed to unpack data\n");
    }

    msgpack_unpacked_destroy(&result);
}

//-----------------------------------------------------------------------------

} // namespace rpc

} // namespace neroshop
#endif // NEROSHOP_USE_MSGPACK
