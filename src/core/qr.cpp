#include "qr.hpp"

neroshop::QR::QR() {}
////////////////////
neroshop::QR::QR(const std::string& filename, int image_size, const std::string& text,
            bool overwrite, qrcodegen::QrCode::Ecc ecc) : QR() {
    this->filename = filename;
    this->size = image_size;
    this->text = text;
    this->overwrite = overwrite;
    this->ecc = ecc;
}
////////////////////
neroshop::QR::~QR() {}
////////////////////    
////////////////////    
bool neroshop::QR::to_png() const {
    if(text.empty()) return false; 
    std::ifstream file(filename.c_str());
    bool exists = file.good();
    file.close();
    if(!overwrite && exists) return false; 
    
    auto qr_code = qrcodegen::QrCode::encodeText(text.c_str(), ecc);

    int multiplicator = size / qr_code.getSize();

    auto result = write_png(qr_code, multiplicator);
    
    return result;
}
////////////////////
bool neroshop::QR::write_png(const QrCode& qr, const int& multiplicator) const {
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	// Open file for writing (binary mode)
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		neroshop::print("Could not open file " + filename + " for writing", 1);
		return false;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		neroshop::print("Could not allocate write struct", 1);
		fclose(fp);
        return false;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		neroshop::print("Could not allocate info struct", 1);
		fclose(fp);
	    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        return false;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		neroshop::print("Error during png creation", 1);
		
		fclose(fp);
	    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return false;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, qr.getSize()*multiplicator, qr.getSize()*multiplicator,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel)
	row = (png_bytep) malloc(3 * multiplicator * qr.getSize() * sizeof(png_byte));

	// Write image data
    int offset_y = 0;
    int offset_x = 0;
	for (int y=0 ; y < qr.getSize() * multiplicator ; y++) {

		for (int x=0 ; x < qr.getSize() * multiplicator; x++) {
			
            if(qr.getModule(offset_x, offset_y)) {
                row[x*3] = row[x*3+1] = row[x*3+2] = 0;
            } else {
                row[x*3] = row[x*3+1] = row[x*3+2] = 255;
            }

            if (x && x % multiplicator == 0) {
                offset_x++;
            }

		}
        offset_x = 0;
        if (y && y % multiplicator == 0) {
            offset_y++;
        }
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return true;
}
////////////////////
bool neroshop::QR::export_png(const std::string& filename, int size, const std::string& text, bool overwrite, QrCode::Ecc ecc) {
    auto qr = qrcodegen::QrCode::encodeText(text.c_str(), ecc);

    int multiplicator = size / qr.getSize();
	
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	
	// Open file for writing (binary mode)
	fp = fopen(filename.c_str(), "wb");
	if (fp == NULL) {
		neroshop::print("Could not open file " + filename + " for writing", 1);
		return false;
	}

	// Initialize write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		neroshop::print("Could not allocate write struct", 1);
		fclose(fp);
        return false;
	}

	// Initialize info structure
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		neroshop::print("Could not allocate info struct", 1);
		fclose(fp);
	    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        return false;
	}

	// Setup Exception handling
	if (setjmp(png_jmpbuf(png_ptr))) {
		neroshop::print("Error during png creation", 1);
		
		fclose(fp);
	    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
        return false;
	}

	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, qr.getSize()*multiplicator, qr.getSize()*multiplicator,
			8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	// Allocate memory for one row (3 bytes per pixel)
	row = (png_bytep) malloc(3 * multiplicator * qr.getSize() * sizeof(png_byte));

	// Write image data
    int offset_y = 0;
    int offset_x = 0;
	for (int y=0 ; y < qr.getSize() * multiplicator ; y++) {

		for (int x=0 ; x < qr.getSize() * multiplicator; x++) {
			
            if(qr.getModule(offset_x, offset_y)) {
                row[x*3] = row[x*3+1] = row[x*3+2] = 0;
            } else {
                row[x*3] = row[x*3+1] = row[x*3+2] = 255;
            }

            if (x && x % multiplicator == 0) {
                offset_x++;
            }

		}
        offset_x = 0;
        if (y && y % multiplicator == 0) {
            offset_y++;
        }
		png_write_row(png_ptr, row);
	}

	// End write
	png_write_end(png_ptr, NULL);

	if (fp != NULL) fclose(fp);
	if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
	if (row != NULL) free(row);

	return true;
}
////////////////////
//unsigned char * neroshop::QR::get_data() const {}
////////////////////
std::string neroshop::QR::get_text() const {
    return text;
}
////////////////////
unsigned int neroshop::QR::get_size() const {
    auto qr_data = qrcodegen::QrCode::encodeText(text.c_str(), ecc);
    return (size / qr_data.getSize()) * qr_data.getSize();
}
////////////////////
std::string neroshop::QR::get_file() const {
    return filename;
}
////////////////////
////////////////////    
