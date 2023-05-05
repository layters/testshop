#pragma once

#include <iostream>
#include <string>

namespace neroshop {

namespace tools {

namespace extractor {
    void extract_tar(const std::string& filename);
    void extract_zip(const std::string& filename);
}

}

}
