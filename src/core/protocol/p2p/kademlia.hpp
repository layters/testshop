#pragma once

#include "node.hpp"
#include "routing_table.hpp"

// TODO: DHT blacklisted nodes

namespace neroshop {

enum class KadResultCode {
    Success = 0,
    Generic, // Generic error code.
    Timeout, // Operation timed out.
    InvalidKey, // Invalid key provided.
    InvalidValue, // Invalid value provided.
    InvalidToken,
    NodeNotFound, // The requested node is not found in the DHT.
    BucketFull, // The routing table bucket is full and cannot accept more nodes.
    StoreFailed, // Failed to store the key-value pair in the DHT.
    StorePartial, StoreToSelf = StorePartial,// Partial success in storing the value - when you fail to store to the closest nodes but succeed in storing in your own node
    RetrieveFailed, // Failed to retrieve the value from the DHT.
    JoinFailed, // Failed to join the Kademlia network.
    PingFailed, // Failed to ping the target node.
    InvalidOperation, // Invalid or unsupported operation requested.
    NetworkError, // A network error occurred during the operation.
    InvalidRequest,
    ParseError,
    DataVerificationFailed,
};

namespace kademlia {

static std::string get_result_code_as_string(KadResultCode result_code) {
    switch (result_code) {
        case KadResultCode::Success:
            return "Success";
        case KadResultCode::Generic:
            return "Error";
        case KadResultCode::Timeout:
            return "Timeout";
        case KadResultCode::InvalidKey:
            return "Invalid key";
        case KadResultCode::InvalidValue:
            return "Invalid value";
        case KadResultCode::InvalidToken:
            return "Invalid token";            
        case KadResultCode::NodeNotFound:
            return "Node not found";
        case KadResultCode::BucketFull:
            return "Bucket full";
        case KadResultCode::StoreFailed:
            return "Store failed";
        case KadResultCode::StorePartial:
            return "Store partial";            
        case KadResultCode::RetrieveFailed:
            return "Retrieve failed";
        case KadResultCode::JoinFailed:
            return "Join failed";
        case KadResultCode::PingFailed:
            return "Ping failed";
        case KadResultCode::InvalidOperation:
            return "Invalid operation";
        case KadResultCode::NetworkError:
            return "Network error";
        case KadResultCode::InvalidRequest:
            return "Invalid request";
        case KadResultCode::ParseError:
            return "Parse error";
        default:
            return "Unknown result code";
    }
}

}

}
