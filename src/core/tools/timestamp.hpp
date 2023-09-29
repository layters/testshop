#pragma once

#include <iostream>
#include <string>
#include <ctime>

namespace neroshop {

namespace timestamp {

    std::string get_current_utc_timestamp();
    std::string get_most_recent_timestamp(const std::string& timestamp1, const std::string& timestamp2);
    bool is_expired(const std::string& expiration_date);
    std::string get_utc_timestamp_after_duration(int duration, const std::string& time_unit);
    
    std::tm unix_timestamp_to_utc(time_t unix_timestamp);
    time_t utc_to_unix_timestamp(const std::string& utc_time_str);

}

}
