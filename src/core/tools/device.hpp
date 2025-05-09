#pragma once

#include "string.hpp"

#include <fstream>

#ifdef __gnu_linux__
#include <pwd.h> // getpwnam, getpwnam_r, getpwuid, getpwuid_r
#include <unistd.h> // geteuid, getcwd, getwd, get_current_dir_name, getpwnam, getpwnam_r, getpwuid, getpwuid_r
#endif

#ifdef _WIN32
#include <windows.h>
#include <Lmcons.h> // UNLEN
#endif

namespace neroshop {

namespace device {
    static std::string get_user() {
	    #ifdef _WIN32
	    char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        if(GetUserName(username, &username_len) == 0) 
            return "";
		return std::string(username);
	    #endif    
	    #ifdef __gnu_linux__ // works!
	    uid_t uid = geteuid();
		struct passwd * pw = getpwuid(uid);
        if(!pw) return "";
        return std::string(pw->pw_name);
	    #endif    
	    return "";
	}
	// NOTE: the get_*distro functions are experimental and may not work as intended
    // current_distro - returns the name of the current distro
    static inline std::string get_current_distro() {
    #ifdef __gnu_linux__
        // get output from command
        FILE* pipe = popen("ls /etc/*release; ls /etc/*version", "r"); // &&
        if (!pipe) return "ERROR";
        char buffer[128];
        std::string result = "";
        while(!feof(pipe)) {
            if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        // save release filenames
        std::vector<std::string> os_filenames = neroshop::string_tools::split(result, "\n");
        if(os_filenames.empty()) return "linux"; // defaults to linux
        // check for lsb-release and os-release first
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/lsb-release") != os_filenames.end()) {}
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/os-release") != os_filenames.end()) { // https://www.linux.org/docs/man5/os-release.html
            std::ifstream os_file("/etc/os-release");
            if(!os_file.good()) return "linux"; // defaults to linux
            std::stringstream os_stream;
            os_stream << os_file.rdbuf(); // dump file content into stringstream//std::cout << os_stream.str() << std::endl;
            std::vector<std::string> os_content = neroshop::string_tools::split(os_stream.str(), "\n"); // split each newline
            // ID= will give us the current OS name
            // TODO: re-do this function. I keep getting VERSION_ID="22.04" instead of ID=ubuntu as a result -_-
            for(int i = 0; i < os_content.size(); i++) {
                if(os_content[i].find("ID=") != std::string::npos) {
                    os_content[i] = neroshop::string_tools::swap_first_of(os_content[i], "ID=", ""); // remove ID=
                    std::string os_name = os_content[i];//neroshop::string_tools::remove_all(os_content[i], "\"");// remove quotes
                    return os_name;
                }            
            }
        }
    #endif
        return "";
    }
    // base_distro - returns the name of the major distro in which the current distro is based off
    static inline std::string get_base_distro() {
    #ifdef __gnu_linux__
        // the ls /etc/*release command should list all the release files on the system
        // ls /etc/*version for some other distros 
        // get output from command
        FILE* pipe = popen("ls /etc/*release; ls /etc/*version", "r"); // &&
        if (!pipe) return "ERROR";
        char buffer[128];
        std::string result = "";
        while(!feof(pipe)) {
            if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        // save release filenames
        std::vector<std::string> os_filenames = neroshop::string_tools::split(result, "\n");
        if(os_filenames.empty()) return "linux"; // defaults to linux
    #ifdef NEROSHOP_DEBUG
        std::cout << "os_filenames " << "(" << os_filenames.size() << "): " << std::endl;
        for(int i = 0; i < os_filenames.size(); i++) std::cout << os_filenames[i] << std::endl;
    #endif
        // check if vector contains a specific release filename - https://serverfault.com/questions/422880/etc-release-files-examples
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/debian_version") != os_filenames.end()) return "debian";
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/arch-release") != os_filenames.end()) return "arch"; // or Arch Linux
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/fedora-release") != os_filenames.end()) return "fedora";
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/SuSE-release") != os_filenames.end()) return "suse"; // or SUSE Linux // deprecated: https://en.opensuse.org/Etc_SuSE-release
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/redhat-release") != os_filenames.end()) return "rhel";//return "fedora"; // or Red Hat Enterprise Linux (RHEL)
        //if(std::find(os_filenames.begin(), os_filenames.end(), "") != os_filenames.end()) return "";
        // fallback to lsb-release and os-release, if all else fails
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/lsb-release") != os_filenames.end()) {}
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/os-release") != os_filenames.end()) { // https://www.linux.org/docs/man5/os-release.html
            std::ifstream os_file("/etc/os-release");
            if(!os_file.good()) return "linux"; // defaults to linux
            std::stringstream os_stream;
            os_stream << os_file.rdbuf(); // dump file content into stringstream//std::cout << os_stream.str() << std::endl;
            std::vector<std::string> os_content = neroshop::string_tools::split(os_stream.str(), "\n"); // split each newline
            // ID_LIKE= will give us the OS that the current OS is based on and ID= will give us the current OS
            for(int i = 0; i < os_content.size(); i++) {
                // Debian and Debian-based distros (e.g Ubuntu)
                if(os_content[i].find("ID=debian") != std::string::npos) return "debian"; // ID=debian
                if(os_content[i].find("ID_LIKE=debian") != std::string::npos) return "debian"; // ID=ubuntu
                // Ubuntu-based distros (e.g Linux Mint)
                if(os_content[i].find("ID_LIKE=ubuntu") != std::string::npos) return "debian"; //return "ubuntu";// ID=linuxmint
                // Arch and Arch-based distros
                if(os_content[i].find("ID=arch") != std::string::npos) return "arch";
                if(os_content[i].find("ID_LIKE=arch") != std::string::npos) return "arch"; // ID=manjaro
                // Fedora and Fedora-based distros (e.g RHEL)
                if(os_content[i].find("ID=fedora") != std::string::npos) return "fedora";
                if(os_content[i].find("ID_LIKE=fedora") != std::string::npos) return "fedora";
                if(os_content[i].find("ID_LIKE=\"fedora\"") != std::string::npos) return "fedora"; // ID="rhel"
                // Red Hat Enterprise Linux (RHEL)-based distros
                if(os_content[i].find("ID_LIKE=\"rhel fedora\"") != std::string::npos) return "fedora";//return "rhel"; // ID="centos"
                // SUSE and SUSE-based distros (e.g OpenSUSE)
                if(os_content[i].find("ID=suse") != std::string::npos) return "suse";
                if(os_content[i].find("ID_LIKE=\"suse\"") != std::string::npos) return "suse"; // ID=opensuse
                // OpenSUSE-based distros
                if(os_content[i].find("ID_LIKE=\"suse opensuse\"") != std::string::npos) return "suse";//return "opensuse"; // ID="opensuse-leap", ID="opensuse-tumbleweed", ID="opensuse-tumbleweed-kubic"
                /*// _ and _-based distros
                if(os_content[i].find("ID=") != std::string::npos) return "";
                if(os_content[i].find("ID_LIKE=") != std::string::npos) return "";*/
            }
        }        
    #endif
        return "";
    }
    // operating system
    static std::string get_os() {
        std::string platform;
        #if defined(__linux__) && !defined(__ANDROID__)
        platform = "linux";
        #endif
        #if defined(_WIN32)
        platform = "windows";
        #endif
        #if defined(__APPLE__) && defined(__MACH__)
        platform = "macos";
        #endif
        #if defined(__ANDROID__)
        platform = "android";
        #endif
        #if defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE
        platform = "iphone"; // ios
        #endif
        #endif    
        return platform;
    }
    // architecture
    static std::string get_architecture() {
        std::string arch;
        #if defined(__i686__)
        arch = "i686";
        #endif    
        #if defined(__x86_64__) || defined(__amd64__)
        arch = "x86_64";
        #endif
        #if defined(__arm__)
        #if defined(__ARM_ARCH_5__)
        arch = "armv5";
        #endif
        #if defined(__ARM_ARCH_6__)
        arch = "armv6";
        #endif
        #if defined(__ARM_ARCH_7__)
        arch = "armv7";
        #endif
        #endif
        #if defined(__powerpc64__)
        arch = "powerpc64";
        #endif        
        #if defined(__aarch64__) // arm64
        arch = "aarch64";
        #endif
        return arch;
    }
}    

}
