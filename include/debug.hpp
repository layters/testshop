// filename: debug.hpp
#ifndef DEBUG_HPP_NEROSHOP
#define DEBUG_HPP_NEROSHOP
#define NEROSHOP_TAG_OUT neroshop::io_write("");
#define NEROSHOP_TAG_IN std::string("\033[1;35;49m[neroshop]: \033[0m") +
#define NEROSHOP_TAG NEROSHOP_TAG_IN
// path
#ifdef _WIN32
#define NEROSHOP_PATH "" // haven't used windows in ages lol
#endif
#ifdef __gnu_linux__
#define NEROSHOP_PATH "/home/" + System::get_user() + "/.config/neroshop"
#endif
#include <iostream> // String(

namespace neroshop {
    ////////////////////
    inline void print(const std::string& text, int code = 0) { // 0=normal, 1=error, 2=warning, 3=success, 
        if(code == 0) std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;37;49m" << text << "\033[0m" << std::endl; // magenta_text_bold
        if(code == 1) std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;91;49m" << text << "\033[0m" << std::endl;
        if(code == 2) std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;33;49m" << text << "\033[0m" << std::endl;
        if(code == 3) std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;32;49m" << text << "\033[0m" << std::endl;
        if(code == 4) std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;34;49m" << text << "\033[0m" << std::endl;
    }    
    ////////////////////
    inline void io_write(const std::string& text) {// like print but without a newline
        std::cout << "\033[1;35;49m" << "[neroshop]: " << "\033[1;37;49m" << text << "\033[0m";// << std::endl; // magenta_text_bold
    }
    ////////////////////
} // inline - this tells the compiler you are deliberately repeating the function definition all over the place, and you'd better be sure the definition is always identical.
#endif
