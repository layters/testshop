#pragma once
#ifndef DOWNLOADER_HPP_NEROSHOP
#define DOWNLOADER_HPP_NEROSHOP

#include <iostream>
#include <string>

namespace neroshop {

namespace tools {

struct downloader {
    static void download_tor();
    static void download_i2pd();
private:
    #if !defined(NEROSHOP_USE_QT)
    static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
    #endif
};

}

}
#endif
