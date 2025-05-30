#pragma once

#ifndef SAM_CLIENT_HPP_NEROSHOP
#define SAM_CLIENT_HPP_NEROSHOP

#include <cstddef> // std::size_t
#include <cstdint> // uint*_t

inline constexpr std::size_t SAM_BUFSIZE              = 8192;
inline constexpr const char* SAM_DEFAULT_ADDRESS      = "127.0.0.1";
inline constexpr std::uint16_t SAM_DEFAULT_PORT_TCP   = 7656;
inline constexpr std::uint16_t SAM_DEFAULT_PORT_UDP   = 7655;
inline constexpr std::uint16_t SAM_DEFAULT_CLIENT_TCP = 7666;
inline constexpr std::uint16_t SAM_DEFAULT_CLIENT_UDP = 7667;
inline constexpr const char* SAM_DEFAULT_PRIVKEY_PATH = "i2p.key";

inline constexpr const char* SAM_NAME_INBOUND_QUANTITY          = "inbound.quantity";
inline constexpr int SAM_DEFAULT_INBOUND_QUANTITY               = 3; // Three tunnels is default now
inline constexpr const char* SAM_NAME_INBOUND_LENGTH            = "inbound.length";
inline constexpr int SAM_DEFAULT_INBOUND_LENGTH                 = 3; // Three jumps is default now
inline constexpr const char* SAM_NAME_INBOUND_LENGTHVARIANCE    = "inbound.lengthVariance";
inline constexpr int SAM_DEFAULT_INBOUND_LENGTHVARIANCE         = 0;
inline constexpr const char* SAM_NAME_INBOUND_BACKUPQUANTITY    = "inbound.backupQuantity";
inline constexpr int SAM_DEFAULT_INBOUND_BACKUPQUANTITY         = 1; // One backup tunnel
inline constexpr const char* SAM_NAME_INBOUND_ALLOWZEROHOP      = "inbound.allowZeroHop";
inline constexpr bool SAM_DEFAULT_INBOUND_ALLOWZEROHOP          = true;
inline constexpr const char* SAM_NAME_INBOUND_IPRESTRICTION     = "inbound.IPRestriction";
inline constexpr int SAM_DEFAULT_INBOUND_IPRESTRICTION          = 2;
inline constexpr const char* SAM_NAME_OUTBOUND_QUANTITY         = "outbound.quantity";
inline constexpr int SAM_DEFAULT_OUTBOUND_QUANTITY              = 3;
inline constexpr const char* SAM_NAME_OUTBOUND_LENGTH           = "outbound.length";
inline constexpr int SAM_DEFAULT_OUTBOUND_LENGTH                = 3;
inline constexpr const char* SAM_NAME_OUTBOUND_LENGTHVARIANCE   = "outbound.lengthVariance";
inline constexpr int SAM_DEFAULT_OUTBOUND_LENGTHVARIANCE        = 0;
inline constexpr const char* SAM_NAME_OUTBOUND_BACKUPQUANTITY   = "outbound.backupQuantity";
inline constexpr int SAM_DEFAULT_OUTBOUND_BACKUPQUANTITY        = 1;
inline constexpr const char* SAM_NAME_OUTBOUND_ALLOWZEROHOP     = "outbound.allowZeroHop";
inline constexpr bool SAM_DEFAULT_OUTBOUND_ALLOWZEROHOP         = true;
inline constexpr const char* SAM_NAME_OUTBOUND_IPRESTRICTION    =  "outbound.IPRestriction";
inline constexpr int SAM_DEFAULT_OUTBOUND_IPRESTRICTION         = 2;
inline constexpr const char* SAM_NAME_OUTBOUND_PRIORITY         = "outbound.priority";
////#define SAM_DEFAULT_OUTBOUND_PRIORITY
inline constexpr const char* SAM_NAME_I2CP_LEASESET_ENC_TYPE    = "i2cp.leaseSetEncType";
inline constexpr const char* SAM_DEFAULT_I2CP_LEASESET_ENC_TYPE = "0,4";

#if defined(_WIN32)
#include <winsock2.h> // core header for Winsock2
#include <ws2tcpip.h> // header for TCP/IP protocols
#include <iphlpapi.h> // header for IP helper functions
#endif

#if defined(__gnu_linux__)
#include <sys/types.h>      // size_t, ssize_t, socklen_t, etc.
#include <sys/socket.h>     // socket(), connect(), bind(), accept(), etc.
#include <netinet/in.h>     // sockaddr_in, htons(), htonl(), IPPROTO_TCP, etc.
#include <arpa/inet.h>      // inet_pton(), inet_ntop(), inet_addr(), etc.
#include <unistd.h>         // close(), read(), write(), etc.
#include <fcntl.h>          // fcntl(), O_NONBLOCK, etc.
#include <string.h>         // memset(), memcpy(), etc.
#include <errno.h>          // errno, perror(), strerror()
//#include <poll.h>           // poll()
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace neroshop {

// SAM minumum version (fixed): 3.1

// Signature type (fixed): SIGNATURE_TYPE=7  (Ed25519)

// Encryption type (fixed): i2cp.leaseSetEncType=4,0 (for ECIES-X25519 and ElGamal, if compatibility is required)

enum class SamSessionStyle { Stream, Datagram, Raw, Datagram2, Datagram3 };

enum class SamResultType { Ok, NoResult, UnknownResult, SocketClosed, NoVersion, I2PError, DuplicatedId, DuplicatedDest, InvalidKey, InvalidId, Timeout, CantReachPeer, KeyNotFound, PeerNotFound, LeaseSetNotFound, AlreadyAccepting }; // If RESULT=I2P_ERROR then a MESSAGE="$message" will often follow

struct SamReply {
    SamResultType result;
    std::string raw_reply;
};

struct SamDestination {
    std::string pubkey;
    std::string privkey;
};

struct SamDatagram {
    std::vector<uint8_t> payload; // raw msgpack payload
    std::string destination; // base64 public destination key
    std::string header;  // full SAM header line
    // TODO: FROM_PORT=, TO_PORT=, and SIZE
};

class SamClient {
public:
    SamClient() = delete; // Prevent default construction
    SamClient(SamSessionStyle style, const std::string& nickname = generate_nickname());
    ~SamClient();

    SamReply hello(int sockfd); // handshake
    void session_prepare(); // restores or generates pubkey and privkey
    void session_create();
    void session_close();
    // Stream
    void stream_connect(const std::string& destination, bool silent = false);
    void stream_accept(bool silent = false);
    void stream_forward(uint16_t port, std::string host = "127.0.0.1", bool silent = false); // use default client TCP port (7666) not SamBridge port (7656)!! // not really used in most apps // whether SILENT is true or false, the SAM bridge always answers with a STREAM STATUS message. // the port is mandatory
    // Datagram
    void datagram_send(const std::string& destination, const std::string& payload);
    void datagram_send(const std::string& destination, const std::vector<uint8_t>& payload);
    std::vector<uint8_t> datagram_receive(); // runs in a loop (blocking)
    static SamDatagram datagram_parse(const std::string& message);
    static SamDatagram datagram_parse(const std::vector<uint8_t>& buffer);
    
    //std::string naming_lookup();
    //SamDestination dest_generate();
    
    // getters
    int get_session_socket() const;
    int get_client_socket() const;
    int get_socket() const; // same as get_client_socket()
    SamSessionStyle get_session_style() const;
    std::string get_session_style_as_string() const;
    std::string get_session_id() const; // same as get_nickname()
    std::string get_nickname() const;
    //std::string get_options() const;
    std::string get_private_key() const;
    std::string get_public_key() const;
    std::string get_i2p_address() const;
    uint16_t get_port() const;
    std::string get_sam_version();
    std::string get_sam_minimum_version();
    std::string get_sam_maximum_version();
    // Reply parser
    static std::string get_value(const std::string& reply, const std::string& key); // get the value of a key in a reply (e.g MESSAGE="$message", DESTINATION=$privkey)
    static SamResultType get_result(const std::string& reply); // get the result of a reply (e.g DUPLICATED_ID)
    static std::string get_result_as_string(SamResultType result); // converts the value of RESULT= to its string form
    // Conversions
    static std::string to_i2p_address(const std::string& public_key);
    // Friends
    friend class Node;
private:
    static SamReply send_sam_command(const std::string& command, int sockfd);
    static std::string read_sam_reply(int sockfd);
    static std::map<std::string, std::string> parse_sam_reply(const std::string& reply);
    static std::string generate_session_id();
    static std::string generate_nickname();
    void send_raw_datagram(const std::string& destination, const uint8_t* data, size_t size);
    sockaddr_in server_addr;
    std::string version; // the i2p router chooses the version
    SamSessionStyle style; // must come before client_port (is initialized inline in the constructor initialization list based on the style)
    std::string nickname; // session ID (or nickname)
    std::string private_key; // destination
    std::string public_key; // base64 public destination key
    std::string i2p_address;
    int session_socket; // for SAM commands like SESSION CREATE (control socket)
    int client_socket; // for data transmission // TODO: split client_socket into two separate sockets for TCP and UDP
    uint16_t client_port; // both client TCP and UDP sockets are able to share the same port without conflict
};

}
#endif
