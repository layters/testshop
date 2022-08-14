#include <dirent.h>
#include <iterator>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <vector>
#include <iostream>
// neroshop
#include "server.hpp"
#include "debug.hpp"
//#include "db2.hpp" // daemon should handle database server requests from the client ??
#define NEROMON_TAG "\033[1;95m[neromon]:\033[0m "

using namespace neroshop;

Server * server = new Server();

void close_server() {
    server->shutdown();
    delete server; // calls destructor which calls closesocket()
    server = nullptr;
    std::cout << NEROMON_TAG "\033[1;91mdisconnected\033[0m" << std::endl;
}
////////////////////
#define DEFAULT_PORT 7000
#define DEFAULT_BACKLOG 128
////////////////////
// Alloc callback (uv_alloc_cb)
// suggested_size で渡された領域を確保
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
    ////memset(buf->base, 0, buf->len);
}
////////////////////
void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
void on_client_write(uv_write_t *req, int status);
////////////////////
// Read callback (uv_read_cb)
void on_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if(nread < 0) {//== -1) {
        neroshop::print("error on_client_read", 1);
        uv_close((uv_handle_t*)stream, NULL);
        return;
    }
    // クライアントから受け取ったデータ
    // データは buf->base に格納されている。    
   //if(nread > 0) {
    std::cout << "server_read_message_from_client: " << buf->base << std::endl; // there is data available
    // ファイルを開く (buf がファイルの場合)
    ////int mode = 0;
    ////uv_fs_open(uv_default_loop(), &open_req, filename, O_RDONLY, mode, on_file_open);    
   //}
   // Write what the server read from the client (works)
    uv_write_t *write_req = (uv_write_t*)malloc(sizeof(uv_write_t));
    write_req->data = (void*)buf->base;
    const_cast<uv_buf_t *>(buf)->len = nread;
    uv_write(write_req, stream, buf, 1, on_client_write);
}
////////////////////
// Write callback (uv_write_cb)
void on_client_write(uv_write_t *req, int status) { // change to on_write_end
    if(status == -1) {
        neroshop::print("error on_client_write", 1);
        return;
    }
    char *buffer_base = (char*) req->data;
    free(buffer_base);
    free(req);
}
////////////////////
// Shutdown callback
void shutdown_cb(uv_shutdown_t *req, int status) {
}
////////////////////
uv_loop_t * loop;
// Connection callback - received an incoming connection
void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        neroshop::print("New connection error: " + std::string(uv_strerror(status)), 1);
        ////uv_close((uv_handle_t*) client, NULL); // close tcp connection
        // error!
        return;
    }
    
    // クライアントを保持するためのメモリを確保
    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    // loop への登録
    uv_tcp_init(loop, client);
    
    // accept the connection
    if (uv_accept(server, (uv_stream_t*) client) == 0) { // success
        //----------------------------------------
        /*std::string message = "Server: Welcome to the server";//NEROMON_TAG "\033[1;32mconnected\033[0m";
        // write to client
        char buffer[100];
        uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
  
        buf.len = message.length();
        buf.base = const_cast<char *>(message.c_str());          
        
        uv_write_t write_req;

        // writes
        int buf_count = 1; // number of times we write
        // To write to the client
        uv_write(&write_req, (uv_stream_t*)client, &buf, buf_count, on_client_write);*/
        //----------------------------------------
        // reading from client
        uv_read_start((uv_stream_t*) client, alloc_buffer, on_client_read);//std::cout << "Got a connection from a peer/client!\n";//neroshop::print(std::string("\033[0;37mReceived a connection from ") + "<ip>" + ":\033[0;36m" + "<port>"); // daemons cannot use std::cout so this is useless
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
    // then continue reading for any client messages
}
///////////////////
void do_heartbeat()
{
    // accept multiple connections - I noticed that new clients are only accepted when the primary client writes to the server
    if(server->accept() != -1) // accepts any incoming connection
    {
	    //std::cout << "server's client_socket: " << server->get_client_socket() << std::endl;// returns 5
	    //std::thread new_client(client); // create a new client thread each time it accepts
	    //new_client.join();
	    server->write(NEROMON_TAG "\033[1;32mconnected\033[0m"); // write to client once
    } //else exit(0);
	std::cout << server->read() << std::flush << std::endl;
}
// For security purposes, we don't allow any arguments to be passed into the daemon
int main(void)
{
    // A daemon cannot use the terminal, so close standard file descriptors for security reasons
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // Daemon-specific intialization should go here
    const int SLEEP_INTERVAL = 1;
    //////////////////////////////////////////////////
    // Start server
    /*std::atexit(close_server);
    int server_port = 1234;//(std::stoi(port));//port 38081 fails
	if(server->bind(server_port)) {
	    std::cout << std::endl << NEROMON_TAG "\033[1;97mbound to port " + std::to_string(server_port) << "\033[0m" << std::endl;
	}
	server->listen(); // listens for any incoming connection*/
	/////////////////////////////////////////////////////////////////
	/*uv_loop_t */loop = uv_default_loop();
	/////////////////////////////////////////////////////////////////
	/*uv_pipe_t server_pipe;
	uv_pipe_init(loop, &server_pipe, 1); // 1 = ipc
	
	////signal(SIGINT, remove_sock);
    int resultp = 0;
    if((resultp = uv_pipe_bind(&server_pipe, "echo.sock"))) {
        fprintf(stderr, "Bind error %s\n", uv_err_name(resultp));
        return 1;
    }
    if((resultp = uv_listen((uv_stream_t*) &server_pipe, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(resultp));
        return 2;
    }*/
    
    //uv_read_start((uv_stream_t*)&server_pipe, alloc_buffer, on_new_connection);	
	/////////////////////////////////////////////////////////////////
    // Start server (libuv)
	uv_tcp_t server;
	int result = uv_tcp_init(loop, &server);
	if(result != 0) {
	    neroshop::print("uv_tcp_init error: " + std::string(uv_strerror(result)), 1);
	}	
	
	struct sockaddr_in bind_addr;
    std::string ipv4_default = "0.0.0.0";
    std::string ipv6_default = "::/0"; // ::/0 is the IPv6 equivalent of 0.0.0.0/0 (IPv4)
    std::string ipv4_localhost = "127.0.0.1";
    std::string ipv6_localhost = "::1"; // ::1/128 is the IPv6 equivalent of 127.0.0.1/8 (IPv4)
	int port = DEFAULT_PORT/*1234*/;
	result = uv_ip4_addr(ipv4_default.c_str(), port, &bind_addr);
	if(result != 0) {
	    neroshop::print("uv_ip4_addr error: " + std::string(uv_strerror(result)), 1);
	}
	// bind server to a port
	result = uv_tcp_bind(&server, (const struct sockaddr *)&bind_addr, 0);
	if(result != 0) {
	    neroshop::print("uv_tcp_bind error: " + std::string(uv_strerror(result)), 1);
	}
	std::cout << std::endl << NEROMON_TAG "\033[1;97mbound to port " + std::to_string(port) << "\033[0m" << std::endl;
	// listen for any incoming connections
	if((result = uv_listen((uv_stream_t*)&server, DEFAULT_BACKLOG, on_new_connection)) != 0) {
		neroshop::print("uv_listen error: " + std::string(uv_strerror(result)), 1);
		return 1;
	}
	return uv_run(loop, UV_RUN_DEFAULT);
}
