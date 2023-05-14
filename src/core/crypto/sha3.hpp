#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace neroshop {

namespace crypto {
    std::string sha3_256(const std::string& plain_text);
    std::string sha3_256(const std::vector<uint8_t>& data);
    std::string sha3_512(const std::string& plain_text);
}

}
