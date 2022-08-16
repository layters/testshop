#ifndef QR_HPP_NEROSHOP
#define QR_HPP_NEROSHOP

#include <qrcodegen.hpp>
using std::uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;
#include <png.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "debug.hpp"

/*
Use:
neroshop::QR qr("qr.png", 400, "888tNkZrPN6JsEgekjMnABU4TBzc2Dt29EPAvkRxbANsAnjyPbb3iQ1YBRk1UXcdRsiKc9dhwMVgN5S9cQUiyoogDavup3H", true, QrCode::Ecc::LOW);
qr.to_png();
*/

namespace neroshop {
class QR {
public:
    QR();
    QR(const std::string& filename, int image_size, const std::string& text,
            bool overwrite, qrcodegen::QrCode::Ecc ecc);
    ~QR();
    bool to_png() const;
    // getters
    unsigned char * get_data() const; // pixel_data
    std::string get_text() const;
    unsigned int get_size() const; // image_size //unsigned int get_size_with_border() const;
    std::string get_file() const; // file_name
    // static
    static bool export_png(const std::string& filename, int size, const std::string& text, bool overwrite, QrCode::Ecc ecc);
private:
    bool write_png(const QrCode& qr_data, const int& multiplicator) const;
    std::string filename;
    int size;
    std::string text;
    bool overwrite;
    QrCode::Ecc ecc; // to-do: change this to a unique_ptr
};
}
#endif
