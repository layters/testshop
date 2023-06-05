#include "base64.hpp"

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

std::string neroshop::base64_encode(const std::string& input) {
    // Create a BIO object for Base64 encoding
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // Create a BIO object for memory buffering
    BIO* mem = BIO_new(BIO_s_mem());
    BIO_push(b64, mem);

    // Write input data to the BIO for encoding
    BIO_write(b64, input.c_str(), static_cast<int>(input.length()));
    BIO_flush(b64);

    // Read the encoded data from the BIO
    BUF_MEM* mem_buf;
    BIO_get_mem_ptr(b64, &mem_buf);

    // Copy the encoded data to a string
    std::string encoded_data(mem_buf->data, mem_buf->length);

    // Cleanup
    BIO_free_all(b64);

    return encoded_data;
}

std::string neroshop::base64_decode(const std::string& encoded) {
    // Create a BIO object for Base64 decoding
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // Create a BIO object for memory buffering
    BIO* mem = BIO_new_mem_buf(encoded.c_str(), static_cast<int>(encoded.length()));
    BIO_push(b64, mem);

    // Determine the size of the decoded data
    size_t max_decoded_length = (encoded.length() * 3) / 4;
    std::string decoded_data(max_decoded_length, '\0');

    // Decode the data
    int decoded_length = BIO_read(b64, decoded_data.data(), static_cast<int>(decoded_data.length()));

    // Resize the string to the actual decoded length
    decoded_data.resize(decoded_length);

    // Cleanup
    BIO_free_all(b64);

    return decoded_data;
}

/*int main() {
    std::string input = "Hello, World!";

    // Base64 encode
    std::string encoded = neroshop::base64_encode(input);
    std::cout << "Encoded: " << encoded << std::endl;

    // Base64 decode
    std::string decoded = neroshop::base64_decode(encoded);
    std::cout << "Decoded: " << decoded << std::endl;

    return 0;
}*/
// g++ base64.cpp -o base64 -lcrypto -lssl

