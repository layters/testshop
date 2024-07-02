#include "file_piece_hasher.hpp"

#include <fstream>

#include "../../crypto/sha3.hpp"

namespace neroshop {

FilePieceHasher::FilePieceHasher(size_t piece_size) : piece_size(piece_size) {}

std::vector<FilePiece> FilePieceHasher::hash_file(const std::string& filename) {
    std::vector<FilePiece> piece_hashes;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return piece_hashes;
    }
    
    std::vector<unsigned char> buffer(piece_size);
    while (!file.eof()) {
        file.read(reinterpret_cast<char*>(buffer.data()), piece_size);
        size_t bytes_read = file.gcount();
        if (bytes_read > 0) {
            std::string buffer_str(buffer.begin(), buffer.begin() + bytes_read);
            std::string hash = crypto::sha3_256(buffer_str);
            piece_hashes.push_back(FilePiece{ hash, bytes_read, std::vector<unsigned char>(buffer.begin(), buffer.begin() + bytes_read) });
        }
    }
    
    return piece_hashes;
}

}
//----------------------------------------------------------------------------------
/*int main() {
    size_t piece_size = 16384; // each file piece will be 16 KB
    neroshop::FilePieceHasher hasher(piece_size);
    std::vector<neroshop::FilePiece> file_pieces = hasher.hash_file("Wownero_Pixel_Power.gif");
    
    // Store or share piece hashes with other nodes in your DHT network

    size_t file_size = 0;
    // Example: Print piece hashes
    for (size_t i = 0; i < file_pieces.size(); ++i) {
        std::cout << "Piece " << i << " Hash: " << file_pieces[i].hash << " (" << file_pieces[i].bytes << ")" << std::endl;
        file_size += file_pieces[i].bytes;
    }
    std::cout << std::endl << "Total pieces: " << file_pieces.size() << std::endl;
    std::cout << "Size per piece: " << piece_size << std::endl;
    std::cout << "Remaining data: " << (file_size % piece_size) << std::endl;
    std::cout << "File size: " << file_size << std::endl;

}*/
// g++ file_piece_hasher.cpp -o fphasher -lcrypto -lssl
