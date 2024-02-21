/**
 * Copyright (c) 2019-2022 polistern
 * Copyright (c) 2017 The I2P Project
 * Copyright (c) 2013-2015 The Anoncoin Core developers
 * Copyright (c) 2012-2013 giv
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or http://www.opensource.org/licenses/mit-license.php.
 *
 * See full documentation about SAM at http://www.i2p2.i2p/samv3.html
 */

#ifndef I2PSAM_H__
#define I2PSAM_H__

#define SAM_BUFSIZE                 8192
#define SAM_DEFAULT_ADDRESS         "127.0.0.1"
#define SAM_DEFAULT_PORT_TCP        7656
#define SAM_DEFAULT_PORT_UDP        7655
#define SAM_DEFAULT_CLIENT_TCP      7666
#define SAM_DEFAULT_CLIENT_UDP      7667
#define SAM_GENERATE_MY_DESTINATION "TRANSIENT"
#define SAM_MY_NAME                 "ME"
#define SAM_DEFAULT_I2P_OPTIONS     SAM_NAME_I2CP_LEASESET_ENC_TYPE "=" SAM_DEFAULT_I2CP_LEASESET_ENC_TYPE // i2cp.leaseSetEncType=0,4
#define SAM_SIGNATURE_TYPE          "EdDSA_SHA512_Ed25519"

#define SAM_NAME_INBOUND_QUANTITY           "inbound.quantity"
#define SAM_DEFAULT_INBOUND_QUANTITY        3 // Three tunnels is default now
#define SAM_NAME_INBOUND_LENGTH             "inbound.length"
#define SAM_DEFAULT_INBOUND_LENGTH          3 // Three jumps is default now
#define SAM_NAME_INBOUND_LENGTHVARIANCE     "inbound.lengthVariance"
#define SAM_DEFAULT_INBOUND_LENGTHVARIANCE  0
#define SAM_NAME_INBOUND_BACKUPQUANTITY     "inbound.backupQuantity"
#define SAM_DEFAULT_INBOUND_BACKUPQUANTITY  1 // One backup tunnel
#define SAM_NAME_INBOUND_ALLOWZEROHOP       "inbound.allowZeroHop"
#define SAM_DEFAULT_INBOUND_ALLOWZEROHOP    true
#define SAM_NAME_INBOUND_IPRESTRICTION      "inbound.IPRestriction"
#define SAM_DEFAULT_INBOUND_IPRESTRICTION   2
#define SAM_NAME_OUTBOUND_QUANTITY          "outbound.quantity"
#define SAM_DEFAULT_OUTBOUND_QUANTITY       3
#define SAM_NAME_OUTBOUND_LENGTH            "outbound.length"
#define SAM_DEFAULT_OUTBOUND_LENGTH         3
#define SAM_NAME_OUTBOUND_LENGTHVARIANCE    "outbound.lengthVariance"
#define SAM_DEFAULT_OUTBOUND_LENGTHVARIANCE 0
#define SAM_NAME_OUTBOUND_BACKUPQUANTITY    "outbound.backupQuantity"
#define SAM_DEFAULT_OUTBOUND_BACKUPQUANTITY 1
#define SAM_NAME_OUTBOUND_ALLOWZEROHOP      "outbound.allowZeroHop"
#define SAM_DEFAULT_OUTBOUND_ALLOWZEROHOP   true
#define SAM_NAME_OUTBOUND_IPRESTRICTION     "outbound.IPRestriction"
#define SAM_DEFAULT_OUTBOUND_IPRESTRICTION  2
#define SAM_NAME_OUTBOUND_PRIORITY          "outbound.priority"
#define SAM_DEFAULT_OUTBOUND_PRIORITY
#define SAM_NAME_I2CP_LEASESET_ENC_TYPE     "i2cp.leaseSetEncType"
#define SAM_DEFAULT_I2CP_LEASESET_ENC_TYPE  "0,4"

// Define this, if you want more of the original standard output diagnostics
//#define DEBUG_ON_STDOUT

#ifdef __cplusplus // __cplusplus
#include "compat.h"

#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <stdio.h>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace SAM
{

typedef u_int SOCKET;

static void print_error(const std::string &err)
{
#ifdef DEBUG_ON_STDOUT
#ifdef WIN32
  std::cout << err << "(" << WSAGetLastError() << ")" << std::endl;
#else // WIN32
  std::cout << err << "(" << errno << ")" << std::endl;
#endif // WIN32
#endif // DEBUG_ON_STDOUT
}

class Message
{
 public:
  enum SessionStyle
    {
      sssStream,
      sssDatagram,
      sssRaw
    };

  enum eStatus
    {
      OK,
      EMPTY_ANSWER,
      CLOSED_SOCKET,
      CANNOT_PARSE_ERROR,
      /** The destination is already in use
       *
       * ->  SESSION CREATE ...
       * <-  SESSION STATUS RESULT=DUPLICATED_DEST
       */
      DUPLICATED_DEST,
      /**
       * The nickname is already associated with a session
       *
       * ->  SESSION CREATE ...
       * <-  SESSION STATUS RESULT=DUPLICATED_ID
       */
      DUPLICATED_ID,
      /**
       * A generic I2P error (e.g. I2CP disconnection, etc.)
       *
       * ->  HELLO VERSION ...
       * <-  HELLO REPLY RESULT=I2P_ERROR MESSAGE={$message}
       *
       * ->  SESSION CREATE ...
       * <-  SESSION STATUS RESULT=I2P_ERROR MESSAGE={$message}
       *
       * ->  STREAM CONNECT ...
       * <-  STREAM STATUS RESULT=I2P_ERROR MESSAGE={$message}
       *
       * ->  STREAM ACCEPT ...
       * <-  STREAM STATUS RESULT=I2P_ERROR MESSAGE={$message}
       *
       * ->  STREAM FORWARD ...
       * <-  STREAM STATUS RESULT=I2P_ERROR MESSAGE={$message}
       *
       * ->  NAMING LOOKUP ...
       * <-  NAMING REPLY RESULT=INVALID_KEY NAME={$name} MESSAGE={$message}
       */
      I2P_ERROR,
      /**
       * Stream session ID doesn't exist
       *
       * ->  STREAM CONNECT ...
       * <-  STREAM STATUS RESULT=INVALID_ID MESSAGE={$message}
       *
       * ->  STREAM ACCEPT ...
       * <-  STREAM STATUS RESULT=INVALID_ID MESSAGE={$message}
       *
       * ->  STREAM FORWARD ...
       * <-  STREAM STATUS RESULT=INVALID_ID MESSAGE={$message}
       */
      INVALID_ID,
      /**
       * The destination is not a valid private destination key
       *
       * ->  SESSION CREATE ...
       * <-  SESSION STATUS RESULT=INVALID_KEY MESSAGE={$message}
       *
       * ->  STREAM CONNECT ...
       * <-  STREAM STATUS RESULT=INVALID_KEY MESSAGE={$message}
       *
       * ->  NAMING LOOKUP ...
       * <-  NAMING REPLY RESULT=INVALID_KEY NAME={$name} MESSAGE={$message}
       */
      INVALID_KEY,
      /**
       * The peer exists, but cannot be reached
       *
       * ->  STREAM CONNECT ...
       * <-  STREAM STATUS RESULT=CANT_REACH_PEER MESSAGE={$message}
       */
      CANT_REACH_PEER,
      /**
       * Timeout while waiting for an event (e.g. peer answer)
       *
       * ->  STREAM CONNECT ...
       * <-  STREAM STATUS RESULT=TIMEOUT MESSAGE={$message}
       */
      TIMEOUT,
      /**
       * The SAM bridge cannot find a suitable version
       *
       * ->  HELLO VERSION ...
       * <-  HELLO REPLY RESULT=NOVERSION MESSAGE={$message}
       */
      NOVERSION,
      /**
       * The naming system can't resolve the given name
       *
       * ->  NAMING LOOKUP ...
       * <-  NAMING REPLY RESULT=INVALID_KEY NAME={$name} MESSAGE={$message}
       */
      KEY_NOT_FOUND,
      /**
       * The peer cannot be found on the network
       *
       * ??
       */
      PEER_NOT_FOUND,
      /**
       * ??
       *
       * ->  STREAM ACCEPT
       * <-  STREAM STATUS RESULT=ALREADY_ACCEPTING
       */
      ALREADY_ACCEPTING,
      /**
       * ??
       */
      FAILED,
      /**
       * ??
       */
      CLOSED
    };

  template<class T>
  struct Answer
  {
    const Message::eStatus status;
    T value;

    Answer(Message::eStatus status, const T &value)
      : status(status), value(value) {}
    explicit Answer(Message::eStatus status) : status(status), value() {}
  };

  static std::string hello(const std::string &minVer,
                           const std::string &maxVer);
  // Stream session
  static std::string
  sessionCreate(SessionStyle style, const std::string &sessionID,
                const std::string &nickname,
                const std::string &destination = SAM_GENERATE_MY_DESTINATION,
                const std::string &options = "",
                const std::string &signatureType = SAM_SIGNATURE_TYPE);
  static std::string streamAccept(const std::string &sessionID,
                                  bool silent = false);
  static std::string streamConnect(const std::string &sessionID,
                                   const std::string &destination,
                                   bool silent = false);
  static std::string streamForward(const std::string &sessionID,
                                   const std::string &host, uint16_t port,
                                   bool silent = false);

  // Datagram session
  static std::string
  sessionCreate(SessionStyle style, const std::string &sessionID,
                const std::string &nickname, const uint16_t port,
                const std::string &host = "127.0.0.1",
                const std::string &destination = SAM_GENERATE_MY_DESTINATION,
                const std::string &options = "",
                const std::string &signatureType = SAM_SIGNATURE_TYPE);
  static std::string datagramSend(const std::string &nickname,
                                  const std::string &destination/*,
                                  const std::string &datagram_payload*/);
  static std::string datagramParse(const std::string &datagram_payload);

  static std::string namingLookup(const std::string &name);
  static std::string destGenerate();

  static eStatus checkAnswer(const std::string &answer);
  static std::string getValue(const std::string &answer,
                              const std::string &key);

 private:
  template<typename... t_args>
  static std::string
  createSAMRequest(const char *msg) { return {msg}; }

  template<typename... t_args>
  static std::string
  createSAMRequest(const char *format, t_args &&... args)
  {
    // ToDo: Check allocated buffer size

    const int bufferStatus = std::snprintf(nullptr, 0, format, args...);
    if (bufferStatus < 0)
      {
        print_error("Failed to allocate buffer");
        return {};
      }

    std::vector<char> buffer(bufferStatus + 1);
    const int status =
      std::snprintf(buffer.data(), buffer.size(), format, args...);

    if (status < 0)
      {
        print_error("Failed to format message");
        return {};
      }

#ifdef DEBUG_ON_STDOUT
    std::cout << "Status: " << status << std::endl;
#endif // DEBUG_ON_STDOUT

    return {buffer.data()};
  }
};

class I2pSocket
{
 public:
  I2pSocket(const std::string &SAMHost, uint16_t SAMPort);
  I2pSocket(const sockaddr_in &addr); // explicit because we don't want to create any socket implicity
  explicit I2pSocket(const I2pSocket &rhs); // creates a new socket with the same parameters
  ~I2pSocket();

  void bootstrapI2P();

  void write(const std::string &msg);
  std::string read();
  SOCKET release();
  void close();

  bool isOk() const;

  const std::string &getVersion() const;
  const std::string &getHost() const;
  uint16_t getPort() const;

  const sockaddr_in &getAddress() const;

  const std::string minVer_ = "3.0";
  const std::string maxVer_ = "3.1";

 private:
  SOCKET socket_;
  sockaddr_in servAddr_;
  std::string SAMHost_;
  uint16_t SAMPort_;
  std::string version_;

#ifdef WIN32
  static int instances_;
  static void initWSA();
  static void freeWSA();
#endif // WIN32

  void handshake();
  void init();

  I2pSocket &operator=(const I2pSocket &);
};

struct FullDestination
{
  std::string pub;
  std::string priv;
  bool isGenerated;

  FullDestination() {}
  FullDestination(const std::string &pub, const std::string &priv, bool isGenerated)
      : pub(pub), priv(priv), isGenerated(isGenerated) {}
};

template<class T>
struct RequestResult
{
  bool isOk;
  T value;

  RequestResult() : isOk(false) {}

  explicit RequestResult(const T &value) : isOk(true), value(value) {}
};

template<class T>
struct RequestResult<std::unique_ptr<T>>
{
  /**
   * a class-helper for resolving a problem with conversion
   * from temporary RequestResult to non-const RequestResult&
   */
  struct RequestResultRef
  {
    bool isOk;
    T *value;

    RequestResultRef(bool isOk, T *value) : isOk(isOk), value(value) {}
  };

  bool isOk;
  std::unique_ptr<T> value;

  RequestResult() : isOk(false) {}

  explicit RequestResult(std::unique_ptr<T> &&value)
      : isOk(true), value(std::move(value)) {}

  // some C++ magic
  RequestResult(RequestResultRef ref) : isOk(ref.isOk), value(ref.value) {}

  RequestResult &operator=(RequestResultRef ref)
  {
    if (value.get() != ref.value)
      {
        isOk = ref.isOk;
        value.reset(ref.value);
      }
    return *this;
  }

  operator RequestResultRef()
  {
    return RequestResultRef(this->isOk, this->value.release());
  }
};

template<>
struct RequestResult<void>
{
  bool isOk;

  RequestResult() : isOk(false) {}

  explicit RequestResult(bool isOk) : isOk(isOk) {}
};


class SAMSession
{
 public:
  SAMSession(const std::string &nickname,
                const std::string &SAMHost = SAM_DEFAULT_ADDRESS,
                uint16_t SAMPort = SAM_DEFAULT_PORT_TCP,
                const std::string &i2pOptions = SAM_DEFAULT_I2P_OPTIONS,
                const std::string &signatureType = SAM_SIGNATURE_TYPE);
  SAMSession(SAMSession& rhs);
  virtual ~SAMSession() = default;

  static std::string generateSessionID();
  RequestResult<const std::string> namingLookup(const std::string &name) const;
  RequestResult<const FullDestination> destGenerate() const;

  FullDestination createSession(const std::string &destination);
  FullDestination createSession(const std::string &destination, const std::string &sigType);
  virtual FullDestination createSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions) = 0;

  const FullDestination &getMyDestination() const;

  const sockaddr_in &getSAMAddress() const;
  const std::string &getSAMHost() const;
  uint16_t getSAMPort() const;
  const std::string &getNickname() const;
  const std::string &getSessionID() const;
  const std::string &getSAMMinVer() const;
  const std::string &getSAMMaxVer() const;
  const std::string &getSAMVersion() const;
  const std::string &getOptions() const;

  bool isSick() const;

 protected:
  static Message::Answer<const std::string> rawRequest(I2pSocket &socket, const std::string &requestStr);
  static Message::Answer<const std::string> request(I2pSocket &socket, const std::string &requestStr, const std::string &keyOnSuccess);
  static Message::eStatus request(I2pSocket &socket, const std::string &requestStr);

  static Message::Answer<const std::string> namingLookup(I2pSocket &socket, const std::string &name);
  static Message::Answer<const FullDestination> destGenerate(I2pSocket &socket);

  void fallSick() const;

  I2pSocket socket_;
  const std::string nickname_;
  const std::string sessionID_;
  FullDestination myDestination_;
  const std::string i2pOptions_;
  mutable bool isSick_;
};

class StreamSession : public SAMSession
{
 public:
  StreamSession(const std::string &nickname,
                const std::string &SAMHost = SAM_DEFAULT_ADDRESS,
                uint16_t SAMPort = SAM_DEFAULT_PORT_TCP,
                const std::string &destination = SAM_GENERATE_MY_DESTINATION,
                const std::string &i2pOptions = SAM_DEFAULT_I2P_OPTIONS,
                const std::string &signatureType = SAM_SIGNATURE_TYPE);
  explicit StreamSession(StreamSession &rhs);
  ~StreamSession();

  RequestResult<std::unique_ptr<I2pSocket>> accept(bool silent);
  RequestResult<std::unique_ptr<I2pSocket>>
  connect(const std::string &destination, bool silent);
  RequestResult<void> forward(const std::string &host, uint16_t port, bool silent);

  void stopForwarding(const std::string &host, uint16_t port);
  void stopForwardingAll();

 private:
  StreamSession(const StreamSession &rhs);
  StreamSession &operator=(const StreamSession &rhs);

  struct ForwardedStream
  {
    I2pSocket *socket;
    std::string host;
    uint16_t port;
    bool silent;

    ForwardedStream(I2pSocket *socket, const std::string &host, uint16_t port,
                    bool silent)
        : socket(socket), host(host), port(port), silent(silent) {}
  };

  typedef std::list<ForwardedStream> ForwardedStreamsContainer;

  ForwardedStreamsContainer forwardedStreams_;

  FullDestination createStreamSession(const std::string &destination);
  FullDestination createStreamSession(const std::string &destination, const std::string &sigType);
  FullDestination createStreamSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);
  FullDestination createSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);

  // commands
  static Message::Answer<const std::string> createStreamSession(
      I2pSocket &socket, const std::string &sessionID,
      const std::string &nickname, const std::string &destination,
      const std::string &options, const std::string &signatureType);

  static Message::eStatus accept(I2pSocket &socket,
                                 const std::string &sessionID, bool silent);
  static Message::eStatus connect(I2pSocket &socket,
                                  const std::string &sessionID,
                                  const std::string &destination, bool silent);
  static Message::eStatus forward(I2pSocket &socket,
                                  const std::string &sessionID,
                                  const std::string &host, uint16_t port,
                                  bool silent);
};

/**
 * WARNING:
 * Can only create a TCP session for manage connection.
 * The UDP client/server is need to be implemented from your app side.
 */

class DatagramSession : public SAMSession
{
 public:
  DatagramSession(const std::string &nickname,
                  const std::string &SAMHost = SAM_DEFAULT_ADDRESS,
                  uint16_t SAMPortTCP = SAM_DEFAULT_PORT_TCP,
                  uint16_t SAMPortUDP = SAM_DEFAULT_PORT_UDP,
                  const std::string &ClientAddress = SAM_DEFAULT_ADDRESS,
                  uint16_t clientPortUDP = SAM_DEFAULT_CLIENT_UDP,
                  const std::string &destination = SAM_GENERATE_MY_DESTINATION,
                  const std::string &i2pOptions = SAM_DEFAULT_I2P_OPTIONS,
                  const std::string &signatureType = SAM_SIGNATURE_TYPE);
  explicit DatagramSession(DatagramSession &rhs);
  ~DatagramSession();
  
  I2pSocket& getI2pSocket();

 private:
  DatagramSession(const DatagramSession &rhs);
  DatagramSession &operator=(const DatagramSession &rhs);

  std::string listenAddress_;
  uint16_t listenPortUDP_;
  uint16_t SAMPortUDP_;

  // commands
  FullDestination createDatagramSession(const std::string &destination);
  FullDestination createDatagramSession(const std::string &destination, const std::string &sigType);
  FullDestination createDatagramSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);
  FullDestination createSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);

  static Message::Answer<const std::string>
  createDatagramSession(I2pSocket &socket, const std::string &sessionID,
                        const std::string &nickname, const uint16_t port,
                        const std::string &host, const std::string &destination,
                        const std::string &options,
                        const std::string &signatureType);
};

/**
 * WARNING:
 * Can only create a TCP session for manage connection.
 * The UDP client/server is need to be implemented from your app side.
 */

class RawSession : public SAMSession
{
 public:
  RawSession(const std::string &nickname,
             const std::string &SAMHost = SAM_DEFAULT_ADDRESS,
             uint16_t SAMPortTCP = SAM_DEFAULT_PORT_TCP,
             uint16_t SAMPortUDP = SAM_DEFAULT_PORT_UDP,
             const std::string &ClientAddress = SAM_DEFAULT_ADDRESS,
             uint16_t clientPortUDP = SAM_DEFAULT_CLIENT_UDP,
             const std::string &destination = SAM_GENERATE_MY_DESTINATION,
             const std::string &i2pOptions = SAM_DEFAULT_I2P_OPTIONS,
             const std::string &signatureType = SAM_SIGNATURE_TYPE);
  explicit RawSession(RawSession &rhs);
  ~RawSession();

 private:
  RawSession(const RawSession &rhs);
  RawSession &operator=(const RawSession &rhs);

  std::string listenAddress_;
  uint16_t listenPortUDP_;
  uint16_t SAMPortUDP_;

  FullDestination createRawSession(const std::string &destination);
  FullDestination createRawSession(const std::string &destination, const std::string &sigType);
  FullDestination createRawSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);
  FullDestination createSession(const std::string &destination, const std::string &sigType, const std::string &i2pOptions);

  static Message::Answer<const std::string>
  createRawSession(I2pSocket &socket, const std::string &sessionID,
                   const std::string &nickname, const uint16_t port,
                   const std::string &host, const std::string &destination,
                   const std::string &options,
                   const std::string &signatureType);
};

} // namespace SAM

#else  // __cplusplus
#include "i2psam-c.h"
#endif // __cplusplus
#endif // I2PSAM_H__
