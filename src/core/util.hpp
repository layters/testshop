#pragma once

#ifndef UTIL_HPP_NEROSHOP
#define UTIL_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#include <QUuid>
#else
#include <uuid.h>
//#include <catch.hpp>
#endif

#ifdef _WIN32
#include <windows.h>
#include <Lmcons.h> // UNLEN
#endif

#ifdef __gnu_linux__
#include <unistd.h> // getcwd, getwd, get_current_dir_name, getpwnam, getpwnam_r, getpwuid, getpwuid_r
#include <sys/types.h> // ??
#include <dirent.h> // opendir, fdopendir, closedir
#include <pwd.h> // getpwnam, getpwnam_r, getpwuid, getpwuid_r
#include <sys/stat.h> // mkdir
#include <string.h> // strdup
#endif

#include <iostream>
#include <sstream> // std::ostringstream
#include <vector>
#include <algorithm> // std::transform, std::remove
#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <filesystem> // std::filesystem
#endif
#include <cassert> // assert
#include <fstream>

namespace neroshop {

namespace string {
	static std::string lower(const std::string& str) 
	{
		std::string temp_str = str;
		std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), [](unsigned char c){ return std::tolower(c); });	
		return temp_str;
	}
	
	static std::string upper(const std::string& str)
	{
		std::string temp_str = str;
		std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), [](unsigned char c){ return std::toupper(c); });	
		return temp_str;		
	}    
	
    template <typename T>
    static std::string precision(const T value, const int n)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << value;
        return out.str();
    } 		
    
	static std::vector<std::string> split(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> output;
        char * dup = strdup(str.c_str());
        char * token = strtok(dup, delimiter.c_str());
        while(token != nullptr)
		{
            output.push_back(std::string(token));
            token = strtok(nullptr, delimiter.c_str());
        }
        free(dup);	
        return output;		
	}
		
	static bool contains(const std::string& str, const std::string& what) {
		return (str.find(what) != std::string::npos);
	}	
	static std::string swap_first_of(const std::string& str, const std::string& from, const std::string& to) // replaces first occurance of a word from a String with another
	{
		std::string string0 (str);
		size_t start = string0.find(from); // find location of the first 'from'
		if(start == std::string::npos)
			return "error"; // error
		return string0.replace(start, from.length(), to); // from location of 'from' to the length of 'from', replace with 'to'
	}
	static std::string swap_last_of(const std::string& str, const std::string& from, const std::string& to) // replace last occurance of a word from a String with another
	{
		std::string string0 (str);
		size_t start = string0.rfind(from); // find location of the last 'from'
		if(start == std::string::npos)
			return "error"; // error
		return string0.replace(start, from.length(), to); // from location of 'from' to the length of 'from', replace with 'to'
	}	
	static std::string swap_all(const std::string& str, const std::string& from, const std::string& to)
	{
		std::string string0 (str);
		while (string0.find(from) != std::string::npos) // while String contains 'from'
			string0.replace(string0.find(from), from.length(), to); // replace all occurances of 'from' (first-to-last)
		return string0;
	}	
	static bool starts_with(const std::string& str, const std::string& what, bool case_sensative = true) {
	    std::string first_word = str.substr(0, str.find_first_of(" "));
	    if(!case_sensative) {
	        return (lower(first_word) == lower(what));
	    }
	    return (first_word == what);
	}
	static std::string trim_left(const std::string& str) {
        const std::string white_spaces(" \f\n\r\t\v");
        std::string temp_str(str);
        std::string::size_type pos = temp_str.find_first_not_of(white_spaces);
        temp_str.erase(0, pos);
        return temp_str;
    }
    static std::string trim_right(const std::string& str) {
        const std::string white_spaces(" \f\n\r\t\v");
        std::string temp_str(str);
        std::string::size_type pos = temp_str.find_last_not_of(white_spaces);
        temp_str.erase(pos + 1);
        return temp_str;
    }
    static std::string trim(const std::string& str) {
        return trim_left(trim_right(str));
    }
    static std::string join(const std::vector<std::string>& string_list, std::string delimeter = ",") {
        std::stringstream ss;
        std::copy(string_list.begin(), string_list.end() - 1, std::ostream_iterator<std::string>(ss, delimeter.c_str()));
        ss << string_list.back();
        return ss.str();
    }
}
//-------------------------
namespace filesystem {
    static bool is_file(const std::string& filename) { // checks if file exists
        return std::filesystem::is_regular_file(filename);
    }
    
    static bool is_directory(const std::string& path) {
        #if defined(__cplusplus) && (__cplusplus >= 201703L)
        return std::filesystem::is_directory(path);
        #endif
        #ifdef _WIN32
	    DWORD dwAttrib = GetFileAttributes(path.c_str());
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
        #endif
        #ifdef __gnu_linux__
        DIR * dir = opendir(path.c_str());
        if(dir) {
            closedir(dir);
	        return true;
        }		
        #endif
        return false;
    }
    
    static bool make_directory(const std::string& path) {
        if(is_directory(path)) {
            std::cout << "\033[1;93mDirectory \"" << path << "\" already exists\033[0m" << std::endl;
            return false;
        }
        #if defined(__cplusplus) && (__cplusplus >= 201703L)
        return std::filesystem::create_directories(path.c_str());
        #endif
        #ifdef _WIN32
	    return (CreateDirectory(path.c_str(), nullptr) != 0);
        #endif
        #ifdef __gnu_linux__
	    return (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
        #endif	
        return false;
    }
    
    static std::string current_directory() {    
        #if defined(__cplusplus) && (__cplusplus >= 201703L)
        return std::filesystem::current_path();
        #endif
        #ifdef _WIN32
        char buffer[1024/*MAX_PATH*/];
        std::string path = std::string(buffer, GetModuleFileName(NULL, buffer, 1024)); // returns "E:\a.exe"
        return path.substr(0, path.find_last_of("\\/"));
        #endif
        #ifdef __gnu_linux__
	    char buffer[1024];
        if(getcwd(buffer, sizeof(buffer)) != nullptr)
		    return std::string(buffer);
        #endif            
        return "";
    }
    static std::vector<std::string> get_directory(const std::string& path, std::string filter) {return {};} // return a list of filenames in a directory
    // filename string manipulation
    /*static std::string get_file_extension(const std::string& filename) {
	    std::string extension = filename.substr(filename.find_last_of(".") + 1);
	    return extension;    
    }*/
}
//-------------------------
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
        std::vector<std::string> os_filenames = neroshop::string::split(result, "\n");
        if(os_filenames.empty()) return "linux"; // defaults to linux
        // check for lsb-release and os-release first
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/lsb-release") != os_filenames.end()) {}
        if(std::find(os_filenames.begin(), os_filenames.end(), "/etc/os-release") != os_filenames.end()) { // https://www.linux.org/docs/man5/os-release.html
            std::ifstream os_file("/etc/os-release");
            if(!os_file.good()) return "linux"; // defaults to linux
            std::stringstream os_stream;
            os_stream << os_file.rdbuf(); // dump file content into stringstream//std::cout << os_stream.str() << std::endl;
            std::vector<std::string> os_content = neroshop::string::split(os_stream.str(), "\n"); // split each newline
            // ID= will give us the current OS name
            // TODO: re-do this function. I keep getting VERSION_ID="22.04" instead of ID=ubuntu as a result -_-
            for(int i = 0; i < os_content.size(); i++) {
                if(os_content[i].find("ID=") != std::string::npos) {
                    os_content[i] = neroshop::string::swap_first_of(os_content[i], "ID=", ""); // remove ID=
                    std::string os_name = os_content[i];//neroshop::string::remove_all(os_content[i], "\"");// remove quotes
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
        std::vector<std::string> os_filenames = neroshop::string::split(result, "\n");
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
            std::vector<std::string> os_content = neroshop::string::split(os_stream.str(), "\n"); // split each newline
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
//--------------------------
namespace uuid {
    static std::string generate() {
        std::string uuid_out = "";
        #if defined(NEROSHOP_USE_QT)
        QString quuid = QUuid::createUuid().toString();
        quuid = quuid.remove("{").remove("}"); // remove brackets
        
        uuid_out = quuid.toStdString();
        #else
        // Creating a new UUID with a default random generator
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        uuids::uuid_random_generator gen{generator};

        uuids::uuid const id = gen();
        assert(!id.is_nil());
        assert(id.as_bytes().size() == 16);
        assert(id.version() == uuids::uuid_version::random_number_based);
        assert(id.variant() == uuids::uuid_variant::rfc);
    
        uuid_out = uuids::to_string(id);
        #endif
        return uuid_out;
    }
}

}

#endif
