#pragma once
#ifndef ZMQ_SERVER_HPP_NEROSHOP
#define ZMQ_SERVER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_LIBZMQ)
#include <zmq.h>

#include <iostream>
#include <string>
#include <vector>

namespace neroshop {

class ZmqServer {
public:
    ZmqServer(const std::string& endpoint, bool async = false);
    ~ZmqServer();

    void receive(std::string& message);
    void receive(std::vector<uint8_t>& message);
    void receive_poll(std::string& message);
    void send(const std::string& message);
    void send(const std::vector<uint8_t>& message);
    void send(const std::string& message, const std::string& topic); // for async
    void close();

private:
    void init_socket(const std::string& endpoint, bool async);

    void* context_ = nullptr;
    void* socket_ = nullptr;
};

}

#endif // NEROSHOP_USE_LIBZMQ
#endif
