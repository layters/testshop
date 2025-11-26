#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <unordered_map>

namespace neroshop {

struct City {
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
};

// country -> region/state -> vector of City structs
inline const std::map<std::string, std::map<std::string, std::vector<City>>> locations = {
    {"Afghanistan", {}},
    {"Albania", {}},
    {"Algeria", {}},
    {"Andorra", {}},
    {"Angola", {}},
    {"Antigua & Deps", {}},
    {"Argentina", {
        {"Buenos Aires", {}},
        {"Catamarca", {}},
        {"Chaco", {}},
        {"Chubut", {}},
        {"Ciudad de Buenos Aires", {}},
        {"Córdoba", {}},
        {"Corrientes", {}},
        {"Entre Ríos", {}},
        {"Formosa", {}},
        {"Jujuy", {}},
        {"La Pampa", {}},
        {"La Rioja", {}},
        {"Mendoza", {}},
        {"Misiones", {}},
        {"Neuquén", {}},
        {"Río Negro", {}},
        {"Salta", {}},
        {"San Juan", {}},
        {"San Luis", {}},
        {"Santa Cruz", {}},
        {"Santa Fe", {}},
        {"Santiago del Estero", {}},
        {"Tierra del Fuego", {}},
        {"Tucumán", {}}
    }},
    {"Armenia", {}},
    {"Australia", {
        {"New South Wales", { City{"Sydney", -33.8688, 151.2093} }},
        {"Queensland", { City{"Brisbane", -27.4698, 153.0251} }},
        {"South Australia", { City{"Adelaide", -34.9285, 138.6007} }},
        {"Tasmania", { City{"Hobart"} }},
        {"Victoria", { City{"Melbourne"} }},
        {"Western Australia", { City{"Perth"} }},
        {"Australian Capital Territory", { City{"Canberra"} }},
        {"Northern Territory", { City{"Darwin"} }}
    }},
    {"Austria", {}},
    {"Azerbaijan", {}},
    {"Bahamas", {}},
    {"Bahrain", {}},
    {"Bangladesh", {}},
    {"Barbados", {}},
    {"Belarus", {}},
    {"Belgium", {}},
    {"Belize", {}},
    {"Benin", {}},
    {"Bermuda", {}},
    {"Bhutan", {}},
    {"Bolivia", {}},
    {"Bosnia Herzegovina", {}},
    {"Botswana", {}},
    {"Brazil", {
        {"Acre", {}},
        {"Alagoas", {}},
        {"Amapá", {}},
        {"Amazonas", {}},
        {"Bahia", {}},
        {"Ceará", {}},
        {"Distrito Federal", {}},
        {"Espírito Santo", {}},
        {"Goiás", {}},
        {"Maranhão", {}},
        {"Mato Grosso", {}},
        {"Mato Grosso do Sul", {}},
        {"Minas Gerais", {}},
        {"Pará", {}},
        {"Paraíba", {}},
        {"Paraná", {}},
        {"Pernambuco", {}},
        {"Piauí", {}},
        {"Rio de Janeiro", {}},
        {"Rio Grande do Norte", {}},
        {"Rio Grande do Sul", {}},
        {"Rondônia", {}},
        {"Roraima", {}},
        {"Santa Catarina", {}},
        {"São Paulo", {}},
        {"Sergipe", {}},
        {"Tocantins", {}}
    }},
    {"Brunei", {}},
    {"Bulgaria", {}},
    {"Burkina", {}},
    {"Burundi", {}},
    {"Cambodia", {}},
    {"Cameroon", {}},
    {"Canada", {
        // Provinces
        {"Alberta", { City{"Calgary", 51.0447, -114.0719}, City{"Edmonton", 53.5461, -113.4938} }},
        {"British Columbia", { City{"Vancouver", 49.2827, -123.1207}, City{"Victoria", 48.4284, -123.3656} }},
        {"Manitoba", { City{"Winnipeg"} }},
        {"New Brunswick", { City{"Fredericton"}, City{"Moncton"} }},
        {"Newfoundland and Labrador", { City{"St. John's"} }},
        {"Nova Scotia", { City{"Halifax"} }},
        {"Ontario", { City{"Ottawa"}, City{"Toronto"} }},
        {"Prince Edward Island", { City{"Charlottetown"} }},
        {"Quebec", { City{"Montreal"}, City{"Quebec City"} }},
        {"Saskatchewan", { City{"Regina"}, City{"Saskatoon"} }},
        
        // Territories
        {"Northwest Territories", { City{"Yellowknife"} }},
        {"Nunavut", { City{"Iqaluit"} }},
        {"Yukon", { City{"Whitehorse"} }}
    }},
    {"Cape Verde", {}},
    {"Central African Rep", {}},
    {"Chad", {}},
    {"Chile", {}},
    {"China", {
        // Provinces
        {"Anhui", {}},
        {"Fujian", {}},
        {"Gansu", {}},
        {"Guangdong", {}},
        {"Guizhou", {}},
        {"Hainan", {}},
        {"Hebei", {}},
        {"Heilongjiang", {}},
        {"Henan", {}},
        {"Hubei", {}},
        {"Hunan", {}},
        {"Jiangsu", {}},
        {"Jiangxi", {}},
        {"Jilin", {}},
        {"Liaoning", {}},
        {"Qinghai", {}},
        {"Shaanxi", {}},
        {"Shandong", {}},
        {"Shanxi", {}},
        {"Sichuan", {}},
        //{"Taiwan", {}},  // Listed as a province by China
        {"Yunnan", {}},
        {"Zhejiang", {}},
        
        // Autonomous Regions
        {"Guangxi", {}},
        {"Inner Mongolia", {}},
        {"Ningxia", {}},
        {"Tibet", {}},
        {"Xinjiang", {}},
        
        // Municipalities
        {"Beijing", {}},
        {"Chongqing", {}},
        {"Shanghai", {}},
        {"Tianjin", {}},
        
        // Special Administrative Regions (SARs)
        {"Hong Kong", {}},
        {"Macau", {}}
    }},
    {"Colombia", {}},
    {"Comoros", {}},
    {"Congo", {}},
    {"Congo (Democratic Rep)", {}},
    {"Costa Rica", {}},
    {"Croatia", {}},
    {"Cuba", {}},
    {"Cyprus", {}},
    {"Czech Republic", {}},
    {"Denmark", {}},
    {"Djibouti", {}},
    {"Dominica", {}},
    {"Dominican Republic", {}},
    {"East Timor", {}},
    {"Ecuador", {}},
    {"Egypt", {}},
    {"El Salvador", {}},
    {"Equatorial Guinea", {}},
    {"Eritrea", {}},
    {"Estonia", {}},
    {"Eswatini", {}},
    {"Ethiopia", {}},
    {"Fiji", {}},
    {"Finland", {}},
    {"France", {}},
    {"Gabon", {}},
    {"Gambia", {}},
    {"Georgia", {}},
    {"Germany", {
        {"Baden-Württemberg", { City{"Stuttgart"} }},
        {"Bavaria (Bayern)", { City{"Munich (München)"} }},
        {"Berlin", { City{"Berlin"} }}, // Both a city and a state
        {"Brandenburg", { City{"Potsdam"} }},
        {"Bremen", { City{"Bremen"} }}, // Both a city and a state
        {"Hamburg", { City{"Hamburg"} }}, // Both a city and a state
        {"Hesse (Hessen)", { City{"Wiesbaden"} }},
        {"Lower Saxony (Niedersachsen)", { City{"Hanover (Hannover)"} }},
        {"Mecklenburg-Western Pomerania (Mecklenburg-Vorpommern)", { City{"Schwerin"} }},
        {"North Rhine-Westphalia (Nordrhein-Westfalen)", { City{"Düsseldorf"} }},
        {"Rhineland-Palatinate (Rheinland-Pfalz)", { City{"Mainz"} }},
        {"Saarland", { City{"Saarbrücken"} }},
        {"Saxony (Sachsen)", { City{"Dresden"} }},
        {"Saxony-Anhalt (Sachsen-Anhalt)", { City{"Magdeburg"} }},
        {"Schleswig-Holstein", { City{"Kiel"} }},
        {"Thuringia (Thüringen)", { City{"Erfurt"} }}
    }},
    {"Ghana", {
        {"Ahafo", { City{"Goaso"} }},
        {"Ashanti", { City{"Kumasi"} }},
        {"Bono", { City{"Sunyani"} }},
        {"Bono East", { City{"Techiman"} }},
        {"Central", { City{"Abura Dunkwa"}, City{"Afransi"}, City{"Agona Swedru"}, City{"Ajumako"}, City{"Apam"}, City{"Assin Breku"}, City{"Assin Fosu"}, City{"Awutu Breku"}, City{"Bremang Asikuma"}, City{"Cape Coast"}, City{"Diaso"}, City{"Dunkwa-on-Offin"}, City{"Elmina"}, City{"Essarkyir"}, City{"Kasoa"}, City{"Nsaba"}, City{"Nsuaem Kyekyewere"}, City{"Potsin"}, City{"Saltpond"}, City{"Twifo Hemang"}, City{"Twifo Praso"}, City{"Winneba"} }},
        {"Eastern", { City{"Koforidua"} }},
        {"Greater Accra", { City{"Abokobi"}, City{"Accra"}, City{"Accra New Town"}, City{"Ada Foah"}, City{"Adenta"}, City{"Amasaman"}, City{"Ashaiman"}, City{"Dansoman"}, City{"Darkuman"}, City{"Dodowa"}, City{"Dzorwulu"}, City{"Kokomlemle"}, City{"Kpone"}, City{"La"}, City{"Larteboikorshie"}, City{"Madina"}, City{"Ngleshie Amanfro"}, City{"Nima"}, City{"Nungua"}, City{"Ofankor"}, City{"Osu"}, City{"Prampram"}, City{"Sege"}, City{"Sowutuom"}, City{"Tema"}, City{"Tema Community 18"}, City{"Tesano"}, City{"Teshie"}, City{"Weija"} }},
        {"Northern", { City{"Tamale"} }},
        {"North East", { City{"Nalerigu"} }},
        {"Oti", { City{"Dambai"} }},
        {"Savannah", { City{"Damongo"} }},
        {"Upper East", { City{"Bolgatanga"} }},
        {"Upper West", { City{"Wa"} }},
        {"Volta", { City{"Ho"} }},
        {"Western", { City{"Sekondi-Takoradi"} }},
        {"Western North", { City{"Sefwi Wiawso"} }}
    }},
    {"Greece", {}},
    {"Grenada", {}},
    {"Guatemala", {}},
    {"Guinea", {}},
    {"Guinea-Bissau", {}},
    {"Guyana", {}},
    {"Haiti", {}},
    {"Honduras", {}},
    {"Hungary", {}},
    {"Iceland", {}},
    {"India", {
        // States
        {"Andhra Pradesh", {}},
        {"Arunachal Pradesh", {}},
        {"Assam", {}},
        {"Bihar", {}},
        {"Chhattisgarh", {}},
        {"Goa", {}},
        {"Gujarat", {}},
        {"Haryana", {}},
        {"Himachal Pradesh", {}},
        {"Jharkhand", {}},
        {"Karnataka", {}},
        {"Kerala", {}},
        {"Madhya Pradesh", {}},
        {"Maharashtra", {}},
        {"Manipur", {}},
        {"Meghalaya", {}},
        {"Mizoram", {}},
        {"Nagaland", {}},
        {"Odisha", {}},
        {"Punjab", {}},
        {"Rajasthan", {}},
        {"Sikkim", {}},
        {"Tamil Nadu", {}},
        {"Telangana", {}},
        {"Tripura", {}},
        {"Uttar Pradesh", {}},
        {"Uttarakhand", {}},
        {"West Bengal", {}},
        
        // Union Territories
        {"Andaman and Nicobar Islands", {}},
        {"Chandigarh", {}},
        {"Dadra and Nagar Haveli and Daman and Diu", {}},
        {"Delhi", {}},
        {"Lakshadweep", {}},
        {"Ladakh", {}},
        {"Jammu and Kashmir", {}},
        {"Puducherry", {}}
    }},
    {"Indonesia", {}},
    {"Iran", {}},
    {"Iraq", {}},
    {"Ireland (Republic)", {}},
    {"Israel", {}},
    {"Italy", {}},
    {"Ivory Coast", {}},
    {"Jamaica", {}},
    {"Japan", {}},
    {"Jordan", {}},
    {"Kazakhstan", {
        {"Akmola", {}},
        {"Aktobe", {}},
        {"Almaty", {}},
        {"Atyrau", {}},
        {"East Kazakhstan", {}},
        {"Jambyl", {}},
        {"Karaganda", {}},
        {"Kostanay", {}},
        {"Kyzylorda", {}},
        {"Mangystau", {}},
        {"North Kazakhstan", {}},
        {"Pavlodar", {}},
        {"South Kazakhstan", {}},
        {"West Kazakhstan", {}},
        //{"Almaty City", {}},  // Special city (not part of any region)
        //{"Nur-Sultan", {}}   // Special city (not part of any region)
    }},
    {"Kenya", {}},
    {"Kiribati", {}},
    {"Korea North", {}},
    {"Korea South", {}},
    {"Kosovo", {}},
    {"Kuwait", {}},
    {"Kyrgyzstan", {}},
    {"Laos", {}},
    {"Latvia", {}},
    {"Lebanon", {}},
    {"Lesotho", {}},
    {"Liberia", {}},
    {"Libya", {}},
    {"Liechtenstein", {}},
    {"Lithuania", {}},
    {"Luxembourg", {}},
    {"Macedonia", {}},
    {"Madagascar", {}},
    {"Malawi", {}},
    {"Malaysia", {}},
    {"Maldives", {}},
    {"Mali", {}},
    {"Malta", {}},
    {"Marshall Islands", {}},
    {"Mauritania", {}},
    {"Mauritius", {}},
    {"Mexico", {}},
    {"Micronesia", {}},
    {"Moldova", {}},
    {"Monaco", {}},
    {"Mongolia", {}},
    {"Montenegro", {}},
    {"Morocco", {}},
    {"Mozambique", {}},
    {"Myanmar", {}},
    {"Namibia", {}},
    {"Nauru", {}},
    {"Nepal", {}},
    {"Netherlands", {}},
    {"New Zealand", {}},
    {"Nicaragua", {}},
    {"Niger", {}},
    {"Nigeria", {
        {"Abia", {}},
        {"Adamawa", {}},
        {"Akwa Ibom", {}},
        {"Anambra", {}},
        {"Bauchi", {}},
        {"Bayelsa", {}},
        {"Benue", {}},
        {"Borno", {}},
        {"Cross River", {}},
        {"Delta", {}},
        {"Ebonyi", {}},
        {"Edo", {}},
        {"Ekiti", {}},
        {"Enugu", {}},
        {"Federal Capital Territory", {}},
        {"Gombe", {}},
        {"Imo", {}},
        {"Jigawa", {}},
        {"Kaduna", {}},
        {"Kano", {}},
        {"Katsina", {}},
        {"Kebbi", {}},
        {"Kogi", {}},
        {"Kwara", {}},
        {"Lagos", {}},
        {"Nasarawa", {}},
        {"Niger", {}},
        {"Ogun", {}},
        {"Ondo", {}},
        {"Osun", {}},
        {"Oyo", {}},
        {"Plateau", {}},
        {"Rivers", {}},
        {"Sokoto", {}},
        {"Taraba", {}},
        {"Yobe", {}},
        {"Zamfara", {}}
    }},
    {"Norway", {}},
    {"Oman", {}},
    {"Online", {}},
    {"Pakistan", {}},
    {"Palau", {}},
    {"Palestine", {}},
    {"Panama", {}},
    {"Papua New Guinea", {}},
    {"Paraguay", {}},
    {"Peru", {}},
    {"Philippines", {}},
    {"Poland", {}},
    {"Portugal", {}},
    {"Qatar", {}},
    {"Romania", {}},
    {"Russian Federation", {
        // Republics
        {"Adygea", {}},
        {"Altai Republic", {}},
        {"Bashkortostan", {}},
        {"Buryatia", {}},
        {"Chechnya", {}},
        {"Chuvashia", {}},
        {"Crimea", {}},
        {"Dagestan", {}},
        {"Ingushetia", {}},
        {"Kabardino-Balkaria", {}},
        {"Kalmykia", {}},
        {"Karachay-Cherkessia", {}},
        {"Karelia", {}},
        {"Khakassia", {}},
        {"Komi", {}},
        {"Mari El", {}},
        {"Mordovia", {}},
        {"North Ossetia–Alania", {}},
        {"Sakha (Yakutia)", {}},
        {"Tatarstan", {}},
        {"Tuva", {}},
        {"Udmurtia", {}},
        
        // Krais
        {"Altai Krai", {}},
        {"Kamchatka Krai", {}},
        {"Khabarovsk Krai", {}},
        {"Krasnodar Krai", {}},
        {"Krasnoyarsk Krai", {}},
        {"Perm Krai", {}},
        {"Primorsky Krai", {}},
        {"Stavropol Krai", {}},
        {"Zabaykalsky Krai", {}},
        
        // Oblasts
        {"Amur Oblast", {}},
        {"Arkhangelsk Oblast", {}},
        {"Astrakhan Oblast", {}},
        {"Belgorod Oblast", {}},
        {"Bryansk Oblast", {}},
        {"Chelyabinsk Oblast", {}},
        {"Irkutsk Oblast", {}},
        {"Ivanovo Oblast", {}},
        {"Kaliningrad Oblast", {}},
        {"Kaluga Oblast", {}},
        {"Kemerovo Oblast", {}},
        {"Kirov Oblast", {}},
        {"Kostroma Oblast", {}},
        {"Kurgan Oblast", {}},
        {"Kursk Oblast", {}},
        {"Leningrad Oblast", {}},
        {"Lipetsk Oblast", {}},
        {"Magadan Oblast", {}},
        {"Moscow Oblast", {}},
        {"Murmansk Oblast", {}},
        {"Nizhny Novgorod Oblast", {}},
        {"Novgorod Oblast", {}},
        {"Novosibirsk Oblast", {}},
        {"Omsk Oblast", {}},
        {"Orenburg Oblast", {}},
        {"Oryol Oblast", {}},
        {"Penza Oblast", {}},
        {"Pskov Oblast", {}},
        {"Rostov Oblast", {}},
        {"Ryazan Oblast", {}},
        {"Samara Oblast", {}},
        {"Saratov Oblast", {}},
        {"Sakhalin Oblast", {}},
        {"Sverdlovsk Oblast", {}},
        {"Smolensk Oblast", {}},
        {"Tambov Oblast", {}},
        {"Tomsk Oblast", {}},
        {"Tula Oblast", {}},
        {"Tver Oblast", {}},
        {"Tyumen Oblast", {}},
        {"Ulyanovsk Oblast", {}},
        {"Vladimir Oblast", {}},
        {"Volgograd Oblast", {}},
        {"Vologda Oblast", {}},
        {"Voronezh Oblast", {}},
        {"Yaroslavl Oblast", {}},
        
        // Federal Cities
        {"Moscow", {}},
        {"Saint Petersburg", {}},
        {"Sevastopol", {}},
        
        // Autonomous Okrugs
        {"Chukotka Autonomous Okrug", {}},
        {"Khanty-Mansi Autonomous Okrug", {}},
        {"Nenets Autonomous Okrug", {}},
        {"Yamalo-Nenets Autonomous Okrug", {}},
        
        // Autonomous Oblast
        {"Jewish Autonomous Oblast", {}}
    }},
    {"Rwanda", {}},
    {"St Kitts & Nevis", {}},
    {"St Lucia", {}},
    {"Saint Vincent & the Grenadines", {}},
    {"Samoa", {}},
    {"San Marino", {}},
    {"Sao Tome & Principe", {}},
    {"Saudi Arabia", {}},
    {"Senegal", {}},
    {"Serbia", {}},
    {"Seychelles", {}},
    {"Sierra Leone", {}},
    {"Singapore", {}},
    {"Slovakia", {}},
    {"Slovenia", {}},
    {"Solomon Islands", {}},
    {"Somalia", {}},
    {"South Africa", {}},
    {"South Sudan", {}},
    {"Spain", {}},
    {"Sri Lanka", {}},
    {"Sudan", {}},
    {"Suriname", {}},
    {"Sweden", {}},
    {"Switzerland", {}},
    {"Syria", {}},
    {"Taiwan", {}},
    {"Tajikistan", {}},
    {"Tanzania", {}},
    {"Thailand", {}},
    {"Togo", {}},
    {"Tonga", {}},
    {"Trinidad & Tobago", {}},
    {"Tunisia", {}},
    {"Turkey", {}},
    {"Turkmenistan", {}},
    {"Tuvalu", {}},
    {"Uganda", {}},
    {"Ukraine", {}},
    {"United Arab Emirates", {}},
    {"United Kingdom", {
        {"England", { City{"London", 51.5074, -0.1278} }},
        {"Northern Ireland", { City{"Belfast", 54.5973, -5.9301} }},
        {"Scotland", { City{"Edinburgh", 55.9533, -3.1883}, City{"Glasgow", 55.8642, -4.2518} }},
        {"Wales", { City{"Cardiff", 51.4816, -3.1791} }},
    }},
    {"United States", {
        {"Alabama", { City{"Montgomery"} }},
        {"Alaska", { City{"Juneau"} }},
        {"American Samoa", { City{"Pago Pago"} }}, // Outlying area
        {"Arizona", { City{"Phoenix"} }},
        {"Arkansas", { City{"Little Rock"} }},
        {"California", { City{"Los Angeles"}, City{"Sacramento"}, City{"San Diego"}, City{"San Francisco"} }},
        {"Colorado", { City{"Denver"} }},
        {"Connecticut", { City{"Hartford"} }},
        {"Delaware", { City{"Dover"} }},
        {"Florida", { City{"Miami"}, City{"Tallahassee"} }},
        {"Georgia", { City{"Atlanta"} }},
        {"Guam", { City{"Hagåtña"} }}, // Outlying area
        {"Hawaii", { City{"Honolulu"} }},
        {"Idaho", { City{"Boise"} }},
        {"Illinois", { City{"Chicago"}, City{"Springfield"} }},
        {"Indiana", { City{"Indianapolis"} }},
        {"Iowa", { City{"Des Moines"} }},
        {"Kansas", { City{"Topeka"} }},
        {"Kentucky", { City{"Frankfort"} }},
        {"Louisiana", { City{"Baton Rouge"} }},
        {"Maine", { City{"Augusta"} }},
        {"Maryland", { City{"Annapolis"} }},
        {"Massachusetts", { City{"Boston"}, City{"Springfield"}, City{"Worcester"} }},
        {"Michigan", { City{"Lansing"} }},
        {"Minnesota", { City{"Saint Paul"} }},
        {"Mississippi", { City{"Jackson"} }},
        {"Missouri", { City{"Jefferson City"} }},
        {"Montana", { City{"Helena"} }},
        {"Nebraska", { City{"Lincoln"} }},
        {"Nevada", { City{"Carson City"}, City{"Las Vegas"} }},
        {"New Hampshire", { City{"Concord"} }},
        {"New Jersey", { City{"Trenton"} }},
        {"New Mexico", { City{"Santa Fe"} }},
        {"New York", { City{"Albany"}, City{"Buffalo"}, City{"New York City"}, City{"Rochester"} }},
        {"North Carolina", { City{"Raleigh"} }},
        {"North Dakota", { City{"Bismarck"} }},
        {"Northern Mariana Islands", { City{"Saipan"} }}, // Outlying area
        {"Ohio", { City{"Columbus"} }},
        {"Oklahoma", { City{"Oklahoma City"} }},
        {"Oregon", { City{"Salem"} }},
        {"Pennsylvania", { City{"Harrisburg"}, City{"Philadelphia"} }},
        {"Puerto Rico", { City{"San Juan"} }}, // Outlying area
        {"Rhode Island", { City{"Providence"} }},
        {"South Carolina", { City{"Columbia"} }},
        {"South Dakota", { City{"Pierre"} }},
        {"Tennessee", { City{"Nashville"} }},
        {"Texas", { City{"Austin"}, City{"Dallas"}, City{"Houston"}, City{"San Antonio"} }},
        {"United States Minor Outlying Islands", {
            {"Baker Island", {}},
            {"Howland Island", {}},
            {"Jarvis Island", {}},
            {"Johnston Atoll", {}},
            {"Kingman Reef", {}},
            {"Midway Atoll", {}},
            {"Navassa Island", {}},
            {"Palmyra Atoll", {}},
            {"Wake Island", {}}
        }}, // Outlying area
        {"United States Virgin Islands", { City{"Charlotte Amalie"} }}, // Outlying area
        {"Utah", { City{"Salt Lake City"} }},
        {"Vermont", { City{"Montpelier"} }},
        {"Virginia", { City{"Richmond"} }},
        {"Washington", { City{"Olympia"}, City{"Seattle"} }},
        {"West Virginia", { City{"Charleston"} }},
        {"Wisconsin", { City{"Madison"} }},
        {"Wyoming", { City{"Cheyenne"} }}
    }},
    {"Unspecified", {}},
    {"Uruguay", {}},
    {"Uzbekistan", {}},
    {"Vanuatu", {}},
    {"Vatican City", {}},
    {"Venezuela", {}},
    {"Vietnam", {}},
    {"Yemen", {}},
    {"Zambia", {}},
    {"Zimbabwe", {}},
    {"Worldwide", {}}
};

//-----------------------------------------------------------------------------

inline std::vector<std::string> get_countries() {
    std::vector<std::string> countries;
    for (const auto& country : locations) {
        countries.push_back(country.first);
    }
    return countries;
}

inline std::vector<std::string> get_regions(const std::string& country) {
    std::vector<std::string> regions;
    auto country_it = locations.find(country);
    if (country_it != locations.end()) {
        for (const auto& region : country_it->second) {
            regions.push_back(region.first);
        }
    }
    return regions;
}

inline std::vector<std::string> get_cities(const std::string& country, const std::string& region) {
    std::vector<std::string> cities;
    auto country_it = locations.find(country);
    if (country_it != locations.end()) {
        auto region_it = country_it->second.find(region);
        if (region_it != country_it->second.end()) {
            for (const auto& city : region_it->second) {
                cities.push_back(city.name);
            }
        }
    }
    return cities;
}

inline std::vector<std::string> get_cities_from_country(const std::string& country) {
    std::vector<std::string> cities;
    std::unordered_map<std::string, std::vector<std::string>> city_map; // Map to track city names

    auto country_it = locations.find(country);
    if (country_it != locations.end()) {
        for (const auto& region : country_it->second) {
            for (const auto& city : region.second) {
                city_map[city.name].push_back(region.first);  // Map city to its region (state)
            }
        }

        // Now we can check for duplicates and decide how to handle them
        for (const auto& city_pair : city_map) {
            if (city_pair.second.size() > 1) {
                // City is present in multiple regions
                std::cout << "Duplicate city detected: " << city_pair.first << std::endl;
                for (const auto& state : city_pair.second) {
                    cities.push_back(city_pair.first + ", " + state);
                }
            } else {
                // City is unique, just add it normally
                cities.push_back(city_pair.first);
            }
        }
    }

    return cities;
}

}

