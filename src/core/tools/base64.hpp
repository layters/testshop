#pragma once

#include <iostream>
#include <string>

namespace neroshop {

extern std::string base64_encode(const std::string& input);
extern std::string base64_decode(const std::string& encoded);

}
