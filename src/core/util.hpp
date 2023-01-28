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
#include <algorithm> // std::transform
#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <filesystem> // std::filesystem
#endif
#include <cassert> // assert

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
        #endif
        #ifdef __gnu_linux__
	    char buffer[1024];
        if(getcwd(buffer, sizeof(buffer)) != nullptr)
		    return std::string(buffer);
        #endif            
        return "";
    }
    //static std::vector<std::string> get_directory() { // return a list of filenames in a directory
    // temporary
    static std::vector<std::string> get_dir(const std::string& path, std::string filter) {
    
	std::vector<std::string> file;/*
#ifdef _WIN32
	filter = "%s/" + filter;
	char search_path[200];
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
    sprintf_s(search_path, 200, filter.c_str(), path.c_str());
#else
    sprintf(search_path, filter.c_str(), path.c_str());
#endif
	WIN32_FIND_DATA fd; 
    HANDLE hFind = FindFirstFile(search_path, &fd); 
    if(hFind != INVALID_HANDLE_VALUE) 
	{
        do 
		{ 
            if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
			{
			    // store module names
                file.push_back(fd.cFileName);
            } 
        } while(FindNextFile(hFind, &fd)); 
        FindClose(hFind); 
    } 
#endif
#ifdef __gnu_linux__
	DIR *dir = opendir (path.c_str());
    struct dirent * ent;
    if (dir != nullptr) 
	{
        while ((ent = readdir (dir)) != nullptr)   // print all the files and directories within directory 
        {
		#ifdef DOKUN_DEBUG0
            std::cout << ent->d_name << std::endl;
		#endif
		    if(!String::contains(ent->d_name, "." + String::remove_all(File::extension(filter), "*"))) // if filename does not contain the extenstion from filter
				continue;
			file.push_back(ent->d_name);
        }
        closedir (dir);
    }
	#endif*/
    return file;		
}    
    //static dialog() {
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
