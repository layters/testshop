#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <unordered_map>

namespace neroshop {

inline const std::map<std::string, std::map<std::string, std::vector<std::string>>> locations = {
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
        {"New South Wales", {"Sydney"}},
        {"Queensland", {"Brisbane"}},
        {"South Australia", {"Adelaide"}},
        {"Tasmania", {"Hobart"}},
        {"Victoria", {"Melbourne"}},
        {"Western Australia", {"Perth"}},
        {"Australian Capital Territory", {"Canberra"}},
        {"Northern Territory", {"Darwin"}}
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
        {"Alberta", {"Calgary", "Edmonton"}},
        {"British Columbia", {"Vancouver", "Victoria"}},
        {"Manitoba", {"Winnipeg"}},
        {"New Brunswick", {"Fredericton", "Moncton"}},
        {"Newfoundland and Labrador", {"St. John's"}},
        {"Nova Scotia", {"Halifax"}},
        {"Ontario", {"Ottawa", "Toronto"}},
        {"Prince Edward Island", {"Charlottetown"}},
        {"Quebec", {"Montreal", "Quebec City"}},
        {"Saskatchewan", {"Regina", "Saskatoon"}},
        
        // Territories
        {"Northwest Territories", {"Yellowknife"}},
        {"Nunavut", {"Iqaluit"}},
        {"Yukon", {"Whitehorse"}}
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
        {"Baden-Württemberg", {"Stuttgart"}},
        {"Bavaria (Bayern)", {"Munich (München)"}},
        {"Berlin", {"Berlin"}}, // Both a city and a state
        {"Brandenburg", {"Potsdam"}},
        {"Bremen", {"Bremen"}}, // Both a city and a state
        {"Hamburg", {"Hamburg"}}, // Both a city and a state
        {"Hesse (Hessen)", {"Wiesbaden"}},
        {"Lower Saxony (Niedersachsen)", {"Hanover (Hannover)"}},
        {"Mecklenburg-Western Pomerania (Mecklenburg-Vorpommern)", {"Schwerin"}},
        {"North Rhine-Westphalia (Nordrhein-Westfalen)", {"Düsseldorf"}},
        {"Rhineland-Palatinate (Rheinland-Pfalz)", {"Mainz"}},
        {"Saarland", {"Saarbrücken"}},
        {"Saxony (Sachsen)", {"Dresden"}},
        {"Saxony-Anhalt (Sachsen-Anhalt)", {"Magdeburg"}},
        {"Schleswig-Holstein", {"Kiel"}},
        {"Thuringia (Thüringen)", {"Erfurt"}}
    }},
    {"Ghana", {
        {"Ahafo", {"Goaso"}},
        {"Ashanti", {"Kumasi"}},
        {"Bono", {"Sunyani"}},
        {"Bono East", {"Techiman"}},
        {"Central", {"Abura Dunkwa", "Afransi", "Agona Swedru", "Ajumako", "Apam", "Assin Breku", "Assin Fosu", "Awutu Breku", "Bremang Asikuma", "Cape Coast", "Diaso", "Dunkwa-on-Offin", "Elmina", "Essarkyir", "Kasoa", "Nsaba", "Nsuaem Kyekyewere", "Potsin", "Saltpond", "Twifo Hemang", "Twifo Praso", "Winneba"}},
        {"Eastern", {"Koforidua"}},
        {"Greater Accra", {"Abokobi", "Accra", "Accra New Town", "Ada Foah", "Adenta", "Amasaman", "Ashaiman", "Dansoman", "Darkuman", "Dodowa", "Dzorwulu", "Kokomlemle", "Kpone", "La", "Larteboikorshie", "Madina", "Ngleshie Amanfro", "Nima", "Nungua", "Ofankor", "Osu", "Prampram", "Sege", "Sowutuom", "Tema", "Tema Community 18", "Tesano", "Teshie", "Weija"}},
        {"Northern", {"Tamale"}},
        {"North East", {"Nalerigu"}},
        {"Oti", {"Dambai"}},
        {"Savannah", {"Damongo"}},
        {"Upper East", {"Bolgatanga"}},
        {"Upper West", {"Wa"}},
        {"Volta", {"Ho"}},
        {"Western", {"Sekondi-Takoradi"}},
        {"Western North", {"Sefwi Wiawso"}}
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
        {"England", {"London"}},
        {"Northern Ireland", {"Belfast"}},
        {"Scotland", {"Edinburgh", "Glasgow"}},
        {"Wales", {"Cardiff"}},
    }},
    {"United States", {
        {"Alabama", {"Montgomery"}},
        {"Alaska", {"Juneau"}},
        {"American Samoa", {"Pago Pago"}}, // Outlying area
        {"Arizona", {"Phoenix"}},
        {"Arkansas", {"Little Rock"}},
        {"California", {"Los Angeles", "Sacramento", "San Diego", "San Francisco"}},
        {"Colorado", {"Denver"}},
        {"Connecticut", {"Hartford"}},
        {"Delaware", {"Dover"}},
        {"Florida", {"Miami", "Tallahassee"}},
        {"Georgia", {"Atlanta"}},
        {"Guam", {"Hagåtña"}}, // Outlying area
        {"Hawaii", {"Honolulu"}},
        {"Idaho", {"Boise"}},
        {"Illinois", {"Chicago", "Springfield"}},
        {"Indiana", {"Indianapolis"}},
        {"Iowa", {"Des Moines"}},
        {"Kansas", {"Topeka"}},
        {"Kentucky", {"Frankfort"}},
        {"Louisiana", {"Baton Rouge"}},
        {"Maine", {"Augusta"}},
        {"Maryland", {"Annapolis"}},
        {"Massachusetts", {"Boston", "Springfield", "Worcester"}},
        {"Michigan", {"Lansing"}},
        {"Minnesota", {"Saint Paul"}},
        {"Mississippi", {"Jackson"}},
        {"Missouri", {"Jefferson City"}},
        {"Montana", {"Helena"}},
        {"Nebraska", {"Lincoln"}},
        {"Nevada", {"Carson City", "Las Vegas"}},
        {"New Hampshire", {"Concord"}},
        {"New Jersey", {"Trenton"}},
        {"New Mexico", {"Santa Fe"}},
        {"New York", {"Albany", "Buffalo", "New York City", "Rochester"}},
        {"North Carolina", {"Raleigh"}},
        {"North Dakota", {"Bismarck"}},
        {"Northern Mariana Islands", {"Saipan"}}, // Outlying area
        {"Ohio", {"Columbus"}},
        {"Oklahoma", {"Oklahoma City"}},
        {"Oregon", {"Salem"}},
        {"Pennsylvania", {"Harrisburg", "Philadelphia"}},
        {"Puerto Rico", {"San Juan"}}, // Outlying area
        {"Rhode Island", {"Providence"}},
        {"South Carolina", {"Columbia"}},
        {"South Dakota", {"Pierre"}},
        {"Tennessee", {"Nashville"}},
        {"Texas", {"Austin", "Dallas", "Houston", "San Antonio"}},
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
        {"United States Virgin Islands", {"Charlotte Amalie"}}, // Outlying area
        {"Utah", {"Salt Lake City"}},
        {"Vermont", {"Montpelier"}},
        {"Virginia", {"Richmond"}},
        {"Washington", {"Olympia", "Seattle"}},
        {"West Virginia", {"Charleston"}},
        {"Wisconsin", {"Madison"}},
        {"Wyoming", {"Cheyenne"}}
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
            // Add cities in the specified region to the vector
            cities = region_it->second;
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
                city_map[city].push_back(region.first);  // Map city to its region (state)
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

