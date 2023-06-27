#pragma once

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <filesystem> // std::filesystem
#endif
#include <iostream>
#include <vector>

#ifdef __gnu_linux__
#include <unistd.h> // getcwd, getwd, get_current_dir_name, getpwnam, getpwnam_r, getpwuid, getpwuid_r
#include <sys/stat.h> // mkdir
#include <dirent.h> // opendir, fdopendir, closedir
#endif

#ifdef _WIN32
#include <windows.h>
#include <Lmcons.h> // UNLEN
#endif

namespace neroshop {

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
            return true;
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
} // g++ filesystem.hpp

}
