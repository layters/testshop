#include "../include/client.hpp"

neroshop::Client::Client() : socket(0) {
    create();
}
//////////
neroshop::Client::~Client() {
	close();
}
//////////
std::unique_ptr<neroshop::Client> neroshop::Client::client_obj (nullptr);
//////////
void neroshop::Client::create() {
#ifdef __windows__
	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_UNSPEC, AF_BTH (Bluetooth), AF_INET6 (IPV6), IPPROTO_UDP (UDP)
	if (socket == SOCKET_ERROR)
    {
        std::cerr << "Could not create socket " << WSAGetLastError() << std::endl;
        WSACleanup();
    }
#endif	
#ifdef __gnu_linux__
    if(socket) return; // socket must be null before a new one can be created (if socket is not null then it means it was never closed)
	socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if(socket < 0) {
		std::cerr << "Could not create socket " << std::endl;
	}
#endif
}
//////////
bool neroshop::Client::connect(unsigned int port, std::string address)
{
#ifdef __windows__
	host = gethostbyname(address.c_str());
	socket_addr.sin_port = htons(port);
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	// connect to a server
    return (::connect(socket, (struct sockaddr *)(&socket_addr), sizeof(socket_addr)) == 0);
#endif
#ifdef __gnu_linux__
	host = gethostbyname(address.c_str());
	if(host == nullptr) {
		std::cerr << "No host to connect to" << std::endl;
	}
	bzero((char *) &socket_addr, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    bcopy((char *)host->h_addr, (char *)&socket_addr.sin_addr.s_addr, host->h_length);
    socket_addr.sin_port = htons(port);
	// connect to a server	
	if(::connect(socket, (struct sockaddr *)(&socket_addr), sizeof(socket_addr)) < 0) {
		std::cerr << "Could not connect to server" << std::endl; // port is closed
		close(); // kill socket // https://linux.die.net/man/3/connect => Application Usage
		return false;
	}
	return true;
#endif
    return false;
}
//////////
void neroshop::Client::write(const std::string& text)
{
#ifdef __windows__
    send(socket, text.c_str(), text.length(), 0);	
#endif	
#ifdef __gnu_linux__
	ssize_t write_result = ::write(socket, text.c_str(), text.length());
	if(write_result < 0) { // -1 = error
		std::cerr << "Could not write to server" << std::endl;
	}
#endif
}
std::string neroshop::Client::read()
{
	// Display message from server
#ifdef __windows__
	memset(buffer, 0, 511); // clear buffer (fills buffer with 0's) before reading into buffer
	if(recv(socket, buffer, 512, 0) < 0) {
		std::cerr << "Could not read from server" << std::endl;
	}
	return static_cast<std::string>(buffer);
#endif	
#ifdef __gnu_linux__
	memset(buffer, 0, 256); // clear buffer (fills buffer with 0's) before reading into buffer//bzero(buffer, 256); // bzero is deprecated
	ssize_t read_result = ::read(socket, buffer, 255);
	if(read_result < 0) {
		std::cerr << "Could not read from server" << std::endl;	
	}
	return static_cast<std::string>(buffer);
#endif
}
//////////	
void neroshop::Client::close()
{
#ifdef __windows__
    closesocket(socket);	
#endif	
#ifdef __gnu_linux__
    if(socket == 0) return; // cannot close a socket that has already been closed (this is to prevent double closing)
	::close(socket);
	socket = 0; // nullify socket after closing it, so we know it's been deleted//std::cout << "client socket closed: 0\n";
#endif
}
//////////
void neroshop::Client::shutdown()
{
#ifdef __windows__
	::shutdown(socket, SD_BOTH); // SD_RECEIVE, SD_SEND, SD_BOTH
#endif	
#ifdef __gnu_linux__
	::shutdown(socket, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
#endif
}
//////////
//////////
void neroshop::Client::disconnect() { // if only shutdown() is called, the client socket will still be alive which is why we must call close() as well
	shutdown();
	close();
}
//////////
bool neroshop::Client::reconnect(unsigned int port, std::string address) { // kill socket first before attempting to re-connect
    close();
    return connect(port, address);
}
//////////
#ifdef _WIN32
SOCKET neroshop::Client::get_socket() const {
    return socket;
}
#endif
//////////
#ifdef __gnu_linux__
int neroshop::Client::get_socket() const {
    return socket;
}
#endif
//////////
neroshop::Client * neroshop::Client::get_main_client() {
    if(!client_obj.get()) { 
        client_obj = std::make_unique<Client>();
    }
    return client_obj.get();
}
//////////
//////////
bool neroshop::Client::is_connected() const { // https://stackoverflow.com/a/4142038 // can only work when close() is called
    /*//if(socket == 0) return false; // a null socket is a closed socket
    int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (socket, SOL_SOCKET, SO_ERROR, &error, &len);
    // test if socket is up
    if(retval != 0) {
        // there was a problem getting the error code
        neroshop::print(std::string(strerror(errno)) + " (server may be offline)", 1);//strerror(retval));//https://stackoverflow.com/questions/4142012/how-to-find-the-socket-connection-state-in-c#comment68908270_4142038
        return false;
    }
    if(error != 0) {
        // socket has a non zero error status
        neroshop::print("socket error: " + std::string(strerror(error)), 1);
        return false;
    }
    if(!test_socket()) {
        return false;
    }
    return true;*/
    if (not socket) return false;
    #ifdef __linux__
    const std::string& text = "";
    ssize_t send_result = ::send(socket, text.c_str(), text.length(), MSG_NOSIGNAL);
    if (send_result < 0) return false; else return true;
    #endif    
}
//////////
bool neroshop::Client::test_socket() {//const { // testing socket to see if it is connected // https://stackoverflow.com/questions/1795193/check-connection-open-or-closed-in-c-in-linux
    // send message to server?
#ifdef __gnu_linux__
    const std::string& text = "";
	ssize_t send_result = ::send(socket, text.c_str(), text.length(), MSG_NOSIGNAL); // only writes once to server?
	if(send_result < 0) { // -1 = error
	    if(errno == EPIPE) { //EPIPE=32, Broken pipe
	        neroshop::print("socket: " + std::string(strerror(errno)) + " (connection lost)", 1); // errno will print "Broken pipe" if server goes offline : SIGPIPE
            //close(); 
		}
		return false;
	}
    /*struct sigaction new_actn, old_actn;
    new_actn.sa_handler = SIG_IGN;
    sigemptyset (&new_actn.sa_mask);
    new_actn.sa_flags = 0;
    return sigaction (SIGPIPE, &new_actn, &old_actn);*/		
#endif
    return true;
}
//////////
//////////
//////////
//////////
