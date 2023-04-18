#pragma once

#ifndef BENCODE_HPP_NEROSHOP
#define BENCODE_HPP_NEROSHOP

#include <iostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using bencode_variant = std::variant< int64_t, std::string, std::vector< std::variant<int64_t, std::string> >, std::unordered_map<std::string, std::variant< int64_t, std::string, std::vector< std::variant<int64_t, std::string> > >> >;
using bencode_dict = std::unordered_map<std::string, bencode_variant>;
using bencode_list = std::vector< std::variant<int64_t, std::string> >; // std::vector that can store both integers and strings
//using bencode_dict = std::unordered_map<std::string, std::variant< int64_t, std::string, bencode_list >>; // std::unordered_map that can store a string along with any of these types: int, std::string, bencode_list
using bencode_value = std::variant< int64_t, std::string, bencode_list, bencode_dict >;

namespace bencode {

std::string encode(const std::string& s);

std::string encode(int64_t i);

std::string encode(const std::vector<std::string>& l);

std::string encode(const std::vector<int64_t>& l);

std::string encode(const bencode_list& l);

std::string encode(const bencode_dict& d);

//std::string encode(const bencode_value& value);

// -----------------------------------------------------------------------------

std::string decode_string(const std::string& s, size_t& i);

int64_t decode_integer(const std::string& s, size_t& i);

bencode_list decode_list(const std::string& s, size_t& i);

bencode_dict decode_dict(const std::string& s, size_t& i);

bencode_value decode(const std::string& s, size_t& i);

// -----------------------------------------------------------------------------

bool is_bencoded(const std::string& str);

//------------------------------------------------------------------------------

void parse_bencoded_integer(const std::string& str, size_t& pos);

void parse_bencoded_string(const std::string& str, size_t& pos);

void parse_bencoded_list(const std::string& str, size_t& pos);

void parse_bencoded_dict(const std::string& str, size_t& pos);

void parse_bencoded(const std::string& str, size_t& pos);

} // namespace bencode
#endif
