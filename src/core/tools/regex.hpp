#pragma once

#include <iostream>
#include <regex>
#include <string>

namespace neroshop {

namespace string_tools {

    bool is_email(const std::string& email);
    bool is_strong_password(const std::string& password);
    
    bool is_product_code(const std::string& code);
}

}
