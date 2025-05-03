#pragma once

#include <iostream>
#include <string>

namespace neroshop {

struct Image {
    std::string name;
    size_t size;
    unsigned int id = 0; // The specific order in which images are displayed
    std::string source;
    std::vector<std::string> pieces;
    size_t piece_size;
    std::vector<unsigned char> data;
    unsigned int width = 0;
    unsigned int height = 0;
};

}
