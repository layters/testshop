#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace neroshop {

extern std::string base64_encode(const std::string& input);
extern std::string base64_decode(const std::string& encoded);

// untested
extern std::string base64_image_encode(const std::string& image_path);
extern std::vector<char> base64_image_decode(const std::string& encoded);
}
