#include "sam_client.hpp"

#include <fstream>
#include <stdexcept>

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv) {
    try {
        require(argc == 2, "expected test mode");
        const std::string mode = argv[1];
        neroshop::SamClient client(neroshop::SamSessionStyle::Datagram, "regression");
        require(client.hello(client.get_session_socket()).result == neroshop::SamResultType::Ok,
                "HELLO failed");
        if (mode == "idle") {
            require(client.datagram_receive().empty(), "idle receive must be empty");
            const int sender = ::socket(AF_INET, SOCK_DGRAM, 0);
            require(sender >= 0, "sender socket failed");
            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_port = htons(client.get_port());
            inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
            const std::vector<uint8_t> packet{'d', '\n', 0, 255, 'x'};
            require(::sendto(sender, packet.data(), packet.size(), 0,
                             reinterpret_cast<sockaddr*>(&address), sizeof(address)) ==
                    static_cast<ssize_t>(packet.size()), "sendto failed");
            ::close(sender);
            // Wait for readiness without changing the client's non-blocking socket.
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(client.get_client_socket(), &readable);
            timeval timeout{2, 0};
            require(::select(client.get_client_socket() + 1, &readable, nullptr, nullptr,
                             &timeout) == 1, "UDP delivery timed out");
            require(client.datagram_receive() == packet, "binary datagram changed");
            require(client.datagram_receive().empty(), "drained receive must be empty");
            ::close(client.get_client_socket());
            bool threw = false;
            try { client.datagram_receive(); }
            catch (const std::runtime_error&) { threw = true; }
            require(threw, "real socket errors must still throw");
        } else {
            if (mode == "invalid") {
                bool threw = false;
                try { client.session_prepare(); }
                catch (const std::runtime_error&) { threw = true; }
                require(threw, "truncated destination must be rejected");
            } else {
                client.session_prepare();
                std::ifstream expected("expected.pub");
                std::string public_key;
                expected >> public_key;
                require(client.get_public_key() == public_key, "public destination mismatch");
                std::ifstream expected_address("expected.address");
                std::string address;
                expected_address >> address;
                require(client.get_i2p_address() == address,
                        "restored address mismatch");
                client.session_create();
            }
            ::close(client.get_client_socket());
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
