#pragma once

#ifndef MSGPACK_HPP_NEROSHOP
#define MSGPACK_HPP_NEROSHOP

#if defined(NEROSHOP_USE_MSGPACK)

#include <iostream>
#include <string>
#include <vector>
#include <cstdint> // uint8_t
#include <map>
#include <variant>

/*#define MSGPACK_API_VERSION 3 // -DMSGPACK_API_VERSION=
#include <msgpack.hpp> // msgpack::object_handle, msgpack::object
#if !defined(MSGPACK_DEFAULT_API_VERSION)
#define MSGPACK_DEFAULT_API_VERSION 3
#endif*/
#include <msgpack.h>

namespace neroshop {

class Node; // forward declaration

namespace rpc {
    std::vector<uint8_t> msgpack_process(const std::vector<uint8_t>& request, Node& node, bool ipc_mode = false);
    const msgpack_object* msgpack_find(const msgpack_object* map_obj, const char* key); // finds a map key
    std::string msgpack_object_to_json(const msgpack_object* obj);
    void msgpack_print_data(const char* data, size_t len); // prints msgpack::sbuffer (hex) (use after pack)
    void msgpack_print_object(const msgpack_object* obj, int msg_t = 0, bool initial_call = true);
    void msgpack_print(const char* data, size_t len); // converts to msgpack_object then prints
}

} // namespace neroshop

//-----------------------------------------------------------------------------

/*struct MsgValue;

using MsgMap = std::map<std::string, MsgValue>;

// Define MsgValue as a variant derived type
struct MsgValue : std::variant<std::string, uint16_t, int, MsgMap> {
    using std::variant<std::string, uint16_t, int, MsgMap>::variant;
};

} // namespace neroshop

namespace msgpack {
inline namespace v2 {
namespace adaptor {

template<>
struct pack<neroshop::MsgValue> {
    template<typename Stream>
    msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, const neroshop::MsgValue& v) const {
        std::visit([&o](auto&& arg) { o.pack(arg); }, v);
        return o;
    }
};

template<>
struct convert<neroshop::MsgValue> {
    const msgpack::object& operator()(const msgpack::object& o, neroshop::MsgValue& v) const {
        switch(o.type) {
            case msgpack::type::STR: v = o.as<std::string>(); break;
            case msgpack::type::POSITIVE_INTEGER:
            case msgpack::type::NEGATIVE_INTEGER:
                try { v = o.as<uint16_t>(); } catch (...) { v = o.as<int>(); } break;
            case msgpack::type::MAP: {
                neroshop::MsgMap map;
                o.convert(map);
                v = std::move(map);
                break;
            }
            default:
                throw msgpack::type_error();
        }
        return o;
    }
};

template<>
struct object<neroshop::MsgValue> {
    void operator()(msgpack::object& o, const neroshop::MsgValue& v) const {
        std::visit([&o](auto&& arg) { o = msgpack::object(arg, o.zone); }, v);
    }
};

template<>
struct object_with_zone<neroshop::MsgValue> {
    void operator()(msgpack::object::with_zone& o, const neroshop::MsgValue& v) const {
        std::visit([&o](auto&& arg) {
            new (&o) msgpack::object::with_zone(arg, o.zone);
        }, v);
    }
};

} // namespace adaptor
} // inline namespace v2
} // namespace msgpack*/

#endif // NEROSHOP_USE_MSGPACK

#endif // MSGPACK_HPP_NEROSHOP
