#pragma once

#include <iostream>
#include <string>

namespace neroshop {

struct Image {
    std::string name; // base name + ext
    size_t size; // bytes
    unsigned int id = 0; // The specific order in which images are displayed
    std::string source; // never stored in DHT
    std::vector<std::string> pieces;
    size_t piece_size;
    unsigned int width = 0;
    unsigned int height = 0;
};

}
