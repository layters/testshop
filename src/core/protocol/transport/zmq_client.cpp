#include "zmq_client.hpp"

#if defined(NEROSHOP_USE_LIBZMQ)
#include <cstring>
#include <stdexcept>

neroshop::ZmqClient::ZmqClient(const std::string& endpoint) {
        init_socket(endpoint);
}

neroshop::ZmqClient::~ZmqClient() {
        close();
}

void neroshop::ZmqClient::receive(std::string& message) {
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int rc = zmq_msg_recv(&msg, socket_, 0);
        if (rc == -1) {
            zmq_msg_close(&msg);
            throw std::runtime_error("Failed to receive message");
        }
        message.assign(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
        zmq_msg_close(&msg);
}

void neroshop::ZmqClient::receive(std::vector<uint8_t>& message) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int rc = zmq_msg_recv(&msg, socket_, 0);
    if (rc == -1) {
        zmq_msg_close(&msg);
        throw std::runtime_error("Failed to receive message");
    }
    message.resize(zmq_msg_size(&msg));
    memcpy(message.data(), zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);
}

void neroshop::ZmqClient::send(const std::string& message) {
        zmq_msg_t msg;
        zmq_msg_init_size(&msg, message.size());
        memcpy(zmq_msg_data(&msg), message.c_str(), message.size());
        int rc = zmq_msg_send(&msg, socket_, 0);
        if (rc == -1) {
            zmq_msg_close(&msg);
            throw std::runtime_error("Failed to send message");
        }
        zmq_msg_close(&msg);
}

void neroshop::ZmqClient::send(const std::vector<uint8_t>& message) {
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, message.size());
    memcpy(zmq_msg_data(&msg), message.data(), message.size());
    int rc = zmq_msg_send(&msg, socket_, 0);
    if (rc == -1) {
        zmq_msg_close(&msg);
        throw std::runtime_error("Failed to send message");
    }
    zmq_msg_close(&msg);
}

void neroshop::ZmqClient::send_receive(const std::string& message, std::string& response) {
    send(message);
    receive(response);
}

void neroshop::ZmqClient::close() {
        zmq_close(socket_);
        zmq_ctx_destroy(context_);
}

void neroshop::ZmqClient::init_socket(const std::string& endpoint) {
        context_ = zmq_ctx_new();
        if (!context_) {
            throw std::runtime_error("Failed to create ZMQ context");
        }
        socket_ = zmq_socket(context_, ZMQ_REQ);
        if (!socket_) {
            zmq_ctx_destroy(context_);
            throw std::runtime_error("Failed to create ZMQ socket");
        }
        int rc = zmq_connect(socket_, endpoint.c_str());
        if (rc != 0) {
            zmq_close(socket_);
            zmq_ctx_destroy(context_);
            throw std::runtime_error("Failed to connect ZMQ socket");
        }
}

/*int main() {

}*/ // g++ -D NEROSHOP_USE_LIBZMQ zmq_client.cpp zmq_server.cpp -lzmq
#endif // NEROSHOP_USE_LIBZMQ
