#include "currency_converter.hpp"
// copied from price_api_factory.cpp
#include "coincodex/coincodex.hpp"
#include "coingecko/coingecko.hpp"
#include "coinmarketcap/coinmarketcap.hpp"
#include "cointelegraph/cointelegraph.hpp"
#include "cryptorank/cryptorank.hpp"
#include "cryptowatch/cryptowatch.hpp"
#include "fawazahmed0/fawazahmed0.hpp"
#include "kraken/kraken.hpp"

#include "../tools/logger.hpp"
#include "../tools/string.hpp"
#include "currency_map.hpp" // neroshop::CurrencyMap

namespace neroshop {
//-------------------------------------------------------
//-------------------------------------------------------
std::string Converter::json_string ("");
//-------------------------------------------------------
//-------------------------------------------------------
double Converter::to_kg(double amount, const std::string& unit_name) {
    if(neroshop::string_tools::lower(unit_name) == "lb" || neroshop::string_tools::lower(unit_name) == "lbs" || neroshop::string_tools::lower(unit_name) == "pound") {return lb_to_kg(amount);}
    return 0.0;
}
//-------------------------------------------------------
double Converter::lb_to_kg(double lb) {
    double lb2kg = 0.45359237; // 1 lb = 0.45359237 kg
    return lb * lb2kg;
}
//-------------------------------------------------------
//-------------------------------------------------------
std::unique_ptr<neroshop::PriceApi> Converter::make_price_source(PriceSource source)
{
    switch (source) {
        // Crypto Data Aggregators
        case PriceSource::CoinMarketCap:
            return std::make_unique<CoinMarketCapApi>();
        case PriceSource::CoinGecko:
            return std::make_unique<CoinGeckoApi>();
        case PriceSource::CryptoWatch:
            return std::make_unique<CryptoWatchApi>();
        case PriceSource::CoinTelegraph:
            return std::make_unique<CoinTelegraphApi>();
        case PriceSource::CryptoRank:
            return std::make_unique<CryptoRankApi>();
        case PriceSource::CoinCodex:
            return std::make_unique<CoinCodexApi>();
        case PriceSource::Fawazahmed0:
            return std::make_unique<Fawazahmed0CurrencyApi>();
        // Exchanges    
        case PriceSource::Kraken:
            return std::make_unique<KrakenApi>();
    }
    return nullptr;
}
//-------------------------------------------------------
double Converter::get_price(neroshop::Currency from, neroshop::Currency to) {
    const std::vector<neroshop::PriceSource> SOURCES_TO_USE{
        neroshop::PriceSource::CoinMarketCap,
        neroshop::PriceSource::CoinGecko,
        neroshop::PriceSource::CryptoWatch,
        neroshop::PriceSource::CoinTelegraph,
        neroshop::PriceSource::CryptoRank,
        neroshop::PriceSource::CoinCodex,
        neroshop::PriceSource::Fawazahmed0,
        // Exchanges
        neroshop::PriceSource::Kraken,
    };
    
    double price = 0.0;
    for (const auto &source : SOURCES_TO_USE) { 
        auto price_source = make_price_source(source);
        auto price_opt = price_source->price(from, to);
        if(price_opt.has_value()) {
            price = price_opt.value();//std::cout << "get_price result: " << price << "\n";
            return price;
        }
    }
    return 0.0;
}
//-------------------------------------------------------
double Converter::convert_to_xmr(double amount, const std::string& currency) {
    std::string map_key = neroshop::string_tools::upper(currency);
    
    if(neroshop::CurrencyMap.count(map_key) > 0) {
        auto map_value = neroshop::CurrencyMap.at(map_key);
        neroshop::Currency from_currency = std::get<0>(map_value);        
        double rate = Converter::get_price(neroshop::Currency::XMR, from_currency); // 1 xmr = ? currency//std::cout << amount << " " << map_key << " is equal to " << neroshop::string_tools::precision((amount / rate), 12) << " XMR\n";
        return (amount / rate);
    }
    neroshop::log_error(neroshop::string_tools::upper(currency) + " is not supported");
    return 0.0;
}
//-------------------------------------------------------
std::vector<std::string> Converter::get_currency_list() {
    std::vector<std::string> currency_list;
    for (const auto& [key, value] : neroshop::CurrencyMap) {
        currency_list.push_back(key);
    }
    return currency_list;
}
//-------------------------------------------------------
int Converter::get_currency_decimals(const std::string& currency) {
    auto map_key = neroshop::string_tools::upper(currency);
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(map_key) > 0) {
        auto map_value = neroshop::CurrencyMap.at(map_key);
        int decimal_places = std::get<2>(map_value);
        return decimal_places;
    }
    return 2;
}
//-------------------------------------------------------
double Converter::get_xmr_price(const std::string& currency) {
    auto map_key = neroshop::string_tools::upper(currency);
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(map_key) > 0) {////if(neroshop::CurrencyMap.find(map_key) != neroshop::CurrencyMap.end()) {
        auto map_value = neroshop::CurrencyMap.at(map_key);
        neroshop::Currency preferred_currency = std::get<0>(map_value);
        return Converter::get_price(neroshop::Currency::XMR, preferred_currency);
    }
    neroshop::log_error(neroshop::string_tools::upper(currency) + " is not supported");
    return 0.0;
}
//-------------------------------------------------------
std::string Converter::get_currency_sign(const std::string& currency_code) {
    auto key = neroshop::string_tools::upper(currency_code);
    // Check if key exists in std::map
    if(neroshop::CurrencyMap.count(key) > 0) {
        auto value = neroshop::CurrencyMap.at(key);
        std::string sign = std::get<3>(value);
        return sign;
    }
    return "";
} // https://www.xe.com/symbols.php
//-------------------------------------------------------
neroshop::Currency Converter::get_currency_enum(const std::string& currency) {
    auto map_key = neroshop::string_tools::upper(currency);
    if(neroshop::CurrencyMap.count(map_key) > 0) {
        auto map_value = neroshop::CurrencyMap.at(map_key);
        return std::get<0>(map_value);
    }
    return neroshop::Currency::USD;
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
bool Converter::is_supported_currency(const std::string& currency_code) {
    return (neroshop::CurrencyMap.count(neroshop::string_tools::upper(currency_code)) > 0);
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
double Converter::convert_xmr(double quantity, std::string currency, bool to) { //to: if we want currency->xmr (true) or rather xmr->currency (false)
#if !defined(NEROSHOP_USE_QT)    
    std::map<std::string, std::string>  currency_to_id_coinmarketcap = {
        {"usd", "2781"}, {"aud", "2782"}, {"cad", "2784"}, {"chf", "2785"}, {"cny", "2787"}, {"eur", "2790"},
        {"gbp", "2791"}, {"jpy", "2797"}, {"mxn", "2799"}, {"nzd", "2802"}, {"sek", "2807"},  {"btc", "1"}, {"eth", "1027"},
    }; //We can easily add new currencies, it must be added here in the coinmarketcap url that fetch usd prices

    currency = neroshop::string_tools::lower(currency);

    //Definition of variables that will be used later
    std::vector<double> prices;
    double price;
    nlohmann::json json_response;
    std::string url;
    std::string response;
    std::map<std::string, double> currencies_in_usd;

    url = "https://api.coinmarketcap.com/data-api/v3/cryptocurrency/quote/latest?id=2781,2782,2784,2785,2787,2790,2791,2797,2799,2802,2807,1,1027&convertId=2781";
    if (request(url)) {
        response = get_json();
        json_response = nlohmann::json::parse(response);
        auto currencies = json_response["data"];

        for (int i = 0; i<currencies.size(); i++) {
            std::string name = neroshop::string_tools::lower(currencies[i]["symbol"]);
            currencies_in_usd[name] = currencies[i]["quotes"][0]["price"];
        }
    }
    json_string.clear();

    if (is_supported_currency(currency) && quantity >= 0) {

        //From coingecko
        url = "https://api.coingecko.com/api/v3/simple/price?ids=monero&vs_currencies=" + currency;
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            price = json_response["monero"][currency];
            prices.push_back(price);
        }
        json_string.clear();

        //From coinmarketcap
        url = "https://api.coinmarketcap.com/data-api/v3/cryptocurrency/quote/latest?id=328&convertId=" + currency_to_id_coinmarketcap[currency];
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            price = json_response["data"][0]["quotes"][0]["price"];
            prices.push_back(price);
        }
        json_string.clear();

        //From cryptowatch
        url = "https://billboard.service.cryptowat.ch/markets?sort=price&onlyBaseAssets=xmr&onlyQuoteAssets=usd";
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            auto list_exchanges = json_response["result"]["rows"];
            for (int i = 0; i<list_exchanges.size(); i++) { //Prices are in usd only so we need to convert it
                price = list_exchanges[i]["lastPriceByAsset"]["usd"];
                prices.push_back(price/currencies_in_usd[currency]);
            }
        }
        json_string.clear();

        //From cointelegraph
        url = "https://ticker-api.cointelegraph.com/rates/?full=true";
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            if (json_response["data"]["XMR"][neroshop::string_tools::upper(currency)] != nullptr) {
                price = json_response["data"]["XMR"][neroshop::string_tools::upper(currency)]["price"];
            } else {
                price = (double)json_response["data"]["XMR"]["USD"]["price"]/currencies_in_usd[currency];
            }
            prices.push_back(price);
        }
        json_string.clear();

        //From cryptorank
        url = "https://api.cryptorank.io/v0/coins/monero?locale=en";
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            double price_usd = json_response["data"]["price"]["USD"];
            price = price_usd/currencies_in_usd[currency];
            prices.push_back(price);
        }
        json_string.clear();

        //From coincodex
        url = "https://coincodex.com/api/coincodex/get_coin/xmr";
        if (request(url)) {
            response = get_json();
            json_response = nlohmann::json::parse(response);
            double price_usd = json_response["last_price_usd"];
            price = price_usd/currencies_in_usd[currency];
            prices.push_back(price);
        }
        json_string.clear();

        //Final computation of the price
        if (prices.size() > 0) { //Check if at least one request worked
            double sum_price = 0;
            for (double p : prices) {
                sum_price += p;
            }
            price = sum_price/prices.size();
        } else {
            return -1;
        }

        if (to) {
            return quantity/price;
        } else {
            return price*quantity;
        }
    }
#endif    
    return -1;
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
bool Converter::request(const std::string& url)
{
#if !defined(NEROSHOP_USE_QT)
    // parse raw json str
    //std::string buffer;
    CURL * curl = curl_easy_init();
    long http_code = 0;
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.84.0-DEV"); //User aget needed for Cloudflare security from coinmarketcap
        //curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4); // opt - Don't bother trying IPv6, which would increase DNS resolution time.
        //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10); // opt - don't wait forever, time out after 10 secs
        //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // opt follow https redirects if necessary
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Converter::write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Converter::json_string);//&buffer);
        res = curl_easy_perform(curl);
        //curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code); // opt
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed:" << curl_easy_strerror(res) << std::endl;
            return false;
        }
        curl_easy_cleanup(curl);
        //if (http_code == 200) std::cout << "\nGot successful response from " << url << std::endl; // opt
    } 
    else { std::cout << "Could not initialize curl" << std::endl; return false; }
#endif    
    return true;
}
//-------------------------------------------------------
std::string Converter::get_json() // const; // returns whole json as a string
{
    return json_string;
}
//-------------------------------------------------------
//-------------------------------------------------------
std::size_t Converter::write_callback(char* in, std::size_t size, std::size_t num, std::string* out)
{
    const std::size_t total_bytes = size * num;
    if(total_bytes) {
        out->append(in, total_bytes);
        return total_bytes;
    }
    return 0;
}
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
//-------------------------------------------------------
}
