#include <iostream>
#include <string>
// neroshop
#include "../src/neroshop.hpp"
using namespace neroshop;


int main() {

    std::cout << neroshop::string::precision("1.123456789", 9) << std::endl;
    std::cout << neroshop::string::lower("LOWER") << std::endl;
    std::cout << neroshop::string::upper("upper") << std::endl;
    
    #if defined(__cplusplus) && (__cplusplus >= 201402L) // 14
    std::cout << "This project is using C++ 14\n";
    #endif
    #if defined(__cplusplus) && (__cplusplus >= 201703L) // 17
    std::cout << "This project is using C++ 17\n";
    #endif
    std::cout << "current dir: " << neroshop::filesystem::current_directory() << std::endl;
    std::cout << "mkdir result: " << neroshop::filesystem::make_directory("dude") << std::endl;
    std::cout << "get_user result: " << neroshop::device::get_user() << std::endl;
    //std::cout << "filename result: " << neroshop::filesystem::get_file_extension("player.png") << std::endl;
    //-------------------------
    return 0;
}
