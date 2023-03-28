#pragma once

#ifndef CURRENCY_CONVERTER_HPP_NEROSHOP
#define CURRENCY_CONVERTER_HPP_NEROSHOP

#if defined(NEROSHOP_USE_QT)
#else
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#endif

#include <vector>
#include <algorithm>
#include <map>
#include <random>
#include <thread>
#include <future>

#include <memory> // std::unique_ptr
#include "price_api/price_api.hpp"

namespace neroshop {

class Converter {
public:
    // weight (mass)
    double to_kg(double amount, const std::string& unit_name) const;
    static double lb_to_kg(double lb); //static double pound_to_kilogram(double pound); // The correct way of abbreviation in expressing singular or plural pounds is “lb.” though “lbs.”, which stands for libra, is the common abbreviation used in expressing pounds
    
    static std::unique_ptr<PriceApi> make_price_source(PriceSource source);
    // getters
    static double convert_to_xmr(double amount, const std::string& currency);
    static std::vector<std::string> get_currency_list();
    static int get_currency_decimals(const std::string& currency);
    static double get_xmr_price(const std::string& currency);

    static double get_price(neroshop::Currency from, neroshop::Currency to);
    
    // deprecated
    static double convert_xmr(double quantity, std::string currency, bool to);
    
    static std::string get_currency_sign(const std::string& currency_code);
    static neroshop::Currency get_currency_enum(const std::string& currency);
    // boolean
    static bool is_supported_currency(const std::string& currency_code);
private:
    static bool request(const std::string& url);
    static std::string get_json();// const; // returns whole json as a string
    static std::size_t write_callback(char* in, std::size_t size, std::size_t num, std::string* out);
    static std::string json_string;
};
}
#endif
