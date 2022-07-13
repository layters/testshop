// filename: converter.hpp
#ifndef CONVERTER_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define CONVERTER_HPP_NEROSHOP

#include <iostream>
// curl
#include <curl/curl.h>
// nlohmann-json
#include <nlohmann/json.hpp>
// dokun-ui
#include <string.hpp>

#include <vector>
#include <algorithm> 

//enum class tracker {coinmarketcap, coincodex, coingecko};
// https://www.coingecko.com/api/documentations/v3
namespace neroshop {
class Converter {
public:
    // weight (mass)(kilogram[kg] is commonly used worldwide while pounds[lbs] is mostly used in both US and UK)
    double to_kg(double amount, const std::string& unit_name) const;
    static double lb_to_kg(double lb); //static double pound_to_kilogram(double pound); // The correct way of abbreviation in expressing singular or plural pounds is “lb.” though “lbs.”, which stands for libra, is the common abbreviation used in expressing pounds
    // getters
    
    static double convert_xmr(double quantity, std::string currency, bool to);
    static std::string get_currency_symbol(const std::string& currency_code);
    // boolean
    static bool is_supported_currency(const std::string& currency_code);
private:
    static bool request(const std::string& url);
    static std::string get_json();// const; // returns whole json as a string //static std::string get_json(const std::string& key);// const
    static std::size_t write_callback(char* in, std::size_t size, std::size_t num, std::string* out);
    static std::string json_string;//std::string url;
};
}
#endif

/*Usage:
	double pounds = 1;
	std::cout << pounds << " lb = " << Converter::lb_to_kg(pounds) << " kg" << std::endl;
	pounds = 50;
	std::cout << pounds << " lb = " << Converter::lb_to_kg(pounds) << " kg" << std::endl;
*/    
