#include "bencode.hpp"

#include <algorithm> // std::all_of

/** NOTE: This implementation does not support inserting lists and dictionaries into lists 
or inserting dictionaries into dictionaries.
***/

std::string bencode::encode(const std::string& s) {
    return std::to_string(s.length()) + ":" + s;
}

std::string bencode::encode(int64_t i) {
    return "i" + std::to_string(i) + "e";
}

std::string bencode::encode(const std::vector<std::string>& l) {
    std::string result = "l";
    for (const auto& item : l) {
        result += encode(static_cast<std::string>(item));
    }
    result += "e";
    return result;
}

std::string bencode::encode(const std::vector<int64_t>& l) {
    std::string result = "l";
    for (const auto& item : l) {
        result += encode(static_cast<int64_t>(item));
    }
    result += "e";
    return result;
}

std::string bencode::encode(const bencode_list& l) {
    std::string result;
    if (l.empty()) {
        return "le";
    }
    result += "l";
    for (const auto& elem : l) {
        std::visit([&result](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, int64_t>) {
                result += "i" + std::to_string(arg) + "e";
            } else if constexpr (std::is_same_v<T, std::string>) {
                result += std::to_string(arg.size()) + ":" + arg;
            } else {
                // Handle error case
            }
        }, elem);
    }
    result += "e";
    return result;
}

std::string bencode::encode(const bencode_dict& d) {
    std::string result;
    result += "d";
    for (const auto& [key, value] : d) {
        result += std::to_string(key.size()) + ":" + key;
        if (std::holds_alternative<int64_t>(value)) {
            result += "i" + std::to_string(std::get<int64_t>(value)) + "e";
        } else if (std::holds_alternative<std::string>(value)) {
            result += std::to_string(std::get<std::string>(value).size()) + ":" + std::get<std::string>(value);
        } else if (std::holds_alternative<bencode_list>(value)) {
            result += encode(std::get<bencode_list>(value));
        } /*else if (std::holds_alternative<bencode_dict>(value)) {
            result += encode(std::get<bencode_dict>(value));
        }*/
    }
    result += "e";
    return result;
}

//std::string bencode::encode(const bencode_value& value) {
//}

// -----------------------------------------------------------------------------

std::string bencode::decode_string(const std::string& s, size_t& i) { // should return std::string from <length>:<string>
    size_t colon_pos = s.find(':', i);
    if (colon_pos == std::string::npos) {
        throw std::invalid_argument("Invalid bencoded string: colon not found");
    }
    size_t len = std::stoi(s.substr(i, colon_pos - i));
    i = colon_pos + 1;
    std::string result = s.substr(i, len);
    i += len;
    return result;
}

int64_t bencode::decode_integer(const std::string& s, size_t& i) { // should return int64_t from i<integer>e
    size_t e_pos = s.find('e', i);
    if (e_pos == std::string::npos) {
        throw std::invalid_argument("Invalid bencoded string: 'e' not found");
    }
    if (i+1 >= e_pos || s[i] != 'i') {
        throw std::invalid_argument("Invalid bencoded string: 'i' not found or not at the right position");
    }
    std::string integer_str = s.substr(i+1, e_pos-i-1);
    if (integer_str.size() > 1 && integer_str[0] == '0') {
        throw std::invalid_argument("Invalid bencoded string: leading zeros in integer"); // Leading zeros are not allowed in bencoded integers
    }
    if (!std::all_of(integer_str.begin(), integer_str.end(), ::isdigit)) {
        throw std::invalid_argument("Invalid bencoded string: non-numeric characters in integer");
    }
    int64_t result = std::stoll(integer_str);
    i = e_pos + 1;
    return result;
} 

bencode_list bencode::decode_list(const std::string& s, size_t& i) { // should return bencode_list from l<whatever>e
    bencode_list result;
    i++; // move past 'l' character
    while (s[i] != 'e') {
        bencode_value value = decode(s, i);
        
        if (std::holds_alternative<int64_t>(value)) {//if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int64_t>) {
            result.push_back(std::get<int64_t>(value));
        } 
        else if (std::holds_alternative<std::string>(value)) {//else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
            result.push_back(std::get<std::string>(value));
        } 
        else if (std::holds_alternative<bencode_list>(value)) {
            // handle the case where value is a bencode_list
        } 
        else if (std::holds_alternative<bencode_dict>(value)) {
            // handle the case where value is a bencode_dict
        } else {
            // handle the case where value is of an unexpected type
        }
    }
    i++; // move past 'e' character
    return result;
} 

bencode_dict bencode::decode_dict(const std::string& s, size_t& i) { // should return bencode_dict from d<whatever>e
    bencode_dict result;
    i++; // move past 'd' character
    while (s[i] != 'e') {
        std::string key = decode_string(s, i);
        bencode_value value = decode(s, i);
        
        if (std::holds_alternative<int64_t>(value)) {
            result[key] = std::get<int64_t>(value);
        } 
        else if (std::holds_alternative<std::string>(value)) {
            result[key] = std::get<std::string>(value);
        } 
        else if (std::holds_alternative<bencode_list>(value)) {
            result[key] = std::get<bencode_list>(value);
        } 
        /*else if (std::holds_alternative<bencode_dict>(value)) {
            result[key] = std::get<bencode_dict>(value);
        }*/ else {
            // handle the case where value is of an unexpected type
        }
    }
    i++; // move past 'e' character
    return result;
} 

bencode_value bencode::decode(const std::string& s, size_t& i) {
    if (s[i] == 'i') {
        return decode_integer(s, i);
    } else if (s[i] == 'l') {
        return decode_list(s, i);
    } else if (s[i] == 'd') {
        return decode_dict(s, i);
    } else {
        return decode_string(s, i);
    }
}

// -----------------------------------------------------------------------------

bool bencode::is_bencoded(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    
    size_t pos = 0;
    try {
        parse_bencoded(str, pos);
    } catch (const std::invalid_argument& e) {
        return false;
    }
    
    // Check if the entire string has been parsed
    return pos == str.length();
}

//--------------------------------------------------

void bencode::parse_bencoded_integer(const std::string& str, size_t& pos) {
    if (str[pos] != 'i') {
        throw std::invalid_argument("Invalid bencoded integer");
    }
    pos++;
    if (pos >= str.length()) {
        throw std::invalid_argument("Invalid bencoded integer");
    }
    if (str[pos] == '-') {
        pos++;
    }
    if (!std::isdigit(str[pos])) {
        throw std::invalid_argument("Invalid bencoded integer");
    }
    while (pos < str.length() && std::isdigit(str[pos])) {
        pos++;
    }
    if (pos >= str.length() || str[pos] != 'e') {
        throw std::invalid_argument("Invalid bencoded integer");
    }
    pos++;
}

void bencode::parse_bencoded_string(const std::string& str, size_t& pos) {
    size_t colon_pos = str.find(':', pos);
    if (colon_pos == std::string::npos) {
        throw std::invalid_argument("Invalid bencoded string");
    }
    std::string len_str = str.substr(pos, colon_pos - pos);
    if (!std::all_of(len_str.begin(), len_str.end(), [](char c) { return std::isdigit(c); })) {
        throw std::invalid_argument("Invalid bencoded string length");
    }
    size_t len = std::stoi(len_str);
    if (colon_pos + 1 + len > str.length()) {
        throw std::invalid_argument("Invalid bencoded string");
    }

    // Check if the string represents a key-value pair
    size_t colon_pos2 = str.find(':', colon_pos + 1);
    if (colon_pos2 != std::string::npos && colon_pos2 < colon_pos + 1 + len) {
        // Skip the key string
        pos = colon_pos2 + 1;
        // Parse the value
        parse_bencoded(str, pos);
    } else {
        // Skip the entire string
        pos = colon_pos + 1 + len;
    }
}

void bencode::parse_bencoded_list(const std::string& str, size_t& pos) {
    if (str[pos] != 'l') {
        throw std::invalid_argument("Invalid bencoded list");
    }
    pos++;
    while (pos < str.length() && str[pos] != 'e') {
        parse_bencoded(str, pos);
    }
    if (pos >= str.length() || str[pos] != 'e') {
        throw std::invalid_argument("Invalid bencoded list");
    }
    pos++;
}

void bencode::parse_bencoded_dict(const std::string& str, size_t& pos) {
    if (str[pos] != 'd') {
        throw std::invalid_argument("Invalid bencoded dict");
    }
    pos++;
    while (pos < str.length() && str[pos] != 'e') {
        parse_bencoded_string(str, pos);
        parse_bencoded(str, pos); // recursively parse the value
    }
    if (pos >= str.length() || str[pos] != 'e') {
        throw std::invalid_argument("Invalid bencoded dict");
    }
    pos++;
}

void bencode::parse_bencoded(const std::string& str, size_t& pos) {
    if (pos >= str.length()) {
        throw std::invalid_argument("Unexpected end of bencoded data");
    }
    switch (str[pos]) {
        case 'i':
            parse_bencoded_integer(str, pos);
            break;
        case 'l':
            parse_bencoded_list(str, pos);
            break;
        case 'd':
            parse_bencoded_dict(str, pos);
            break;
        default:
            if (isdigit(str[pos])) {
                parse_bencoded_string(str, pos);
            } else {
                throw std::invalid_argument("Invalid bencoded data");
            }
            break;
    }
}

/*int main() {
    std::string spam = "spam";
    int int17 = 17;
    int intneg5 = -5;
    std::vector<std::string> fruits = {"apple", "banana", "watermelon", "grapes"};//std::vector<std::string> fruits = {"apple", "banana", "watermelon", "grapes"};
    std::vector<int64_t> numbers = { 1, 2, 3, 4, 5, 6, };
    bencode_list hybrid_list = {"apple", 1, "banana", 2};

    std::cout << spam << " = " << bencode::encode(spam) << "\n";
    std::cout << int17 << "   = " << bencode::encode(int17) << "\n";
    std::cout << intneg5 << "   = " << bencode::encode(intneg5) << "\n";
    std::cout << "fruits" << " = " << bencode::encode(fruits) << "\n\n";
    std::cout << "numbers" << " = " << bencode::encode(numbers) << "\n\n";
    std::cout << "hybrid_list" << " = " << bencode::encode(hybrid_list) << "\n\n";
    
    
    bencode_dict students;
    students["sid"] = "26";
    students["jack"] = "44";
    students["dude"] = "18";
    std::cout << "students: " << bencode::encode(students) << "\n\n";

    bencode_list names = {"John", "Doe"};
    bencode_list scores = {10, 20, 30};
    bencode_list items = {"sword", "shield", "potion"};
    bencode_dict player_stats;
    player_stats["name"] = names;
    player_stats["score"] = scores;
    player_stats["items"] = items;
    
    std::cout << "player_stats: " << bencode::encode(player_stats) << "\n";
    std::cout << std::endl;
    //---------------------------------------------------------------------
    size_t pos = 0;
    std::cout << bencode::decode_string("3:egg", pos) << "\n";
    pos = 0;
    std::cout << bencode::decode_integer("i96e", pos) << "\n";
    pos = 0;
    std::cout << bencode::decode_integer("i144000e", pos) << "\n\n";
    
    pos = 0;
    bencode_list decoded_list = bencode::decode_list("l5:applei1e6:bananai2e6:orangei3ee", pos);//l5:apple6:banana10:watermelon6:grapese   li1ei2ei3ei4ei5ei6ee
    for(const auto& value : decoded_list) {
        std::visit([](const auto& val) {
            std::cout << val << ", ";
        }, value);
    }
    std::cout << "\n" << std::endl;
    
    pos = 0;
    bencode_dict decoded_dict = bencode::decode_dict("d5:itemsl5:sword6:shield6:potione5:scoreli10ei20ei30ee4:namel4:John3:Doeee", pos);
    for (const auto& [key, value] : decoded_dict) {
        std::cout << key << ": ";
        if (std::holds_alternative<int64_t>(value)) {
            std::cout << std::get<int64_t>(value) << std::endl;
        } else if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value) << std::endl;
        } else if (std::holds_alternative<bencode_list>(value)) {
            const bencode_list& list = std::get<bencode_list>(value);
            std::cout << "[";

            for (const auto& item : list) {
                if (std::holds_alternative<int64_t>(item)) {
                    std::cout << std::get<int64_t>(item);
                } else if (std::holds_alternative<std::string>(item)) {
                    std::cout << std::get<std::string>(item);
                }
                std::cout << ", ";
            }

            std::cout << "]" << std::endl;
        }
    }
    
    return 0;
}*/ // g++ bencode_test.cpp -std=c++17

