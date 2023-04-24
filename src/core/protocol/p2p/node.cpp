#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "routing_table.hpp"
#include "../transport/ip_address.hpp"
#include "../rpc/bencode.hpp"
#include "../rpc/krpc.hpp"

#include <cstring> // memset
#include <future>
#include <iomanip> // std::set*
#include <cassert>

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port, bool local) : sockfd(-1), bootstrap(false) { 
    // Convert URL to IP (in case it happens to be a url)
    std::string ip_address = url_to_ip(address);
    // Generate a random node ID - use public ip address for uniqueness
    public_ip_address = (local) ? get_public_ip_address() : ip_address;
    id = generate_node_id(public_ip_address, port);
    // TODO: maybe shorten the sha3-256 256-bit (32 bytes or 64 hex characters) node id to a 160-bit (20 bytes or 40 hex characters) node id to conform to the 160-bit requirement of Kademlia or not?
    //---------------------------------------------------------------------------
    memset(&storage, 0, sizeof(storage));
    if(is_ipv4(ip_address)) storage.ss_family = AF_INET;
    if(is_ipv6(ip_address)) storage.ss_family = AF_INET6;
    //std::cout << "Socket IP Type: " << ((storage.ss_family == AF_INET6) ? "IPv6" : "IPv4\n");
    //---------------------------------------------------------------------------
    // If this node is a local node that you own
    if(local == true) {
        // Create a UDP socket
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if(sockfd < 0) {
            perror("socket");
            throw std::runtime_error("::socket failed");
        }

        // Set socket options with setsockopt
        /*int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));*/
        // Set to broadcast mode - will broadcast a message to other nodes within the local network/to all devices on the local network, but it does not send the message beyond the network
        // This is good for testing multiple local nodes
        // In general, the first node or bootstrap node is the one that initiates the network and starts the communication. It can use broadcasting to announce its presence and make itself discoverable to other nodes. Once other nodes join the network, they can use other means, such as peer-to-peer discovery or querying a directory service, to find other nodes.
        /*if(is_bootstrap_node()) {
            int enable_broadcast = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(enable_broadcast));
        }*/

        // set a timeout of TIMEOUT_VALUE seconds for recvfrom
        struct timeval tv;
        tv.tv_sec = TIMEOUT_VALUE;  // timeout in seconds
        tv.tv_usec = 0; // timeout in microseconds
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
            std::cerr << "Error setting socket options" << std::endl;
            close(sockfd);
            exit(0);
        }
        
        // Make sockfd non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        if(flags == -1) {
            perror("fcntl");
        }
        if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl");
        }

        // To avoid "bind" error and allow multiple nodes to run on the same IP address, we can use different port numbers for each local node we create
        // Bind to port
        int port_dynamic = port; // initial port number
        const int MAX_PORT_NUM = 65535; // port cannot go past this number
        while(true) {        
            if(storage.ss_family == AF_INET) {
                sockin = {0};//memset(&sockin, 0, sizeof(sockin)); // both approaches are valid, but memset is more reliable and portable
                sockin.sin_family = storage.ss_family;
                sockin.sin_port = htons(port_dynamic);//htons(std::stoi(std::to_string(port_dynamic))); // the second approach may be useful in cases where the port number is being manipulated as a string or integer elsewhere in the code
                /*if(is_bootstrap_node()) { sockin.sin_addr.s_addr = htonl(INADDR_BROADCAST); } // broadcast=255.255.255.255
                else {*/
                    if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin.sin_addr) <= 0) { //sockin.sin_addr.s_addr = htonl(INADDR_ANY) - binds to any available network interface // inet_addr(ip_address.c_str()) - binds to a specific ip // recommended to use inet_pton() over inet_addr() when working with networking in modern systems.
                        perror("inet_pton");
                    }
                //}
            }
            if(storage.ss_family == AF_INET6) {
                memset(&sockin6, 0, sizeof(sockin6));
                sockin6.sin6_family = storage.ss_family;
                sockin6.sin6_port = htons(port_dynamic);
                /*if(is_bootstrap_node()) { inet_pton(AF_INET6, "ff02::1", &sockin6.sin6_addr); } // broadcast=ff02::1 ; multicast=ff02::1:2
                else {*/
                    if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin6.sin6_addr) <= 0) { 
                        perror("inet_pton");
                    }
                //}
            }
            
            if(bind(sockfd, (storage.ss_family == AF_INET6) ? (struct sockaddr *)&sockin6 : (struct sockaddr *)&sockin, (storage.ss_family == AF_INET6) ? sizeof(sockin6) : sizeof(sockin)) == 0) {
                // Update node_id if the port ever changes
                id = generate_node_id(public_ip_address, port_dynamic);
                std::cout << "DHT node bound to port " << port_dynamic << std::endl;
                break;
            }
            std::cout << "Port " << port_dynamic << " already in use." << std::endl;
            port_dynamic++;
            if (port_dynamic > MAX_PORT_NUM) {
                throw std::runtime_error("Unable to bind to any available port.");
            }
        }

        // Node is now bound to a unique port number
    }
    //---------------------------------------------------------------------------
    // If this is an external node that you do not own
    if(local == false) {
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket");
        }
        // This socket is not meant to be bound to any port. It will only be used to retrieve the IP address and port number
    
        if(storage.ss_family == AF_INET) {
            memset(&sockin, 0, sizeof(sockin));
            sockin.sin_family = storage.ss_family;
            sockin.sin_port = htons(port);
            if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin.sin_addr) <= 0) { 
                perror("inet_pton");
            }
        }
        if(storage.ss_family == AF_INET6) {
            memset(&sockin6, 0, sizeof(sockin6));
            sockin6.sin6_family = storage.ss_family;
            sockin6.sin6_port = htons(port);
            if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin6.sin6_addr) <= 0) { 
                perror("inet_pton");
            }
        }
        // Set socket options, such as timeout or buffer size, if needed
    }
}

neroshop::Node::~Node() {
    if(sockfd > 0) {
        close(sockfd);
        sockfd = -1;
    }
}

std::string neroshop::Node::generate_node_id(const std::string& address, int port) {
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

// Define the list of bootstrap nodes
std::vector<neroshop::Peer> bootstrap_nodes = {
    {"127.0.0.1", DEFAULT_PORT},
    {"node.neroshop.org", DEFAULT_PORT}, // $ ping neroshop.org # or nslookup neroshop.org
};

void neroshop::Node::join(std::function<void()> on_join_callback) {
    if(sockfd < 0) throw std::runtime_error("socket is dead");

    // Create the routing table with the vector of nodes
    if(!routing_table.get()) {
        routing_table = std::make_unique<RoutingTable>(std::vector<Node*>{});
    }
    
    // Was going to add this node to the routing table, but nodes do not typically add their own public IP address to their routing table
    // Bootstrap node should ping itself in case its the only node in the network?
    ////if(bootstrap) { std::cout << "Skipping join for bootstrap node\n"; return; }//bootstrap_nodes.push_back({"127.0.0.1", DEFAULT_PORT});
    int port_dynamic = DEFAULT_PORT;
    // Bootstrap the DHT node with a set of known nodes
    for (const auto& bootstrap_node : bootstrap_nodes) {
        std::cout << "Joining bootstrap node - " << bootstrap_node.address << ":" << bootstrap_node.port << "\n";

        // Ping each known node to confirm that it is online - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, (bootstrap_node.address == "127.0.0.1") ? port_dynamic : bootstrap_node.port)) {
            std::cerr << "ping: failed to ping bootstrap node\n"; continue;
        }
        
        // Add the bootstrap node to routing table ; dht_insert_node - stores the node in the routing table for later use.
        auto new_node = std::make_unique<Node>(bootstrap_node.address, (bootstrap_node.address == "127.0.0.1") ? port_dynamic : bootstrap_node.port, false);
        routing_table->add_node(std::move(new_node).get());
        
        port_dynamic++;
        // Send a "find_node" message to the bootstrap node and wait for a response message
        auto nodes = find_node(new_node->get_id());
        if(nodes.empty()) {
            std::cerr << "find_node: No nodes found\n"; continue;
        }
        // Then add nodes to the routing table
    }
    
    // Print the contents of the routing table
    routing_table->print_table();
    
    // Try listening for a connection
    // Edit: UDP socket cannot listen for connections in the same way that a TCP socket can.
    on_join_callback(); // Call the callback function after the node joins the network
}

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs. 
    if(sockfd < 0) throw std::runtime_error("socket is dead");
    ////assert(port == get_port()); // both this->sockin and dest_addr must be listening on the same port for the communication to happen between them. But if I'm testing nodes on a single computer, it is necessary to comment this out
    
    std::string transaction_id = rpc::krpc::generate_transaction_id();
    std::string message_type = "q"; // q = query
    std::string query_type = "ping";
    bencode_dict arguments = {{"id", this->id}};
    //std::string client_version = "NS01"; // use XX for unknown clients or custom clients

    bencode_dict ping_dict;
    ping_dict["t"] = transaction_id;
    ping_dict["y"] = message_type;
    ping_dict["q"] = query_type;
    ping_dict["a.id"] = this->id;//ping_dict["a"] = arguments; // "a" must be a dict with a single key "id" containing the querying node's id
    //ping_dict["v"] = client_version; // not necessary to include a "v" key in the message. The "v" key is specific to the Kademlia RPC protocol used by BitTorrent clients, and is used to identify the client and version in the context of the protocol.

    std::string ping_message = bencode::encode(ping_dict);

    // Create a sockaddr_in structure containing the IP address and port number of the destination node.
    /*struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(ip_address.c_str());*/ // ONLY works with IPv4 addresses in dotted-decimal notation (e.g., "192.168.0.1") so "router.bittorrent.com:6881" will not work. It does not support domain names like "router.bittorrent.com". If you pass a domain name to inet_addr, it will return the value INADDR_NONE, which is defined as 0xffffffff or 255.255.255.255 in dotted-decimal notation.
    // TODO: add support for ipv6 addresses
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4
    hints.ai_socktype = SOCK_DGRAM; // use UDP //SOCK_STREAM; // use TCP
    if (getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error resolving hostname" << std::endl; // probably the wrong family
        return false;
    }
    struct sockaddr_in dest_addr;
    if(memcpy(&dest_addr, res->ai_addr, res->ai_addrlen) == NULL) {
        freeaddrinfo(res);
        std::cerr << "Error copying memory" << std::endl;
        return false;
    }
    freeaddrinfo(res);
    
    // Send the ping message to the destination node using the sendto() function.
    ssize_t bytes_sent = sendto(sockfd, ping_message.c_str(), ping_message.length(), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (bytes_sent == -1) {
        std::cerr << "Error sending data to " << inet_ntoa(dest_addr.sin_addr) << ": " << strerror(errno) << std::endl;
        return false;
    } else {
        std::cout << "ping_message (sent): \033[91m" << ping_message << " \033[0m(" << bytes_sent << " bytes)\n";
    }
    
    // Set a timeout for the receive operation (in microseconds).
    const int timeout_usec = TIMEOUT_VALUE * 1000000;

    // Wait for the pong response from the destination node.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout_usec;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);
    for (int i = TIMEOUT_VALUE; i > 0; i--) {
        std::cout << "Waiting for response... (" << i << " seconds remaining)" << std::endl;
        int select_result = select(sockfd + 1, &read_fds, NULL, NULL, &tv);
        if (select_result == -1) {
            perror("select");
            return false;
        } else if (select_result == 0) {
            std::cerr << "Timeout waiting for response from " << address << ":" << port << std::endl;
            return false;
        }
    }
    
    // Receive the pong response from the destination node using the recvfrom() function.
    char buffer[1024];
    int recvlen = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if(recvlen == -1) {
        perror("recvfrom");
        return false;
    } 
    std::string pong_message(buffer, recvlen);
    if(pong_message.find("pong") != std::string::npos) std::cout << "pong_message (received): \033[32m" << pong_message << "\033[0m\n";  // The first node in the network usually pings itself so it would receive a ping rather than a pong
    else std::cout << "\033[33myou've pinged yourself as you are likely the only node in the network\033[0m\n";
    
    // Parse the pong response to extract the transaction ID, message type, and response fields.
    size_t size = 0;
    bencode_dict pong_dict = bencode::decode_dict(pong_message, size);

    std::string pong_transaction_id, pong_message_type, pong_return_id, pong_return_type;
    if (pong_dict.count("t") > 0) pong_transaction_id = std::get<std::string>(pong_dict["t"]); // In the pong message, the transaction_id parameter should match the transaction_id parameter from the corresponding ping message, so that the pong can be properly matched with its corresponding ping.
    if (pong_dict.count("y") > 0) pong_message_type = std::get<std::string>(pong_dict["y"]); // message type should be "r" (response) to indicate that this is a response message.//if (pong_dict.count("r") > 0) std::string pong_response_type = std::get<std::string>(pong_dict["r"]);//pong_dict["r"] = bencode_dict{}; // empty additional response data to indicate successful ping - optional and unnecessary
    if (pong_dict.count("r") > 0) pong_return_type = std::get<std::string>(pong_dict["r"]); // "pong"
    if (pong_dict.count("r.id") > 0) pong_return_id = std::get<std::string>(pong_dict["r.id"]); // "id"

    assert(transaction_id == pong_transaction_id && "Transaction IDs do not match");

    return true;
}

std::vector<neroshop::Node*> neroshop::Node::find_node(const std::string& target_id) {    
    std::vector<Node*> nodes;
    std::string transaction_id = rpc::krpc::generate_transaction_id();

    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "find_node";
    query["a.id"] = this->id;//query["a"] = bencode_list { this->id, target_id }; // A find_node query has two arguments, "id" containing the node ID of the querying node, and "target" containing the ID of the node sought by the queryer
    query["a.target"] = target_id;
    //, "address": get_public_ip_address() + ":" + std::to_string(DEFAULT_PORT);
    
    std::string find_node_message = bencode::encode(query);
    std::cout << "find_node_message (sent): \033[91m" << find_node_message << "\033[0m\n";
    
    // Get the nodes from the routing table that are closest to the target node
    std::vector<Node*> closest_nodes = routing_table->find_closest_nodes(target_id);
    /*std::cout << "Closest nodes:\n";
    for (auto node : closest_nodes) {
        std::cout << node->get_ip_address() << ":" << node->get_port() << " (id: " << node->get_id() << ")\n";
    }*/
    // Send a find_node message to the bootstrap node
    /*
    // ...

    // Add the nodes returned by the find_node message to the routing table
    for (auto node : nodes) {
        if (node->id != this->id && !routing_table->has_node(node->id)) { // don't add yourself to the routing table
            routing_table->add_node(node);
        }
    }*/
    //--------------------------------------------------------
    // Handling find_node messages:
    // Find nodes that are closest to the target ID
    ////std::vector<Node> closest_nodes = routing_table.get_closest_nodes(target_id);

    // Create a "nodes" message with the closest nodes
    ////std::string nodes_message = create_nodes_message(closest_nodes);
    /*size_t pos = 0;
    bencode_dict nodes_dict = bencode::decode_dict(nodes_message, pos);*/
    // Reply back with a "nodes" message
    ////send_message(sender_ip, sender_port, nodes_message);

    /*bencode_list decoded_list = std::get<bencode_list>(nodes_dict["a"]);
    for(const auto& value : decoded_list) {
        std::visit([](const auto& val) {
            std::cout << val << ", ";
        }, value);
    }*/    
    
    return nodes;    
}

std::vector<neroshop::Peer> neroshop::Node::get_peers() {
    std::vector<Peer> peers = {};
    
    std::string transaction_id = rpc::krpc::generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "get_peers";
    
    std::string get_peers_message = bencode::encode(query);
    
    
    return peers;
}

void neroshop::Node::announce_peer() {
    
    std::string transaction_id = rpc::krpc::generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "announce_peer";
    
    std::string announce_peer_message = bencode::encode(query);
}

void neroshop::Node::put(const std::string& key, const std::string& value) {
    
    std::string transaction_id = rpc::krpc::generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "put";
    //query["a"] = 
    
    std::string put_message = bencode::encode(query);
    
    
    data[key] = value;
}

std::string neroshop::Node::get(const std::string& key) {
    
    std::string transaction_id = rpc::krpc::generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "get";
    //query["a"] = 
    
    std::string get_message = bencode::encode(query);
    
    
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

void neroshop::Node::remove(const std::string& key) {
    
    /*std::string transaction_id = rpc::krpc::generate_transaction_id();
    std::string message_type = "q";
    std::string query_type = "";
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = 
    
    std::string _message = bencode::encode(query);*/
    
    
    data.erase(key);
}

// ?
/*std::vector<std::string> neroshop::Node::get_nodes() {
    struct sockaddr_in sin[100];
    struct sockaddr_in6 sin6[100];
    int num = 100, num6 = 100;

    if(dht_get_nodes(sin, &num, sin6, &num6) < 0) {
        perror("dht_get_nodes returned an error");
    }

    for (int i = 0; i < num; i++) {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sin[i].sin_addr), buf, INET_ADDRSTRLEN);
        printf("IPv4 node %d: %s:%d\n", i, buf, ntohs(sin[i].sin_port));
    }

    for (int i = 0; i < num6; i++) {
        char buf[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(sin6[i].sin6_addr), buf, INET6_ADDRSTRLEN);
        printf("IPv6 node %d: [%s]:%d\n", i, buf, ntohs(sin6[i].sin6_port));
    }
}*/

void neroshop::Node::loop() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    const size_t MAX_MESSAGE_SIZE = 4096; // adjust this value as needed
    char receive_buffer[MAX_MESSAGE_SIZE]; // $ sysctl net.ipv4.udp_rmem_min
    
    while (true) {
        std::memset(receive_buffer, 0, sizeof(receive_buffer)); // clear buffer
        
        int bytes_received = recvfrom(sockfd, receive_buffer, sizeof(receive_buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);

        if (bytes_received < 0) {//continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // recvfrom timed out - if the recvfrom function times out, the code will print a message to indicate that no data was received and continue waiting for new data.
                ////std::cerr << "Timed out waiting for data" << std::endl;
            } else {
                // some other error occurred
                std::cerr << "Error receiving data" << std::endl;
            }
        } else {
            // data received successfully
            std::cout << "Received " << bytes_received << " bytes from " << inet_ntoa(client_addr.sin_addr) << ":\033[0;36m" << ntohs(client_addr.sin_port) << "\033[0m" << std::endl;
        
            if (bytes_received <= std::string().max_size()) {
                std::string request_message(receive_buffer, bytes_received);
                std::string response_message = neroshop::rpc::krpc::process(request_message, this->id); // process request message
                //size_t pos = 0;
                //bencode_dict response_dict = std::get<bencode_dict>(bencode::decode(response_message, pos));
                //std::string transaction_id = std::get<std::string>(response_dict["t"]); // All KRPC messages contain a "t" (transaction ID) parameter
                
                // Store node that pinged you
                std::string ip_address = inet_ntoa(client_addr.sin_addr);
                if (ip_address.length() <= 15) { // check that the length of the IP address string is not longer than 15 characters (maximum length for an IPv4 address)
                    //auto node_that_pinged = std::make_unique<Node>(ip_address, ntohs(client_addr.sin_port), false);
                    //routing_table->add_node(std::move(node_that_pinged).get());
                } else {
                    std::cerr << "Received invalid IP address" << std::endl;
                }
                
                // Send the response message back to the client
                int bytes_sent = sendto(sockfd, response_message.c_str(), response_message.length(), 0, (struct sockaddr*)&client_addr, client_addr_len);
                if (bytes_sent < 0) {
                    std::cerr << "Error sending response message" << std::endl;
                } else {
                    // Response message sent successfully
                    std::cout << "Sent " << bytes_sent << " bytes to " << inet_ntoa(client_addr.sin_addr) << ":\033[0;36m" << ntohs(client_addr.sin_port) << "\033[0m" << std::endl;
                }
                
            } else {
                std::cerr << "Received message is too long to handle" << std::endl;
            }
        }
        
        std::memset(receive_buffer, 0, sizeof(receive_buffer)); // clear buffer
    }
}

std::string neroshop::Node::get_id() {
    return id;
}

std::string neroshop::Node::get_ip_address() const {
    const int ADDRSTRLEN = (storage.ss_family == AF_INET6) ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
    char ip_address[ADDRSTRLEN] = {0};

    if(storage.ss_family == AF_INET) {
        strcpy(ip_address, inet_ntoa(sockin.sin_addr));//std::string ip_address = inet_ntoa(sockin.sin_addr);
    } 
    if(storage.ss_family == AF_INET6) {
        inet_ntop(storage.ss_family, &(sockin6.sin6_addr), ip_address, ADDRSTRLEN);
    }

    return std::string(ip_address);
}

std::string neroshop::Node::get_local_ip_address() const {
    return get_ip_address();
}

std::string neroshop::Node::get_device_ip_address() const {
    std::future<std::string> result = std::async(std::launch::async, neroshop::get_device_ip_address);
    return result.get();
}

std::string neroshop::Node::get_public_ip_address() const {
    std::future<std::string> result = std::async(std::launch::async, neroshop::get_public_ip_address);
    return result.get();
}

int neroshop::Node::get_port() const {
    uint16_t port = -1;
    
    if(storage.ss_family == AF_INET) {
        port = ntohs(sockin.sin_port);
    } 
    if(storage.ss_family == AF_INET6) {
        port = ntohs(sockin6.sin6_port);
    }
    
    return port;
}

neroshop::RoutingTable * neroshop::Node::get_routing_table() const {
    return routing_table.get();
}

bool neroshop::Node::is_bootstrap_node() {
    return (bootstrap == true);
}

void neroshop::Node::set_bootstrap(bool bootstrap) {
    this->bootstrap = bootstrap;
}
/*int main() {
    // Create a new DHT instance and join the bootstrap node
    neroshop::Node dht_node("127.0.0.1", DEFAULT_PORT);
    neroshop::Peer bootstrap_peer = {"bootstrap.example.com", 5678}; // can be a randomly chosen existing node that provides the initial information to the new node that connects to it
    dht_node.join(bootstrap_peer);
    // Add some key-value pairs to the DHT
    dht_node.put("key1", "value1");
    dht_node.put("key2", "value2");
    // Retrieve a value from the DHT
    std::string value = dht_node.get("key1");
    std::cout << "Value: " << value << std::endl;
    // Remove a key-value pair from the DHT
    dht_node.remove("key2");
    // Find a node
    node.find_node("target_id");
    return 0;
} // g++ node.cpp ../../../crypto/sha3.cpp ../../../util/logger.cpp -I"../../../crypto/" -o node -lcrypto -lssl
*/
