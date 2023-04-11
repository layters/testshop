#include "regex.hpp"

#include "../util/logger.hpp" // neroshop::print

std::regex neroshop::tools::regex::email() {
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    return pattern;
}

std::regex neroshop::tools::regex::password() {
    // Minimum eight characters, at least one uppercase letter, one lowercase letter, one number and one special character:
    const std::regex pattern("^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$"); // source: https://stackoverflow.com/questions/19605150/regex-for-password-must-contain-at-least-eight-characters-at-least-one-number-a
    return pattern;
}

/*int main() {
    auto email_pattern = neroshop::tools::regex::email();
    std::string email = "mail@neroshop.org";
    if(!std::regex_match(email, email_pattern)) {
        neroshop::print("Email address is not valid", 1);
        return 1;
    }

    auto password_pattern = neroshop::tools::regex::password();
    std::string password = "supersecretpassword123";
    if(!std::regex_match(password, password_pattern))
    {
        // Figure out the specific reason why password failed to follow the regex rules  //if(password.empty()) { std::cout << "Please enter a valid password" << std::endl; return false; }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[A-Z])"))) {
            neroshop::print("Password must have at least one upper case letter", 1);
        }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[a-z])"))) {
            neroshop::print("Password must have at least one lower case letter", 1);
        }
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[0-9])"))) {
            neroshop::print("Password must have at least one digit", 1);
        }              
        if(!std::regex_search(password.c_str(), std::regex("(?=.*?[#?!@$%^&*-])"))) {
            neroshop::print("Password must have at least one special character", 1);
        }         
        if(password.length() < 8) {
            neroshop::print("Password must be at least 8 characters long", 1);
        } 
        ////neroshop::print("Please enter a stronger password (at least 1 upper case letter, 1 lower case letter, 1 digit, and 1 special character)", 1);
        return 1;
    }
    
    return 0;
} // g++ regex.cpp -std=c++17
*/
