#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iterator> // std::ostream_iterator
#include <cstring> // strtok, strdup
#include <sstream> // std::ostringstream
#include <algorithm> // std::transform, std::remove
#include <cctype> // std::tolower
#include <regex>

namespace neroshop {

namespace string_tools {

static bool is_email(const std::string& email) {
    const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
    return std::regex_match(email, pattern);
}

static bool is_strong_password(const std::string& password) {
    // Minimum eight characters, at least one uppercase letter, one lowercase letter, one number and one special character:
    const std::regex pattern("^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$%^&*-]).{8,}$"); // source: https://stackoverflow.com/questions/19605150/regex-for-password-must-contain-at-least-eight-characters-at-least-one-number-a
    return std::regex_match(password, pattern);
}

/**
    The username must begin with a letter (uppercase or lowercase), represented by [a-zA-Z].
    The username can contain any combination of letters (uppercase or lowercase), numbers, period . , underscore _ or hyphen -, represented by [a-zA-Z0-9._-].
    The username must be between 2 and 30 characters long, which is enforced by {0,28} in the regex. The total length includes the starting and ending letter, hence 0 to 28 characters are allowed between them.
    The username must end with a letter or number, which is enforced by [a-zA-Z0-9]$.
**/
static bool is_valid_username(const std::string& username) {
    const std::regex pattern("^[a-zA-Z][a-zA-Z0-9._-]{0,28}[a-zA-Z0-9]$");
    return std::regex_match(username, pattern);
}

// untested
static bool is_product_code(const std::string& code) {
    // Define regular expressions for each product code type
    std::regex upc("^\\d{12}$");
    std::regex ean("^\\d{13}$");
    std::regex jan("^49\\d{10}$");
    std::regex isbn("^(\\d{9}(\\d|X)|978\\d{10}|979\\d{10})$"); // validates both ISBN-10 and ISBN-13 codes
    std::regex issn("^\\d{8}$");
    std::regex gtin("^\\d{8,14}$");
    std::regex sku("^\\w+$"); // Assumes SKU can contain alphanumeric characters
    std::regex mpn("^\\w+$"); // Assumes MPN can contain alphanumeric characters
    std::regex ndc("^\\d{10}$");
    
    // Check if the code matches any of the regular expressions
    return std::regex_match(code, upc) || std::regex_match(code, ean) || std::regex_match(code, isbn) || 
           std::regex_match(code, issn) || std::regex_match(code, gtin) || std::regex_match(code, sku) ||
           std::regex_match(code, mpn) || std::regex_match(code, ndc);
}

// does not work for email-styled addresses
static bool is_valid_domain(const std::string& domain) {
    std::regex regex("^[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)+$"); // Basic domain structure

    if (!std::regex_match(domain, regex)) {
        return false;
    }

    std::string lastPart = domain.substr(domain.find_last_of('.') + 1);
    try {
        int lastPartInt = std::stoi(lastPart);
        if (std::to_string(lastPartInt) != lastPart) {
            return true;
        }
    } catch (const std::invalid_argument& e) {
        return true;
    }

    return false;
}
}
namespace string {
	static std::string lower(const std::string& str) 
	{
		std::string temp_str = str;
		std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), [](unsigned char c){ return std::tolower(c); });	
		return temp_str;
	}
	
	static std::string upper(const std::string& str)
	{
		std::string temp_str = str;
		std::transform(temp_str.begin(), temp_str.end(), temp_str.begin(), [](unsigned char c){ return std::toupper(c); });	
		return temp_str;		
	}    
	
    template <typename T>
    static std::string precision(const T value, const int n)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << value;
        return out.str();
    } 		
    
	static std::vector<std::string> split(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> output;
        char * dup = strdup(str.c_str());
        char * token = strtok(dup, delimiter.c_str());
        while(token != nullptr)
		{
            output.push_back(std::string(token));
            token = strtok(nullptr, delimiter.c_str());
        }
        free(dup);	
        return output;		
	}
		
	static bool contains(const std::string& str, const std::string& what) {
		return (str.find(what) != std::string::npos);
	}	
	static bool contains_first_of(const std::string& str, const std::string& what) {
		return (str.find(what, 0) == 0);
	}		
	static std::string swap_first_of(const std::string& str, const std::string& from, const std::string& to) // replaces first occurance of a word from a String with another
	{
		std::string string0 (str);
		size_t start = string0.find(from); // find location of the first 'from'
		if(start == std::string::npos)
			return "error"; // error
		return string0.replace(start, from.length(), to); // from location of 'from' to the length of 'from', replace with 'to'
	}
	static std::string swap_last_of(const std::string& str, const std::string& from, const std::string& to) // replace last occurance of a word from a String with another
	{
		std::string string0 (str);
		size_t start = string0.rfind(from); // find location of the last 'from'
		if(start == std::string::npos)
			return "error"; // error
		return string0.replace(start, from.length(), to); // from location of 'from' to the length of 'from', replace with 'to'
	}	
	static std::string swap_all(const std::string& str, const std::string& from, const std::string& to)
	{
		std::string string0 (str);
		while (string0.find(from) != std::string::npos) // while String contains 'from'
			string0.replace(string0.find(from), from.length(), to); // replace all occurances of 'from' (first-to-last)
		return string0;
	}	
	static bool starts_with(const std::string& str, const std::string& what, bool case_sensative = true) {
	    std::string first_word = str.substr(0, str.find_first_of(" "));
	    if(!case_sensative) {
	        return (lower(first_word) == lower(what));
	    }
	    return (first_word == what);
	}
	static std::string trim_left(const std::string& str) {
        const std::string white_spaces(" \f\n\r\t\v");
        std::string temp_str(str);
        std::string::size_type pos = temp_str.find_first_not_of(white_spaces);
        temp_str.erase(0, pos);
        return temp_str;
    }
    static std::string trim_right(const std::string& str) {
        const std::string white_spaces(" \f\n\r\t\v");
        std::string temp_str(str);
        std::string::size_type pos = temp_str.find_last_not_of(white_spaces);
        temp_str.erase(pos + 1);
        return temp_str;
    }
    static std::string trim(const std::string& str) {
        return trim_left(trim_right(str));
    }
    static std::string join(const std::vector<std::string>& string_list, std::string delimeter = ",") {
        std::stringstream ss;
        std::copy(string_list.begin(), string_list.end() - 1, std::ostream_iterator<std::string>(ss, delimeter.c_str()));
        ss << string_list.back();
        return ss.str();
    }
} // g++ string_tools.hpp

}
