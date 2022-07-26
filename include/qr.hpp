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
    QR(std::string fileName, int imgSize, std::string text,
            bool overwriteExistingFile, QrCode::Ecc ecc);
    ~QR();
    bool to_png() const;
    bool write_to_png(const QrCode& qr_data, const int& multiplicator) const;
    unsigned int image_size(const QrCode& qr_data) const;
    unsigned int image_size_with_border(const QrCode& qr_data) const;
private:
    std::string filename;
    int size;
    std::string text;
    bool overwrite_existing_file;
    QrCode::Ecc ecc; // to-do: change this to a unique_ptr
};
}
#endif
