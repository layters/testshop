#pragma once

#ifndef PROTOBUF_HPP_NEROSHOP
#define PROTOBUF_HPP_NEROSHOP

#if defined(NEROSHOP_USE_PROTOBUF)
#include "../../../../build/proto/message.pb.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstdint> // uint8_t

namespace neroshop {

class Node; // forward declaration

namespace rpc {
    std::vector<uint8_t> protobuf_process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode = false);
}

}

#endif // NEROSHOP_USE_PROTOBUF

#endif // PROTOBUF_HPP_NEROSHOP
