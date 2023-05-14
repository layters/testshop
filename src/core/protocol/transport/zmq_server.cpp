#include "zmq_server.hpp"

#if defined(NEROSHOP_USE_LIBZMQ)
#include <cstring>
#include <stdexcept>

neroshop::ZmqServer::ZmqServer(const std::string& endpoint, bool async) {
        init_socket(endpoint, async);
}

neroshop::ZmqServer::~ZmqServer() {
        close();
}

void neroshop::ZmqServer::receive(std::string& message) {
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

void neroshop::ZmqServer::receive(std::vector<uint8_t>& message) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int rc = zmq_msg_recv(&msg, socket_, 0);
    if (rc == -1) {
        zmq_msg_close(&msg);
        throw std::runtime_error("Failed to receive message");
    }
    size_t size = zmq_msg_size(&msg);
    message.resize(size);
    memcpy(message.data(), zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);
}

// use the zmq_poll() function to wait for incoming messages from the client. The zmq_poll() function blocks until there is at least one message available to receive, or until a timeout occurs. You can set the timeout to zero to make the function return immediately if there are no messages to receive.
void neroshop::ZmqServer::receive_poll(std::string& message) {
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    
    zmq_pollitem_t items[] = {
        { socket_, 0, ZMQ_POLLIN, 0 }
    };
    
    int rc = zmq_poll(items, 1, 0);
    if (rc == -1) {
        zmq_msg_close(&msg);
        throw std::runtime_error("Failed to poll for messages");
    }
    
    if (items[0].revents & ZMQ_POLLIN) {
        rc = zmq_msg_recv(&msg, socket_, 0);
        if (rc == -1) {
            zmq_msg_close(&msg);
            throw std::runtime_error("Failed to receive message");
        }
        message.assign(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
        zmq_msg_close(&msg);
    } else {
        message.clear();
    }
}

void neroshop::ZmqServer::send(const std::string& message) {
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

void neroshop::ZmqServer::send(const std::vector<uint8_t>& message) {
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

void neroshop::ZmqServer::send(const std::string& message, const std::string& topic) {
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, message.size());
    memcpy(zmq_msg_data(&msg), message.c_str(), message.size());
    int rc = zmq_msg_send(&msg, socket_, ZMQ_SNDMORE);
    if (rc == -1) {
        zmq_msg_close(&msg);
        throw std::runtime_error("Failed to send message");
    }
    zmq_msg_t topic_msg;
    zmq_msg_init_size(&topic_msg, topic.size());
    memcpy(zmq_msg_data(&topic_msg), topic.c_str(), topic.size());
    rc = zmq_msg_send(&topic_msg, socket_, 0);
    if (rc == -1) {
        zmq_msg_close(&msg);
        zmq_msg_close(&topic_msg);
        throw std::runtime_error("Failed to send message");
    }
    zmq_msg_close(&msg);
    zmq_msg_close(&topic_msg);
}

void neroshop::ZmqServer::close() {
        zmq_close(socket_);
        zmq_ctx_destroy(context_);
}

void neroshop::ZmqServer::init_socket(const std::string& endpoint, bool async) {
        context_ = zmq_ctx_new();
        if (!context_) {
            throw std::runtime_error("Failed to create ZMQ context");
        }
        socket_ = zmq_socket(context_, (async == true) ? ZMQ_PUB: ZMQ_REP);
        if (!socket_) {
            zmq_ctx_destroy(context_);
            throw std::runtime_error("Failed to create ZMQ socket");
        }
        int rc = zmq_bind(socket_, endpoint.c_str());
        if (rc != 0) {
            zmq_close(socket_);
            zmq_ctx_destroy(context_);
            throw std::runtime_error("Failed to bind ZMQ socket");
        }
}

//int main() {} // g++ -D NEROSHOP_USE_LIBZMQ zmq_client.cpp zmq_server.cpp -lzmq

#endif // NEROSHOP_USE_LIBZMQ
