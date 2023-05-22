#pragma once

#include "node.hpp"
#include "routing_table.hpp"

// TODO, // DHT blacklisted nodes and probably callbacks

namespace neroshop {

enum class KadResultCode {
    Success = 0,
    Generic, // Generic error code.
    Timeout, // Operation timed out.
    Invalid_Key, // Invalid key provided.
    Invalid_Value, // Invalid value provided.
    Node_Not_Found, // The requested node is not found in the DHT.
    Bucket_Full, // The routing table bucket is full and cannot accept more nodes.
    Store_Failed, // Failed to store the key-value pair in the DHT.
    Retrieve_Failed, // Failed to retrieve the value from the DHT.
    Join_Failed, // Failed to join the Kademlia network.
    Ping_Failed, // Failed to ping the target node.
    Invalid_Operation, // Invalid or unsupported operation requested.
    Network_Error, // A network error occurred during the operation.
};

}
