#include "../include/server.hpp"

neroshop::Server::Server()
{
	#ifdef __windows__
	    socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	    if (socket == INVALID_SOCKET)
        {
            std::cerr << "Could not create socket " << WSAGetLastError() << std::endl;
			WSACleanup();
        }
	#endif
	#ifdef __gnu_linux__
	    socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket < 0) 
		{
			std::cerr << "Could not create socket" << std::endl;
			return;
		}
		// set socket options : SO_REUSEADDR (TCP:restart a closed/killed process on the same address so you can reuse address over and over again)
		int one = 1; // enable
        if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one)) < 0)
        {
            std::cerr << "Could not set socket options" << std::endl;
        }
	#endif
}
//////////
neroshop::Server::~Server()
{
	close();
}
//////////
bool neroshop::Server::bind(unsigned int port)
{
	#ifdef __windows__
        memset(&server_addr, 0, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons( port );        // Port to listen
        server_addr.sin_addr.s_addr = INADDR_ANY;
	
        if (::bind(this->socket, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) 
	    {
            std::cerr << "Cannot bind socket" << std::endl; 
		    close();
			shutdown();
			return false;
        }
	#endif
	
	#ifdef __gnu_linux__
	    bzero((char *) &server_addr, sizeof(server_addr));
	    server_addr.sin_port = htons(port);
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = INADDR_ANY;
		
		if (::bind(socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
		{
			std::cerr << "Cannot bind socket" << std::endl;
			close();
			return false;
		}
	#endif
	this->port = port;
	return true;
}
//////////	
bool neroshop::Server::listen()
{
#ifdef __windows__
    if(::listen(socket, 2) < 0) // Now, listen for a connection  // number of connections allowed on the incoming queue
	{
        std::cerr << "Cannot listen for a connection" << std::endl;
		shutdown();
		close();
		return false;
    }
#endif
#ifdef __gnu_linux__
	if(::listen(socket, SOMAXCONN) < 0) {//SOMAXCONN=4096 // number of requests to listen to at a time
        std::cerr << "Cannot listen for a connection" << std::endl;
		shutdown();
		close();			
		return false;
	}
#endif
	return true;
}
//////////
bool neroshop::Server::accept()
{
#ifdef __windows__
	clilen = sizeof(client_addr);
	newsocket = ::accept(this->socket, (struct sockaddr *) &client_addr, &clilen);
    if (newsocket < 0)
	{		 
		shutdown();
		close();
		return false;
	}
	if(newsocket > 0)
	{
		std::cout << newsocket << "\n";
	}
	std::cout << "server: got connection from " << inet_ntoa(client_addr.sin_addr) << " port " << ntohs(client_addr.sin_port) << std::endl;
#endif	
#ifdef __gnu_linux__
	clilen = sizeof(client_addr);
    newsocket = ::accept(socket, (struct sockaddr *) &client_addr, &clilen);
    if (newsocket < 0) {
        std::cerr << ("ERROR on accept\n");
		shutdown();
		close();
		return false;
	}
	std::cout << NEROSHOP_TAG "\033[0;37mReceived connection from " << inet_ntoa(client_addr.sin_addr) << ":\033[0;36m" << ntohs(client_addr.sin_port) << "\033[0m" << std::endl;//std::cout << "client_socket: " << newsocket << std::endl; // always prints out 5
#endif
	return true;
}
//////////
bool neroshop::Server::accept_all()
{
	/*
	struct addrinfo *result = 0, *ai = 0, *ai4 = 0;
	static struct addrinfo hints = {};

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;	
	
    getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result);	
	 for (ai = result; ai; ai = ai->ai_next) {
        if (ai->ai_family == PF_INET6) break;
        else if (ai->ai_family == PF_INET) ai4 = ai;
    }
	ai = ai ? ai : ai4;
	
	newsocket = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	int b = ::bind(this->socket, result->ai_addr, (int)result->ai_addrlen);
	int l = ::listen(this->socket, 1);
	int a = ::accept(this->socket, nullptr, nullptr);
	*/
	return true;
}
//////////
void neroshop::Server::write(const std::string& text) // send data
{
#ifdef __windows__
    send(newsocket, text.c_str(), text.length(), 0);	
#endif
	
#ifdef __gnu_linux__
    //bzero(buffer,256);
    //fgets(buffer,255,stdin);
	ssize_t write_result = ::write(newsocket, text.c_str(), text.length()/*buffer, strlen(buffer)*/); // #include <unistd.h>
    if(write_result < 0) { // -1 = error
        std::cout << "Server unable to write to client" << std::endl;
    }
#endif
}
//////////
std::string neroshop::Server::read() // receive data
{
#ifdef __windows__
    if (recv(newsocket, buffer, 512, 0) < 0)
	{	
		std::cerr << "Client disconnected" << std::endl;
		shutdown();
		close();
	}
#endif
#ifdef __gnu_linux__
    memset(buffer, 0, 256); // clear buffer (fills buffer with 0's) before reading into buffer
    ssize_t read_result = ::read(newsocket, buffer, 255);//::read(newsocket, (void *)buffer_new.c_str(), buffer_new.length()); // https://stackoverflow.com/questions/10105591/is-it-possible-to-use-an-stdstring-for-read  // #include <unistd.h>
	if(read_result < 0) { // -1 = error
		perror("socket read error: ");//std::cerr << "Client disconnected" << std::endl;
		shutdown();
		close();			
	}
	if(read_result == 0) {
        std::cerr << NEROSHOP_TAG "\033[0mClient orderly shut down the connection." << std::endl;
        shutdown();
        close();
        exit(0);//or wait for client to re-connect to server ?
    }
    if(read_result > 0 && read_result < 511) {
        //std::cout << "Received less bytes than expected" << std::endl;
    }
    //::write (STDOUT_FILENO, buffer, read_result);
#endif	
	return static_cast<std::string>(buffer);//buffer_new.c_str();
}
//////////
void neroshop::Server::close()
{
#ifdef __windows__
	closesocket(socket);
	closesocket(newsocket);
#endif	
#ifdef __gnu_linux__
	::close(socket);
	::close(newsocket);
#endif
}
//////////
void neroshop::Server::shutdown()
{
#ifdef __windows__
	::shutdown(socket, SD_BOTH); // SD_RECEIVE, SD_SEND, SD_BOTH
	::shutdown(newsocket, SD_BOTH);
#endif
#ifdef __gnu_linux__
	::shutdown(socket, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
	::shutdown(newsocket, SHUT_RDWR);
#endif
}
////////// // figure out how to accept to multiple clients, and how to check if connection is lost
#ifdef __gnu_linux__
int neroshop::Server::get_socket() const {
    return socket;
}
//////////
int neroshop::Server::get_client_socket(int index) const {
    return newsocket;
}
#endif
//////////
//////////
