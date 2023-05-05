#include "regex.hpp"

#include "../tools/logger.hpp" // neroshop::print

bool neroshop::string_tools::is_email(const std::string& email) {
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    return std::regex_match(email, pattern);
}

bool neroshop::string_tools::is_strong_password(const std::string& password) {
    // Minimum eight characters, at least one uppercase letter, one lowercase letter, one number and one special character:
    const std::regex pattern("^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$"); // source: https://stackoverflow.com/questions/19605150/regex-for-password-must-contain-at-least-eight-characters-at-least-one-number-a
    return std::regex_match(password, pattern);
}

// untested
bool is_product_code(const std::string& code) {
    // Define regular expressions for each product code type
    std::regex upc("^\\d{12}$");
    std::regex ean("^\\d{13}$");
    std::regex jan("^49\\d{10}$");
    std::regex isbn("^(\\d{9}(\\d|X)|978\\d{10}|979\\d{10})$"); // validates both ISBN-10 and ISBN-13 codes
    std::regex issn("^\\d{8}$");
    std::regex gtin("^\\d{8,14}$");
    std::regex sku("^\\w+$"); // Assumes SKU can contain alphanumeric characters
    std::regex mpn("^\\w+$"); // Assumes MPN can contain alphanumeric characters
    std::regex ndc("^\\d{10}$");
    
    // Check if the code matches any of the regular expressions
    return std::regex_match(code, upc) || std::regex_match(code, ean) || std::regex_match(code, isbn) || 
           std::regex_match(code, issn) || std::regex_match(code, gtin) || std::regex_match(code, sku) ||
           std::regex_match(code, mpn) || std::regex_match(code, ndc);
}

/*int main() {
    std::string email = "mail@neroshop.org";
    if(!neroshop::string_tools::is_email(email)) {
        neroshop::print("Email address is not valid", 1);
        return 1;
    }

    std::string password = "supersecretpassword123";
    if(!neroshop::string_tools::is_strong_password(password))
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
} // g++ regex.cpp logger.cpp -std=c++17 -I.
*/
