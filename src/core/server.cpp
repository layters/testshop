#include "server.hpp"

neroshop::Server::Server()
#if defined(NEROSHOP_USE_LIBUV)
 : handle_tcp(nullptr), handle_udp (nullptr)
#endif
{
    #if defined(NEROSHOP_USE_LIBUV)
    //uv_loop_t loop = uv_default_loop();
    // causes Segfault if called outside of main function
	int result = uv_tcp_init(uv_default_loop()/*loop*/, handle_tcp);
	if(result != 0) {
	    neroshop::print("uv_tcp_init error: " + std::string(uv_strerror(result)), 1);
	}	    
    #endif	
	#if defined(__gnu_linux__)
	socket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0) {
		std::cerr << "Could not create socket" << std::endl;
		return;
	}
	// set socket options : SO_REUSEADDR (TCP:restart a closed/killed process on the same address so you can reuse address over and over again)
	int one = 1; // enable
    if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one)) < 0) {
        std::cerr << "Could not set socket options" << std::endl;
    }
	#endif    
	// Create a new raft server
	/*int server_count = 1;
	void * connection_user_data = nullptr; // pointer to userdata
	// Generate random number to be used for node_id
	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(0, 100);
	int node_id = static_cast<int>(dist(mt)); // SHOULD be random
	bool peer_is_self = true;
    
    raft = raft_new();

	for(int i = 0; i < server_count; ++i) {
	    raft_add_node(static_cast<raft_server_t*>(raft), connection_user_data, node_id, peer_is_self);
	    std::cout << "raft node has been added\n";
	}*/
}
////////////////////
neroshop::Server::~Server() {
	close();
}
////////////////////
bool neroshop::Server::bind(unsigned int port)
{
    #if defined(NEROSHOP_USE_LIBUV)
	struct sockaddr_in bind_addr;
    std::string ipv4_default = "0.0.0.0"; // Client addr
    std::string ipv6_default = "::/0"; // ::/0 is the IPv6 equivalent of 0.0.0.0/0 (IPv4)
    std::string ipv4_localhost = "127.0.0.1"; // Server addr
    std::string ipv6_localhost = "::1"; // ::1/128 is the IPv6 equivalent of 127.0.0.1/8 (IPv4)
	//int port = DEFAULT_PORT;//7000;//1234;
	int result = uv_ip4_addr(ipv4_localhost.c_str(), port, &bind_addr);
	if(result != 0) {
	    neroshop::print("uv_ip4_addr error: " + std::string(uv_strerror(result)), 1);
	}
	
	result = uv_tcp_bind(handle_tcp, (const struct sockaddr *)&bind_addr, 0);
	if(result != 0) {
	    neroshop::print("uv_tcp_bind error: " + std::string(uv_strerror(result)), 1);
	}
    #endif	
    #if defined(__gnu_linux__)
    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));//bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_port = htons(port);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
		
	if(::bind(socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Cannot bind socket" << std::endl;
		close();
		return false;
	}    
    #endif
	return true;
}
////////////////////	
bool neroshop::Server::listen()
{
    #if defined(NEROSHOP_USE_LIBUV)
    int result = 0;
	if((result = uv_listen((uv_stream_t*)handle_tcp, 128/*DEFAULT_BACKLOG*/, on_new_connection)) != 0) {
		neroshop::print("uv_listen error: " + std::string(uv_strerror(result)), 1);
		return false;
	}
    #endif
    #if defined(__gnu_linux__)
	if(::listen(socket, SOMAXCONN) < 0) {//SOMAXCONN=4096 // number of requests to listen to at a time
        std::cerr << "Cannot listen for a connection" << std::endl;
		shutdown();
		close();			
		return false;
	}    
    #endif    
	return true;
}
////////////////////
bool neroshop::Server::accept() {
    #if defined(__gnu_linux__)
    struct sockaddr_in client_addr;
	socklen_t clilen = sizeof(client_addr);
    /*int */client_socket = ::accept(socket, (struct sockaddr *) &client_addr, &clilen);
    if (client_socket < 0) {
        std::cerr << ("ERROR on accept\n");
		shutdown();
		close();
		return false;
	}
	// Daemon cannot write to stdin, so we must use the Server::write function
	//std::cout << NEROSHOP_TAG "\033[0;37mReceived connection from " << inet_ntoa(client_addr.sin_addr) << ":\033[0;36m" << ntohs(client_addr.sin_port) << "\033[0m" << std::endl;//std::cout << "client_socket: " << client_socket << std::endl; // always prints out 5    
    write("\033[0;37mReceived connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":\033[0;36m" + std::to_string(ntohs(client_addr.sin_port)) + "\033[0m\n");
    #endif
	return true;
}
////////////////////
bool neroshop::Server::accept_all()
{
	return true;
}
////////////////////
void neroshop::Server::write(const std::string& message) {
    #if defined(__gnu_linux__)
	ssize_t write_result = ::write(client_socket, message.c_str(), message.length()/*buffer, strlen(buffer)*/);
    if(write_result < 0) { // -1 = error
        std::cout << "Server unable to write to client" << std::endl;
    }    
    #endif
}
////////////////////
std::string neroshop::Server::read() // receive data
{
    #if defined(__gnu_linux__)
    memset(buffer, 0, 256); // clear buffer (fills buffer with 0's) before reading into buffer
    ssize_t read_result = ::read(client_socket, buffer, 255);//::read(client_socket, (void *)buffer_new.c_str(), buffer_new.length()); // https://stackoverflow.com/questions/10105591/is-it-possible-to-use-an-stdstring-for-read  // #include <unistd.h>
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
    return static_cast<std::string>(buffer);    
    #endif
	return "";
}
////////////////////
void neroshop::Server::close() {
    #if defined(__gnu_linux__)
    ::close(socket);
    #endif
}
////////////////////
void neroshop::Server::shutdown() {
    #if defined(__gnu_linux__)
    ::shutdown(socket, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
    #endif
}
////////////////////
////////////////////
////////////////////
#if defined(NEROSHOP_USE_LIBUV)
void neroshop::Server::on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        neroshop::print("New connection error: " + std::string(uv_strerror(status)), 1);
        ////uv_close((uv_handle_t*) client, NULL); // close tcp connection
        // error!
        return;
    }
    
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop()/*loop*/, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) { // success
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);//std::cout << "Got a connection from a peer/client!\n";//neroshop::print(std::string("\033[0;37mReceived a connection from ") + "<ip>" + ":\033[0;36m" + "<port>");
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }    
}
////////////////////
void neroshop::Server::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
    //memset(buf->base, 0, buf->len);
}
////////////////////
void neroshop::Server::echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if(nread < 0) {//== -1) {
        if(nread != UV_EOF) neroshop::print("Read error: " + std::string(uv_err_name(nread)), 1);
        uv_close((uv_handle_t*)client, NULL);
        return;
    }        

    std::cout << "server_read_message_from_client: " << buf->base << std::endl;

    // Write what the server read from the client (works so far)
    //if (nread > 0) {
        uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
        write_req->data = (void*)buf->base;
        const_cast<uv_buf_t *>(buf)->len = nread;
        uv_write(write_req, client, buf, 1, echo_write);
        //return;
    //}

    ////free(buf->base);
}
////////////////////
void neroshop::Server::echo_write(uv_write_t *req, int status) {
    if(status == -1) {
        neroshop::print("Write error: " + std::string(uv_strerror(status)));
    }
    char *buffer_base = (char*) req->data;
    free(buffer_base);
    free(req);
}
#endif
////////////////////
//template<typename F> void neroshop::Server::bind(const std::string& name, F func) {//F response) {
	// if client requests a string e.g "LOGIN" then server will respond with login()
	//response();
	// I don't know what I'm doing here ...
	//std::map<std::string, F> rr_pair = { { request, response }, };
//}
////////////////////
/*#if !defined(_WIN32)
//#include <sys/types.h> // ?
#include <ifaddrs.h> // freeifaddrs
#include <linux/if_link.h> // rtnl_link_stats
#endif*/
//void neroshop::Server::get_local_ip() {
/*struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        } 
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return;// 0;*/
    
    //From https://www.man7.org/linux/man-pages/man3/getifaddrs.3.html :
/*struct ifaddrs *ifaddr;
           int family, s;
           char host[NI_MAXHOST];

           if (getifaddrs(&ifaddr) == -1) {
               perror("getifaddrs");
               exit(EXIT_FAILURE);
           }

           // Walk through linked list, maintaining head pointer so we
           //   can free list later. 

           for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
                    ifa = ifa->ifa_next) {
               if (ifa->ifa_addr == NULL)
                   continue;

               family = ifa->ifa_addr->sa_family;

               // Display interface name and family (including symbolic
               //   form of the latter for the common families). 

               printf("%-8s %s (%d)\n",
                      ifa->ifa_name,
                      (family == AF_PACKET) ? "AF_PACKET" :
                      (family == AF_INET) ? "AF_INET" :
                      (family == AF_INET6) ? "AF_INET6" : "???",
                      family);

               // For an AF_INET* interface address, display the address. 

               if (family == AF_INET || family == AF_INET6) {
                   s = getnameinfo(ifa->ifa_addr,
                           (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                                 sizeof(struct sockaddr_in6),
                           host, NI_MAXHOST,
                           NULL, 0, NI_NUMERICHOST);
                   if (s != 0) {
                       printf("getnameinfo() failed: %s\n", gai_strerror(s));
                       exit(EXIT_FAILURE);
                   }

                   printf("\t\taddress: <%s>\n", host);

               } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
                   struct rtnl_link_stats *stats = (struct rtnl_link_stats *)ifa->ifa_data;

                   printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                          "\t\ttx_bytes   = %10u; rx_bytes   = %10u\n",
                          stats->tx_packets, stats->rx_packets,
                          stats->tx_bytes, stats->rx_bytes);
               }
           }

           freeifaddrs(ifaddr);    */
//}
////////////////////
