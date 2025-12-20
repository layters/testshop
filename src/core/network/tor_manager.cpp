#include "tor_manager.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>   // socket, connect, getsockopt, sockaddr, SOL_SOCKET, SO_ERROR
#include <netinet/in.h>   // sockaddr_in, htons, AF_INET
#include <arpa/inet.h>    // inet_addr
#include <unistd.h>       // close
#include <fcntl.h>        // fcntl, F_GETFL, F_SETFL, O_NONBLOCK
#include <errno.h>        // errno
#include <string.h>       // memset (if you use it)
#include <signal.h>
#endif

#include <thread>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <regex>
#include <cassert> // assert

namespace neroshop {

//-----------------------------------------------------------------------------

TorManager::TorManager(uint16_t socks_port) : socks_port(socks_port), control_port(socks_port + 1) {
    base_dir = neroshop::get_default_config_path();
    hs_dir = neroshop::get_hidden_service_dir_path();
    data_dir = neroshop::get_default_tor_path() / "data";
    torrc_path = (neroshop::get_default_tor_path() / TOR_TORRC_FILENAME).string();
    set_socks_port(socks_port); // Initializes both socks_port and control_port
}

//-----------------------------------------------------------------------------

TorManager::~TorManager() {
    if (is_running) stop_tor();
}

//-----------------------------------------------------------------------------

static bool is_file_empty(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file) {
        // File doesn't exist or can't be opened, treat as empty
        return true;
    }
    return (file.peek() == std::ifstream::traits_type::eof());
}

//-----------------------------------------------------------------------------

void TorManager::create_torrc(const std::string& torrc_path, const std::string& hidden_service_dir, uint16_t hidden_service_port) {
    // If torrc already exists, exit function
    if (std::filesystem::exists(torrc_path)) {
        std::cout << "[TorManager]: Found torrc: " << torrc_path << "\n";
        return;
    }
    
    // Check if file is empty to determine whether to open it in truncate mode or append mode (in case we want to add more hidden services)
    bool file_empty = is_file_empty(torrc_path);
    std::ofstream torrc_file(torrc_path, file_empty ? std::ios::out : std::ios::app);
    if (!torrc_file) {
        throw std::runtime_error("Failed to open torrc file for writing");
    }
    
    // If file is not empty, add a newline before appending new hidden service
    if (!file_empty) {
        torrc_file << "\n";
    } else {
        torrc_file << "DataDirectory " << data_dir.string() << "\n"; // ~/.config/neroshop/tor/data
        torrc_file << "SocksPort 127.0.0.1:" << socks_port << "\n"; // For outgoing connections
        torrc_file << "ControlPort 127.0.0.1:" << control_port << "\n"; // Optional: for control (GETINFO queries, etc.)
        torrc_file << "CookieAuthentication 1\n"; // ~/.config/neroshop/tor/control_auth_cookie ("locks" the control port)
        torrc_file << "Log notice stdout\n";////torrc_file << "Log notice file " << (data_dir / "tor.log").string() << "\n";
        torrc_file << "\n"; // For readability and separating each local peer's hidden service
    }
    torrc_file << "HiddenServiceDir " << hidden_service_dir << "\n";
    torrc_file << "HiddenServicePort " << hidden_service_port 
        << " 127.0.0.1:" << hidden_service_port << "\n";
    // For second neroshopd instance
    auto hidden_service_dir1 = neroshop::get_hidden_service_dir_path(1);
    uint16_t hidden_service_port1  = (TOR_HIDDEN_SERVICE_PORT + 1);
    torrc_file << "\n";
    torrc_file << "HiddenServiceDir " << hidden_service_dir1 << "\n";
    torrc_file << "HiddenServicePort " << hidden_service_port1 
        << " 127.0.0.1:" << hidden_service_port1 << "\n";

    torrc_file.close();
    
    // Start Tor daemon with the following command and args: ./tor -f ~/.config/neroshop/tor/torrc
}

//-----------------------------------------------------------------------------

void TorManager::start_tor() {
    // If external tor is running, don't start tor again
    if(is_external_tor_running()) {
        std::cout << "[TorManager]: External Tor detected on port " << socks_port << "\n";
        // Cannot set 'is_running' to true since we don't own this process
        external_tor = true;
        // Get hostname ('.onion' address)
        hs_dir = neroshop::get_hidden_service_dir_path(1); // Switch to hidden_service_1
        std::cout << hs_dir.string() << "\n";
        auto hostname_path = hs_dir / "hostname";
        if (std::filesystem::exists(hostname_path) && 
            std::filesystem::is_regular_file(hostname_path)) {
            std::ifstream file(hostname_path.string());
            std::getline(file, onion_address);
        } else {
            std::cout << "[TorManager]: No existing hostname found for external Tor\n";
            onion_address = "";
        }
        // Don't store bootstrap progress as we don't own this process
        return; // Exit function without starting tor
    }

    std::string tor_path = base_dir + "/tor";
    #ifdef _WIN32
    std::string program = tor_path + "/" + "tor.exe";
    #else
    std::string program = tor_path + "/" + "./tor";
    #endif
    
    // If tor binary is not found, try current_dir
    if (!std::filesystem::exists(program)) {
        #ifdef _WIN32
        program = "tor/tor.exe";
        #else
        program = "tor/./tor";
        #endif
        if (!std::filesystem::exists(program)) {
            throw std::runtime_error("Tor binary not found");
        }
    }
    tor_binary = program;
    
    // Create 'data' folder BEFORE writing torrc (Tor creates HiddenServiceDir automatically)
    std::filesystem::create_directories(data_dir);

    // Create torrc
    create_torrc(torrc_path, hs_dir.string(), TOR_HIDDEN_SERVICE_PORT);
    
    // Start Tor
    const char* command[] = {tor_binary.c_str(), "-f", torrc_path.c_str(), nullptr};
    //struct subprocess_s tor_proc;

    if (subprocess_create(command,
            subprocess_option_no_window | subprocess_option_enable_async |
            subprocess_option_combined_stdout_stderr, &tor_proc) != 0) {
        throw std::runtime_error("Failed to start Tor process");
    }
    
    is_running = true;
    
    // Track the bootstrap progress so we can store it
    // Read stdout in background thread
    std::thread progress_thread([this]() {
        FILE* stdout_pipe = subprocess_stdout(&tor_proc);
        if (!stdout_pipe) return;
        char buffer[2048];
        while (fgets(buffer, sizeof(buffer), stdout_pipe)) {
            printf("[Tor] %s", buffer);
            if (strstr(buffer, "Bootstrapped")) {
                const char* boot_pos = strstr(buffer, "Bootstrapped ");
                if (boot_pos) {
                    int progress;
                    if (sscanf(boot_pos, "Bootstrapped %d%%", &progress) == 1) {
                        bootstrap_progress.store(progress);
                    }
                }
            }
        }
    });
    progress_thread.detach();
    
    // Now we need to check if the hidden service folder is created and retrieve the onion address from the 'hostname' file
    read_hostname(); // Still blocks for onion
}

//-----------------------------------------------------------------------------

void TorManager::stop_tor() {
    if (!is_running) return;
    
    // Terminate process
    subprocess_terminate(&tor_proc);
    
    // Wait for it to finish
    int exit_code;
    subprocess_join(&tor_proc, &exit_code);
    
    subprocess_destroy(&tor_proc);
    is_running = false;
    onion_address.clear();
    bootstrap_progress.store(0);  // Reset progress
    std::cout << "[TorManager]: Tor stopped. Progress reset to " << bootstrap_progress.load() << "\n";
}

//-----------------------------------------------------------------------------

void TorManager::add_hidden_service(const std::string& hidden_service_dir, uint16_t hidden_service_port) {
    // Check if file is empty to determine whether to open it in truncate mode or append mode (in case we want to add more hidden services)
    bool file_empty = is_file_empty(torrc_path);
    std::ofstream torrc_file(torrc_path, file_empty ? std::ios::out : std::ios::app);
    if (!torrc_file) {
        throw std::runtime_error("Failed to open torrc file for writing");
    }
    
    // If file is not empty, add a newline before appending new hidden service
    if (!file_empty) {
        torrc_file << "\n";
    } else {
        torrc_file << "DataDirectory " << data_dir.string() << "\n"; // ~/.config/neroshop/tor/data
        torrc_file << "SocksPort 127.0.0.1:" << socks_port << "\n"; // For outgoing connections
        torrc_file << "ControlPort 127.0.0.1:" << control_port << "\n"; // Optional: for control (GETINFO queries, etc.)
        torrc_file << "CookieAuthentication 1\n"; // ~/.config/neroshop/tor/control_auth_cookie ("locks" the control port)
        torrc_file << "Log notice stdout\n";////torrc_file << "Log notice file " << (data_dir / "tor.log").string() << "\n";
        torrc_file << "\n"; // For readability and separating each local peer's hidden service
    }
    torrc_file << "HiddenServiceDir " << hidden_service_dir << "\n";
    torrc_file << "HiddenServicePort " << hidden_service_port 
        << " 127.0.0.1:" << hidden_service_port << "\n";
        
    torrc_file.close();
    
    std::cout << "[TorManager]: Hidden service added: " << hidden_service_dir << " (127.0.0.1:" << hidden_service_port << ")\n";
    
    // Tor daemon must be restarted if previously running with the following command and args: ./tor -f ~/.config/neroshop/tor/torrc
}

//-----------------------------------------------------------------------------

void TorManager::read_hostname() {
    // Poll hostname file every 500ms, timeout after 60s
    auto hostname_path = hs_dir / "hostname";
    auto start = std::chrono::steady_clock::now();
    
    while (std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start).count() < 60) {
        
        if (std::filesystem::exists(hostname_path) && 
            std::filesystem::is_regular_file(hostname_path)) {
            
            std::ifstream file(hostname_path.string());
            std::getline(file, onion_address);
            ////onion_address = onion_address.substr(0, onion_address.find('.')); // <- removes '.onion' prefix
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    throw std::runtime_error("Tor bootstrap timeout - no onion address");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void TorManager::set_socks_port(uint16_t socks_port) {
    // If tor is already running then its too late to set the socks port
    if(is_tor_running() || external_tor) {
        std::cerr << "[TorManager]: Cannot change SocksPort while Tor is running\n";
        return;
    }
    
    if (is_socks_port_same_as_torrc(socks_port)) {
        // Ports are the same, no need to change it
        return;
    }
    
    // Ports differ, update the SocksPort
    this->socks_port = socks_port;
    control_port = socks_port + 1;
    
    // Remove old torrc file to force recreation
    if (std::filesystem::exists(torrc_path) && std::filesystem::is_regular_file(torrc_path)) {
        try {
            std::filesystem::remove(torrc_path);
            std::cerr << "[TorManager]: Deleted previous torrc file\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "[TorManager]: Delete failed: " << e.what() << "\n";
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

uint16_t TorManager::get_socks_port() const {
    return socks_port;
}

//-----------------------------------------------------------------------------

int TorManager::get_bootstrap_progress() const { 
    return bootstrap_progress.load(); 
}

//-----------------------------------------------------------------------------

std::string TorManager::get_onion_address() const {
    return onion_address;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool TorManager::is_tor_running() const {
    return is_running;
}

//-----------------------------------------------------------------------------

bool TorManager::is_tor_ready() const {
    if(external_tor) {
        assert(!onion_address.empty());
        return true;
    }
    
    return bootstrap_progress.load() >= 100;
}

//-----------------------------------------------------------------------------

bool TorManager::is_tor_external() const {
    return external_tor;
}

//-----------------------------------------------------------------------------

bool TorManager::is_socks_port_same_as_torrc(uint16_t socks_port) {
    if (!std::filesystem::exists(torrc_path) || !std::filesystem::is_regular_file(torrc_path)) {
        return false; // No file to compare
    }

    std::ifstream file(torrc_path);
    if (!file.is_open()) {
        return false; // Can't open file, assume different
    }

    std::string line;
    std::regex socks_regex(R"(^\s*SocksPort\s+[^\s:]*:?(\d+))", std::regex::icase);
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, socks_regex)) {
            // Extract port
            int current_socks_port = std::stoi(match[1]);
            #ifdef NEROSHOP_DEBUG0
            std::cout << "[TorManager] Full match: '" << match[0] << "' Port: '" << match[1] << "'\n";
            #endif
            // SocksPort in torrc matches the socks_port arg
            return current_socks_port == socks_port;
        }
    }
    return false; // SocksPort not found, assume different
}

//-----------------------------------------------------------------------------

bool TorManager::is_external_tor_running() const {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(socks_port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(fd, (sockaddr*)&addr, sizeof(addr));

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);
    timeval tv = {0, 500000};

    bool writable = (select(fd+1, nullptr, &wfds, nullptr, &tv) > 0);
    
    int so_error = 0;
    socklen_t len = sizeof(so_error);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
    
    close(fd);
    
    // FIXED: Only return true for actual TCP success
    return writable && so_error == 0;
}

//-----------------------------------------------------------------------------

} 

/*int main() {
    try {
        neroshop::TorManager tor_manager(9050);//(9052);
        tor_manager.start_tor();
        
        if(!tor_manager.is_tor_running()) {
            throw std::runtime_error("Tor failed to start");
        }
        
        // Get the onion address
        std::cout << "Onion: " << tor_manager.get_onion_address() << std::endl;
        
        // Track the bootstrap progress
        while (!tor_manager.is_tor_ready()) {
            printf("Progress: %d%%\n", tor_manager.get_bootstrap_progress());
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        // Print final progress (100%)
        printf("Progress: %d%%\n", tor_manager.get_bootstrap_progress());
        
        // Wait 10 secs before stopping Tor
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        ////tor_manager.stop_tor();
        // Tor will stop at end of scope even without calling stop_tor()
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}*/

// g++ -std=c++17 -pthread -I ../../../external/subprocess.h/ tor_manager.cpp -o torman
