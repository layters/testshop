#include "i2p.hpp"

int i2pd_sam_test() {
    // Address and ports for SAM bridge
    std::string address = "127.0.0.1";
    int portTCP = 7656;
    int portUDP = 7655;

    // Create SAM bridge
    i2p::client::SAMBridge samBridge(address, portTCP, portUDP, false); // Set the last parameter to true for single-threaded operation

    // Start the SAM bridge
    samBridge.Start();

/*    // Create a SAM session
    std::string sessionID = "mysession";
    i2p::client::SAMSessionType sessionType = i2p::client::eSAMSessionTypeStream;
    std::string destination = ""; // empty string means transient
    std::map<std::string, std::string> params; // additional parameters if needed

    std::shared_ptr<i2p::client::SAMSession> session = samBridge.CreateSession(sessionID, sessionType, destination, &params);

    // Add the session to the bridge
    samBridge.AddSession(session);

    // Perform SAM-related operations
    // ...*/

    // Stop the SAM bridge
    samBridge.Stop();
    
    return 0;
}
/*int main(int argc, char* argv[]) {
    // Initialize i2pd router
    i2p::api::InitI2P(argc, argv, "MyI2PApp");

    // Create a local destination
    std::shared_ptr<i2p::client::ClientDestination> localDestination =
        i2p::api::CreateLocalDestination(true, i2p::data::SIGNING_KEY_TYPE_ECDSA_SHA256_P256);

    // Start the i2pd router
    i2p::api::StartI2P(std::shared_ptr<std::ostream>(nullptr));

    // Request LeaseSet for a remote destination
    i2p::data::IdentHash remoteIdent;
    // You should set 'remoteIdent' to the actual IdentHash of the remote destination
    i2p::api::RequestLeaseSet(localDestination, remoteIdent);

    // Create an i2p stream to the remote destination
    std::shared_ptr<i2p::stream::Stream> stream = i2p::api::CreateStream(localDestination, remoteIdent);
    if (stream) {
        // Send data over the stream
        const char* data = "Hello, I2P!";
        stream->Send(reinterpret_cast<const uint8_t*>(data), std::strlen(data));

        // ... Perform other stream-related operations

        // Close and destroy the stream
        i2p::api::DestroyStream(stream);
    }

    // Stop the i2pd router
    i2p::api::StopI2P();

    // Terminate i2pd
    i2p::api::TerminateI2P();
    return 0;
}*/

/*int i2psam_test() {
    // Create an Instance of DatagramSession (UDP-based)
    std::string session_nickname = "neromon";
    SAM::DatagramSession datagram_session(session_nickname,
        SAM_DEFAULT_ADDRESS, SAM_DEFAULT_PORT_TCP, SAM_DEFAULT_PORT_UDP,
        SAM_DEFAULT_ADDRESS, 50881); // inherits from SAMSession (which has a I2pSocket member)
    // Print DatagramSession Information
    std::cout << "SAM Host: " << datagram_session.getSAMHost() << std::endl;
    std::cout << "SAM Port: " << datagram_session.getSAMPort() << std::endl;
    std::cout << "SAM Nickname: " << datagram_session.getNickname() << std::endl;
    std::cout << "SAM Session ID: " << datagram_session.getSessionID() << std::endl;
    std::cout << "SAM Version: " << datagram_session.getSAMVersion() << std::endl;
    std::cout << "SAM Options: " << datagram_session.getOptions() << std::endl;
    std::cout << "SAM isSick: " << datagram_session.isSick() << std::endl;
    
    // Generate the Destination for Datagram Session
    auto destination_result = datagram_session.destGenerate();
    if (!destination_result.isOk) {
        std::cerr << "Error generating destination: " 
        << "pub: " << destination_result.value.pub
        << ", priv: " << destination_result.value.priv
        << ", is_generated: " << destination_result.value.isGenerated << std::endl;
        return 1;
    }
    SAM::FullDestination destination = destination_result.value;
    std::cout << "destination.pub: " << destination.pub << std::endl << std::endl;
    std::cout << "destination.priv: " << destination.priv << std::endl;
    std::cout << "is_generated: " << destination_result.value.isGenerated << std::endl;
    //-------------------------------------------------------
    // To talk to another SAM session we use namingLookup(const std::string &<b32.i2p address>)
    //-------------------------------------------------------
    ////datagram_session.socket_ // I2pSocket socket_ (protected class member)
    // This isn't working :(
    std::string datagram_payload = "Hello, I2P!";
    std::cout << "Sending Datagram: " << datagram_payload << std::endl;
    datagram_session.getI2pSocket().write(datagram_payload);

    // Receive a datagram
    std::string received_datagram = datagram_session.getI2pSocket().read();
    std::cout << "Received Datagram: " << received_datagram << std::endl;
    //-------------------------------------------------------
    ////SAM::I2pSocket i2p_socket(SAM_DEFAULT_ADDRESS, SAM_DEFAULT_PORT_UDP); // bootstrapI2P(), which calls handshake() is called here in this constructor
    // If the handshake works, we're talking to a valid I2P router.
    //i2p_socket.write("My name's Jeff");
    //std::cout << "Response: " << i2p_socket.read() << std::endl;
    //const std::string &i2p_socket.getVersion() const;
    //const std::string &i2p_socket.getHost() const;
    //uint16_t i2p_socket.getPort() const;
    //-------------------------------------------------------
    
    //-------------------------------------------------------
    return 0;
}*/

/*int main(int argc, char* argv[]) {
    int error = i2psam_test();

    return error;
}*/

// g++ i2p.cpp -o i2p -I../../../external/i2pd/daemon -I../../../external/i2pd/i18n -I../../../external/i2pd/libi2pd -I../../../external/i2pd/libi2pd_client -L../../../build -li2pd -li2pd_client -lboost_program_options -lboost_filesystem -lboost_system -lboost_date_time -lcrypto -lssl -lz -lpthread 

// g++ i2p.cpp -o i2p -I../../../external/i2psam -L../../../build -li2psam
