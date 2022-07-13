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
#include "server.hpp" // server // https://www.linuxhowtos.org/C_C++/socket.htm
#include "debug.hpp"
//#include "db2.hpp" // daemon should handle database server requests from the client ??
#define NEROMON_TAG std::string("\033[1;95m[neromon]:\033[0m ") +

using namespace neroshop;

Server * server = new Server();

void close_server() {
    server->shutdown();
    delete server; // calls destructor which calls closesocket()
    server = nullptr;
    std::cout << NEROMON_TAG "\033[1;91mdisconnected\033[0m" << std::endl;
}
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
    // Define variables
    pid_t pid, sid;
    // Fork the current process
    pid = fork();
    // The parent process continues with a process ID greater than 0
    if(pid > 0) {
        return EXIT_SUCCESS;
    }
   // A process ID lower than 0 indicates a failure in either process
    else if(pid < 0) {
        return EXIT_FAILURE;
    }
    // The parent process has now terminated, and the forked child process will continue
    // (the pid of the child process was 0)
    // Since the child process is a daemon, the umask needs to be set so files and logs can be written
    umask(0);
    // Open system logs for the child process
    openlog("neromon", LOG_NOWAIT | LOG_PID, LOG_USER);
    syslog(LOG_NOTICE, "Successfully started neromon");
    // Generate a session ID for the child process
    sid = setsid();
    // Ensure a valid SID for the child process
    if(sid < 0) {
        // Log failure and exit
        syslog(LOG_ERR, "Could not generate session ID for child process");
        // If a new session ID could not be generated, we must terminate the child process
        // or it will be orphaned
        exit(EXIT_FAILURE);
    }
    // Change the current working directory to a directory guaranteed to exist
    if((chdir("/")) < 0) {
        // Log failure and exit
        syslog(LOG_ERR, "Could not change working directory to /");
        // If our guaranteed directory does not exist, terminate the child process to ensure
        // the daemon has not been hijacked
        return EXIT_FAILURE;
    }
    // A daemon cannot use the terminal, so close standard file descriptors for security reasons
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // Daemon-specific intialization should go here
    const int SLEEP_INTERVAL = 1;
    //////////////////////////////////////////////////
    // Start server
    std::atexit(close_server);
    int server_port = 1234;//(std::stoi(port));//port 38081 fails
	if(server->bind(server_port)) {
	    std::cout << std::endl << NEROMON_TAG "\033[1;97mbound to port " + std::to_string(server_port) << "\033[0m" << std::endl;
	}
	server->listen(); // listens for any incoming connection
    // Enter daemon loop
    while(1)
    {
        // Execute daemon heartbeat, where your recurring activity occurs
        do_heartbeat();
        // Sleep for a period of time (in seconds)// sleep for a while in case a new client connects
        sleep(SLEEP_INTERVAL);
    }
    // Close system logs for the child process
    syslog(LOG_NOTICE, "Stopping neroshop-daemon");
    closelog();
    // Terminate the child process when the daemon completes
    return EXIT_SUCCESS;	
}
