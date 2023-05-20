#pragma once
#ifndef ZMQ_CLIENT_HPP_NEROSHOP
#define ZMQ_CLIENT_HPP_NEROSHOP

#if defined(NEROSHOP_USE_LIBZMQ)
#include <zmq.h>

#include <iostream>
#include <string>
#include <vector>

namespace neroshop {

class ZmqClient {
public:
    ZmqClient(const std::string& endpoint);
    ~ZmqClient();

    void receive(std::string& message);
    void receive(std::vector<uint8_t>& message);
    void send(const std::string& message);
    void send(const std::vector<uint8_t>& data);
    void send_receive(const std::string& message, std::string& response);
    void close();

private:
    void init_socket(const std::string& endpoint);

    void* context_ = nullptr;
    void* socket_ = nullptr;
};
 
}

#endif // NEROSHOP_USE_LIBZMQ
#endif
