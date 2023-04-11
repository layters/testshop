#pragma once

#include <iostream>
#include <string>

namespace neroshop {

namespace crypto {
    std::string sha256(const std::string& plain_text);
}

}
