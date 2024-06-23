#pragma once

#include <iostream>
#include <string>

namespace neroshop {

enum class DhtResultCode {
    Success = 0,
    Generic, // Generic error code.
    Timeout, // Operation timed out.
    InvalidKey, // Invalid key provided.
    InvalidValue, // Invalid value provided.
    InvalidToken,
    NodeNotFound, // The requested node is not found in the DHT.
    BucketFull, // The routing table bucket is full and cannot accept more nodes.
    StoreFailed, // Failed to store the key-value pair in the DHT.
    StorePartial, // Stored to less than NEROSHOP_DHT_REPLICATION_FACTOR nodes but not zero nodes
    StoreToSelf, // Failed to store to the closest nodes but succeeded in storing in your own node
    RetrieveFailed, // Failed to retrieve the value from the DHT.
    JoinFailed, // Failed to join the Kademlia network.
    PingFailed, // Failed to ping the target node.
    InvalidOperation, // Invalid or unsupported operation requested.
    NetworkError, // A network error occurred during the operation.
    InvalidRequest,
    ParseError,
    DataVerificationFailed,
    DataRejected,
    RemoveFailed,
};

static std::string get_dht_result_code_as_string(DhtResultCode result_code) {
    switch (result_code) {
        case DhtResultCode::Success:
            return "Success";
        case DhtResultCode::Generic:
            return "Error";
        case DhtResultCode::Timeout:
            return "Timeout";
        case DhtResultCode::InvalidKey:
            return "Invalid key";
        case DhtResultCode::InvalidValue:
            return "Invalid value";
        case DhtResultCode::InvalidToken:
            return "Invalid token";            
        case DhtResultCode::NodeNotFound:
            return "Node not found";
        case DhtResultCode::BucketFull:
            return "Bucket full";
        case DhtResultCode::StoreFailed:
            return "Store failed";
        case DhtResultCode::StorePartial:
            return "Store partial";
        case DhtResultCode::StoreToSelf:
            return "Store to self";
        case DhtResultCode::RetrieveFailed:
            return "Retrieve failed";
        case DhtResultCode::JoinFailed:
            return "Join failed";
        case DhtResultCode::PingFailed:
            return "Ping failed";
        case DhtResultCode::InvalidOperation:
            return "Invalid operation";
        case DhtResultCode::NetworkError:
            return "Network error";
        case DhtResultCode::InvalidRequest:
            return "Invalid request";
        case DhtResultCode::ParseError:
            return "Parse error";
        case DhtResultCode::DataVerificationFailed:
            return "Data verification failed";
        case DhtResultCode::DataRejected:
            return "Data rejected";
        case DhtResultCode::RemoveFailed:
            return "Remove failed";
        default:
            return "Unknown result code";
    }
}

}
