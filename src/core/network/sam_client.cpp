#include "sam_client.hpp"

#include <sstream> // std::istringstream
#include <random> // std::random_device
#include <algorithm> // std::replace
#include <fstream>
#include <filesystem>
#include <thread>

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace neroshop {

//-----------------------------------------------------------------------------

SamClient::SamClient(SamSessionStyle style, const std::string& nickname) : session_socket(-1), server_addr({}), style(style), nickname(nickname), client_socket(-1), client_port(((style == SamSessionStyle::Datagram) || (style == SamSessionStyle::Raw)) ? SAM_DEFAULT_CLIENT_UDP : SAM_DEFAULT_CLIENT_TCP) {
    session_socket = ::socket(AF_INET, SOCK_STREAM, 0); // Always use TCP for the control socket
    if (session_socket < 0) {
        throw std::runtime_error("Error creating socket");
    }
    // Connect to SAM bridge TCP address + port
    memset(&server_addr, 0, sizeof(server_addr)); // Clear all bytes to 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SAM_DEFAULT_PORT_TCP); // Always TCP control socket (7656)
    inet_pton(AF_INET, SAM_DEFAULT_ADDRESS, &server_addr.sin_addr);
    if (::connect(session_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Error connecting to SAM bridge");
    }
    // Re-use server_addr for SAM bridge UDP port
    memset(&server_addr, 0, sizeof(server_addr)); // Clear all bytes to 0
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SAM_DEFAULT_PORT_UDP); // Change to UDP port (7655)
    inet_pton(AF_INET, SAM_DEFAULT_ADDRESS, &server_addr.sin_addr);

    //------------------------------------------------------------
    client_socket = ::socket(AF_INET, ((style == SamSessionStyle::Datagram) || (style == SamSessionStyle::Raw)) ? SOCK_DGRAM : SOCK_STREAM, 0);
    if(client_socket < 0) {
        throw std::runtime_error("Error creating socket");
    }
    
    sockaddr_in client_addr = {};
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(client_port);
    if ((style == SamSessionStyle::Datagram) || (style == SamSessionStyle::Raw)) {
        client_addr.sin_addr.s_addr = INADDR_ANY;
        if (::bind(client_socket, (sockaddr*)&client_addr, sizeof(client_addr)) < 0) {//throw std::runtime_error("Error binding socket to port " + std::to_string(SAM_DEFAULT_CLIENT_UDP));
            // Try ephemeral port
            client_addr.sin_port = htons(0); // 0 = ephemeral
            if (::bind(client_socket, (sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
                throw std::runtime_error("Error binding to ephemeral port");
            }
            // Get the actual port number used by the socket
            sockaddr_in actual_addr = {};
            socklen_t len = sizeof(actual_addr);
            if (getsockname(client_socket, (sockaddr*)&actual_addr, &len) < 0) {
                throw std::runtime_error("getsockname() failed to get ephemeral port");
            }
            this->client_port = ntohs(actual_addr.sin_port);
        }
    } else {
        inet_pton(AF_INET, SAM_DEFAULT_ADDRESS, &client_addr.sin_addr);
        if (::connect(client_socket, (sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
            throw std::runtime_error("Error connecting to " + std::string(SAM_DEFAULT_ADDRESS) + ":" + std::to_string(SAM_DEFAULT_CLIENT_TCP));
        }
    }
    
    // Set socket to non-blocking mode (allows for SIGINT)
    int flags = fcntl(client_socket, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set socket to non-blocking mode");
    }
}

//-----------------------------------------------------------------------------

SamClient::~SamClient() {
    session_close();
}

//-----------------------------------------------------------------------------

std::string SamClient::read_sam_reply(int sockfd) {
    std::string line;
    char ch;
    ssize_t n;
    while ((n = ::recv(sockfd, &ch, 1, 0)) == 1) { 
        if (ch == '\n') break;
        line += ch;
    }
    return line;
}

//-----------------------------------------------------------------------------

// Parse SAM reply into a map of key=value
std::map<std::string, std::string> SamClient::parse_sam_reply(const std::string& reply) {
    std::map<std::string, std::string> kv;
    std::istringstream iss(reply);
    std::string token;
    while (iss >> token) {
        auto pos = token.find('=');
        if (pos != std::string::npos) {
            kv[token.substr(0, pos)] = token.substr(pos + 1);
        } else {
            kv[token] = ""; // handles things like "HELLO" or "REPLY"
        }
    }
    return kv;
}

//-----------------------------------------------------------------------------

std::string SamClient::generate_nickname() {
    const std::string chars =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> length_dist(5, 9); // Random length (5-9 chars)
    std::uniform_int_distribution<> char_dist(0, chars.size() - 1);

    int length = length_dist(gen);
    std::string nickname;
    nickname.reserve(length);

    for (int i = 0; i < length; ++i) {
        nickname += chars[char_dist(gen)];
    }

    return nickname;
}

//-----------------------------------------------------------------------------

std::string SamClient::generate_session_id() {
    return generate_nickname();
}

//-----------------------------------------------------------------------------

// Helper: base64 decode with '-' and '~' mapped
static std::vector<uint8_t> base64_i2p_decode(const std::string& input) {
    std::string fixed_input = input;
    std::replace(fixed_input.begin(), fixed_input.end(), '-', '+');
    std::replace(fixed_input.begin(), fixed_input.end(), '~', '/');

    BIO* bio = BIO_new_mem_buf(fixed_input.data(), fixed_input.size());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    std::vector<uint8_t> output(1024); // 1024 is enough
    int len = BIO_read(bio, output.data(), output.size());
    if (len <= 0) {
        BIO_free_all(bio);
        throw std::runtime_error("Failed to decode base64");
    }
    output.resize(len);
    BIO_free_all(bio);
    return output;
}

// Helper: base32 encode SHA256 of decoded destination
static std::string base32_i2p_encode(const uint8_t* data, size_t length) {
    static const char* alphabet = "abcdefghijklmnopqrstuvwxyz234567";
    std::string result;
    int bits = 0, value = 0;

    for (size_t i = 0; i < length; ++i) {
        value = (value << 8) | data[i];
        bits += 8;
        while (bits >= 5) {
            result += alphabet[(value >> (bits - 5)) & 31];
            bits -= 5;
        }
    }
    if (bits > 0) {
        result += alphabet[(value << (5 - bits)) & 31];
    }
    return result;
}

// Final function: destination -> .b32.i2p
//-----------------------------------------------------------------------------

std::string SamClient::to_i2p_address(const std::string& pubkey) {
    auto decoded = base64_i2p_decode(pubkey);

    // Hash entire decoded destination with SHA-256
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(decoded.data(), decoded.size(), hash);

    std::string b32 = base32_i2p_encode(hash, SHA256_DIGEST_LENGTH);
    return b32 + ".b32.i2p";
}

//-----------------------------------------------------------------------------

bool save_key(const std::string& private_key, const std::string& filename) {
    if(private_key.empty()) {
        std::cerr << "\033[0;91m[!] Private key is empty" << "\033[0m\n";
        return false;
    }

    if (std::filesystem::exists(filename)) {
        std::cerr << "\033[0;91m[!] File already exists: " << filename << "\033[0m\n";
        return false;
    }

    std::ofstream out(filename, std::ios::out);
    if (!out.is_open()) {
        std::cerr << "\033[0;91m[!] Failed to open file: " << filename << "\033[0m\n";
        return false;
    }
    
    if (out) {
        out << private_key;  // no "PRIV=" prefix
        out.close();
    }
    return true;
}

std::string load_key(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "[load_key] File does not exist: " << filename << "\n";
        return "";
    }

    std::ifstream in(filename, std::ios::in);
    if (!in) {
        std::cerr << "[load_key] Failed to open file: " << filename << "\n";
        return "";
    }

    std::string private_key((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());

    in.close();

    // Optional: Trim whitespace or validate base64 structure here
    if (private_key.empty()) {
        std::cerr << "[load_key] Loaded key is empty.\n";
        return "";
    }

    return private_key;
}

//-----------------------------------------------------------------------------

SamReply SamClient::send_sam_command(const std::string& command, int sockfd) {
    if (sockfd < 0) { throw std::runtime_error("socket is closed"); }
    ssize_t sent_bytes = -1; // invalid until proven otherwise
    
    sent_bytes = ::send(sockfd, command.data(), command.size(), 0);
    if (sent_bytes < 0) {
        perror("send failed");
        if (errno == EBADF) {
            return SamReply{ SamResultType::SocketClosed, "" }; // Socket is invalid (closed)
        } else if (errno == ENOTCONN) {
            return SamReply{ SamResultType::SocketClosed, "" }; // Socket is not connected
        }
    } else if (sent_bytes == 0) {
        return SamReply{ SamResultType::NoResult, "" }; // No data sent, but socket is not necessarily closed
    }
    std::cout << "\033[93m" << command << "\033[0m\n";
    
    std::string reply = read_sam_reply(sockfd);
    std::cout << reply << "\n";

    // Return SamReply with both result and raw reply (not all replies have a "RESULT=")
    return SamReply{ get_result(reply), reply };
}

//-----------------------------------------------------------------------------

SamReply SamClient::hello(int sockfd) {
    std::string command = "HELLO VERSION MIN=3.1 MAX=3.3\n";
    auto reply = send_sam_command(command, sockfd);
    
    version = get_value(reply.raw_reply, "VERSION");
    ////std::cout << "Selected SAM version: " << version << "\n";
    
    return reply;
}

//-----------------------------------------------------------------------------

void SamClient::session_prepare() {
    if (session_socket < 0) { throw std::runtime_error("socket is closed"); }
    
    private_key = load_key(SAM_DEFAULT_PRIVKEY_PATH);
    if (private_key.empty()) {
        // Call DEST GENERATE if a persistent identity does not yet exist
        std::string command = "DEST GENERATE SIGNATURE_TYPE=7\n";
        auto reply = send_sam_command(command, session_socket);
    
        public_key = get_value(reply.raw_reply, "PUB");
        private_key = get_value(reply.raw_reply, "PRIV");
        ////std::cout << "Public Key: " << public_key << "\n\n";
        ////std::cout << "Private Key: " << private_key << "\n\n";
        
        // Save PRIV (the private key) securely, e.g., to a file
        if(!save_key(private_key, SAM_DEFAULT_PRIVKEY_PATH)) {
            throw std::runtime_error(std::string("File saving failed: '") + SAM_DEFAULT_PRIVKEY_PATH + "'\n");
        }
        std::cout << "\033[0;92m[+] Saved $privkey to file\033[0m\n";
    } else {
        std::cout << "\033[0;92m[+] Restored $privkey from file\033[0m\n";
    }
    
    // Extract public key from private key
    if(public_key.empty()) {
        std::string command = "NAMING LOOKUP NAME=" + private_key + "\n";
        auto reply = send_sam_command(command, session_socket);
        if(reply.result != SamResultType::Ok) {
            std::cerr << "\033[91m" << reply.raw_reply << "\033[0m\n"; 
            session_close();
            exit(1);
        }
        public_key = get_value(reply.raw_reply, "VALUE");
        ////std::cout << "Public key (extracted from private key): " << public_key << "\n";
    }
    
    // Convert public key to b32.i2p address
    i2p_address = to_i2p_address(public_key);
    ////std::cout << "Your I2P address: " << i2p_address << "\n\n";
}

//-----------------------------------------------------------------------------

void SamClient::session_create() {
    if (session_socket < 0) { throw std::runtime_error("socket is closed"); }
    
    std::string style_ = get_session_style_as_string();
    std::string command = "SESSION CREATE STYLE=" + style_ + " ID=" + nickname + " DESTINATION=" + private_key + 
        (((style == SamSessionStyle::Datagram) || (style == SamSessionStyle::Raw)) ? " PORT=" + std::to_string(client_port) + " i2cp.leaseSetEncType=4,0\n" : " i2cp.leaseSetEncType=4,0\n"); // PORT=$port is required for DATAGRAM* and RAW, invalid for STREAM
    auto reply = send_sam_command(command, session_socket);
    if(reply.result != SamResultType::Ok) {
        std::cerr << "\033[91m" << reply.raw_reply << "\033[0m\n";
        session_close();
        exit(1);
    }
}

//-----------------------------------------------------------------------------

void SamClient::session_close() {
    if(session_socket >= 0) {
        ::shutdown(session_socket, SHUT_RDWR); // Tell the peer "we're done" // SHUT_RDWR means disallow sending and receiving
        ::close(session_socket);               // Actually release the socket
        session_socket = -1;
        #ifdef NEROSHOP_DEBUG
        std::cout << "SAM session (" << get_nickname() << ") closed\n";//log_debug("SAM: Session {}{}{} closed", color_yellow, get_nickname(), color_reset);
        #endif
    }
}

//-----------------------------------------------------------------------------

void SamClient::stream_connect(const std::string& destination, bool silent) {
    if(style != SamSessionStyle::Stream) {
        throw std::runtime_error("not in Stream session mode");
    }
    
    std::string command = "STREAM CONNECT ID=" + nickname + " DESTINATION=" + destination + 
        ((silent == true) ? " SILENT=true" : " SILENT=false") + "\n";
    auto reply = send_sam_command(command, client_socket); // This time we use another socket, not the session socket (not sure if we connect to SAM TCP Port or the Client's own TCP Port?)
    if(reply.result == SamResultType::NoResult && reply.raw_reply.empty()) {
        std::cerr << "\033[91mNo reply received. Did you forget to handshake?\033[0m\n";
        return;
    }
    if(reply.result != SamResultType::Ok) {
        std::cerr << "\033[91m" << reply.raw_reply << "\033[0m\n";
    }
}

//-----------------------------------------------------------------------------

void SamClient::stream_accept(bool silent) {
    if(style != SamSessionStyle::Stream) {
        throw std::runtime_error("not in Stream session mode");
    }
    
    std::string command = "STREAM ACCEPT ID=" + nickname + 
        ((silent == true) ? " SILENT=true" : " SILENT=false") + "\n";
    auto reply = send_sam_command(command, client_socket); // This time we use another socket, not the session socket (not sure if we connect to SAM TCP Port or the Client's own TCP Port?)
    if(reply.result == SamResultType::NoResult && reply.raw_reply.empty()) {
        std::cerr << "\033[91mNo reply received. Did you forget to handshake?\033[0m\n";
        return;
    }
    if(reply.result != SamResultType::Ok) {
        std::cerr << "\033[91m" << reply.raw_reply << "\033[0m\n";
    }
}

//-----------------------------------------------------------------------------

void SamClient::stream_forward(uint16_t port, std::string host, bool silent) {} // Port is mandatory - use default client TCP port (7666) not SamBridge port (7656)!! // not really used in most apps // whether SILENT is true or false, the SAM bridge always answers with a STREAM STATUS message.

//-----------------------------------------------------------------------------

/*
 After establishing a SAM session with STYLE=DATAGRAM or STYLE=RAW, the client can send repliable or raw datagrams through SAM's UDP port (7655 by default).

The first line of a datagram sent through this port must be in the following format. This is all on one line (space separated), shown on multiple lines for clarity:

    3.0                                  # As of SAM 3.2, any "3.x" is allowed. Prior to that, "3.0" is required.
    $nickname
    $destination
*/
void SamClient::send_raw_datagram(const std::string& destination, const uint8_t* data, size_t size) {
    if (style != SamSessionStyle::Datagram) {
        throw std::runtime_error("not in DATAGRAM session mode");
    }

    std::string header = "3.0 " + nickname + " " + destination + "\n";

    std::vector<uint8_t> datagram;
    datagram.reserve(header.size() + size);
    datagram.insert(datagram.end(), header.begin(), header.end());
    datagram.insert(datagram.end(), data, data + size);

    ssize_t sent_bytes = ::sendto(client_socket, datagram.data(), datagram.size(), 0,
                                  (sockaddr*)&server_addr, sizeof(server_addr));
    if (sent_bytes < 0) {
        throw std::runtime_error("Failed to send datagram to SAM bridge");
    }
}

//-----------------------------------------------------------------------------

void SamClient::datagram_send(const std::string& destination, const std::string& payload) {
    send_raw_datagram(destination, reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
}

//-----------------------------------------------------------------------------

void SamClient::datagram_send(const std::string& destination, const std::vector<uint8_t>& payload) {
    send_raw_datagram(destination, payload.data(), payload.size());
}

//-----------------------------------------------------------------------------

std::vector<uint8_t> SamClient::datagram_receive() {
    if (style != SamSessionStyle::Datagram) {
        throw std::runtime_error("not in DATAGRAM session mode");
    }

    char buffer[SAM_BUFSIZE] = {0};
    sockaddr_in sender_addr = {};
    socklen_t addr_len = sizeof(sender_addr);
    ssize_t received_bytes = ::recvfrom(client_socket, buffer, SAM_BUFSIZE - 1, 0, (sockaddr*)&sender_addr, &addr_len);

    if (received_bytes < 0) {
        throw std::runtime_error("Failed to receive datagram");
    }

    // Optional: parse the payload (depends on SAM protocol specifics)
    std::string message(buffer, received_bytes);
    std::cout << "Received datagram: " << message << " (" << received_bytes << " bytes): " << std::endl;

    // You could parse the SAM message format here if needed
    // e.g., extract source/destination/payload, etc.
    
    return std::vector<uint8_t>(buffer, buffer + received_bytes);
}
/*
std::vector<uint8_t> data = sam_client->datagram_receive();
std::string maybe_string(data.begin(), data.end());
*/

//-----------------------------------------------------------------------------

SamDatagram SamClient::datagram_parse(const std::string& message) {
    SamDatagram datagram;

    auto header_end = message.find('\n');
    if (header_end == std::string::npos) {
        throw std::runtime_error("Malformed SAM datagram: missing newline");
    }

    datagram.header = message.substr(0, header_end);
    
    // Parse the payload
    // If payload were a string: datagram.payload = message.substr(header_end + 1);
    datagram.payload = std::vector<uint8_t>(message.begin() + header_end + 1, message.end());

    // Extract destination (everything before " FROM_PORT=")
    auto from_pos = datagram.header.find(" FROM_PORT=");
    if (from_pos != std::string::npos) {
        datagram.destination = datagram.header.substr(0, from_pos);
    } else {
        throw std::runtime_error("Malformed SAM header: missing FROM_PORT");
    }

    return datagram;
}

//-----------------------------------------------------------------------------

SamDatagram SamClient::datagram_parse(const std::vector<uint8_t>& buffer) {
    return datagram_parse(std::string(buffer.begin(), buffer.end()));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

SamSessionStyle SamClient::get_session_style() const {
    return style;
}

//-----------------------------------------------------------------------------

std::string SamClient::get_session_style_as_string() const {
    switch(style) {
        case SamSessionStyle::Stream:
            return "STREAM";
        case SamSessionStyle::Datagram:
            return "DATAGRAM";
        case SamSessionStyle::Raw:
            return "RAW";
        default:
            return "";
    }
}

//-----------------------------------------------------------------------------

std::string SamClient::get_session_id() const {
    return get_nickname();
}

//-----------------------------------------------------------------------------

std::string SamClient::get_nickname() const {
    return nickname; // ID=$nickname
}

//-----------------------------------------------------------------------------

std::string SamClient::get_private_key() const {
    return private_key; // DESTINATION=$privkey
}

//-----------------------------------------------------------------------------

std::string SamClient::get_public_key() const {
    return public_key;
}

//-----------------------------------------------------------------------------
    
std::string SamClient::get_i2p_address() const {
    return i2p_address;
}

//-----------------------------------------------------------------------------

uint16_t SamClient::get_port() const {
    return client_port;
}

//-----------------------------------------------------------------------------

std::string SamClient::get_sam_version() {
    return version;
}

//-----------------------------------------------------------------------------

std::string SamClient::get_sam_minimum_version() {
    return "3.1"; // MAX version for i2pd
}

//-----------------------------------------------------------------------------

std::string SamClient::get_sam_maximum_version() {
    return "3.3";
}
    
//-----------------------------------------------------------------------------

int SamClient::get_session_socket() const {
    return session_socket;
}

//-----------------------------------------------------------------------------

int SamClient::get_client_socket() const {
    return client_socket;
}

//-----------------------------------------------------------------------------

int SamClient::get_socket() const {
    return get_client_socket();
}

//-----------------------------------------------------------------------------

std::string SamClient::get_value(const std::string& reply, const std::string& key) {
    auto fields = parse_sam_reply(reply);
    if (fields.count(key) == 0) {
        throw std::runtime_error(std::string("key '" + key + "' not found"));
    }
    
    return fields[key];
}

//-----------------------------------------------------------------------------

SamResultType SamClient::get_result(const std::string& reply) {
    auto fields = parse_sam_reply(reply);
    if (fields.count("RESULT") == 0) {
        return SamResultType::NoResult; // instead of throwing an error
    }
    if (fields["RESULT"] == "OK") { return SamResultType::Ok; }
    if (fields["RESULT"] == "NOVERSION") { return SamResultType::NoVersion; }
    if (fields["RESULT"] == "I2P_ERROR") { return SamResultType::I2PError; }
    if (fields["RESULT"] == "DUPLICATED_ID") { return SamResultType::DuplicatedId; }
    if (fields["RESULT"] == "DUPLICATED_DEST") { return SamResultType::DuplicatedDest; }
    if (fields["RESULT"] == "INVALID_KEY") { return SamResultType::InvalidKey; }
    if (fields["RESULT"] == "INVALID_ID") { return SamResultType::InvalidId; }
    if (fields["RESULT"] == "TIMEOUT") { return SamResultType::Timeout; }
    if (fields["RESULT"] == "CANT_REACH_PEER") { return SamResultType::CantReachPeer; }
    if (fields["RESULT"] == "KEY_NOT_FOUND") { return SamResultType::KeyNotFound; }
    if (fields["RESULT"] == "PEER_NOT_FOUND") { return SamResultType::PeerNotFound; }
    if (fields["RESULT"] == "LEASESET_NOT_FOUND") { return SamResultType::LeaseSetNotFound; }
    if (fields["RESULT"] == "ALREADY_ACCEPTING") { return SamResultType::AlreadyAccepting; }
    //if (fields["RESULT"] == "") { return SamResultType::; }
    
    return SamResultType::UnknownResult;
}

//-----------------------------------------------------------------------------

std::string SamClient::get_result_as_string(SamResultType result) {
    switch (result) {
        case SamResultType::Ok:
            return "OK";
        case SamResultType::NoResult: // custom
            return "NORESULT";
        case SamResultType::UnknownResult: // custom
            return "UNKNOWN_RESULT";
        case SamResultType::SocketClosed: // custom
            return "SOCKET_CLOSED";
        case SamResultType::NoVersion:
            return "NOVERSION";
        case SamResultType::I2PError:
            return "I2P_ERROR";
        case SamResultType::DuplicatedId:
            return "DUPLICATED_ID";
        case SamResultType::DuplicatedDest:
            return "DUPLICATED_DEST";
        case SamResultType::InvalidKey:
            return "INVALID_KEY";
        case SamResultType::InvalidId:
            return "INVALID_ID";
        case SamResultType::Timeout:
            return "TIMEOUT";
        case SamResultType::CantReachPeer:
            return "CANT_REACH_PEER";
        case SamResultType::KeyNotFound:
            return "KEY_NOT_FOUND";
        case SamResultType::PeerNotFound:
            return "PEER_NOT_FOUND";
        case SamResultType::LeaseSetNotFound:
            return "LEASESET_NOT_FOUND";
        case SamResultType::AlreadyAccepting:
            return "ALREADY_ACCEPTING";
        /*case SamResultType:::
            return "";*/
        default:
            return "UNKNOWN_RESULT";
    }
}

//-----------------------------------------------------------------------------

}
// g++ -DOPENSSL_SUPPRESS_DEPRECATED sam_client.cpp -lcrypto -lssl -lz -lpthread -o sam_client
