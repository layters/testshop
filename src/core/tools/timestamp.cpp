#include "timestamp.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

std::string neroshop::timestamp::get_current_utc_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now); // current time
    std::stringstream datetime;
    datetime << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
    std::string utc_timestamp = datetime.str();
    return utc_timestamp;
}

bool neroshop::timestamp::is_expired(const std::string& expiration_date) {
    // Get the current UTC time
    std::time_t current_time = std::time(nullptr);
    std::tm* current_tm = std::gmtime(&current_time);

    // Parse the expiration date string
    std::tm expiration_tm{};
    std::istringstream ss(expiration_date);
    ss >> std::get_time(&expiration_tm, "%Y-%m-%dT%H:%M:%SZ");

    // Compare the expiration time with the current time
    return (std::mktime(&expiration_tm) <= std::mktime(current_tm));
}

std::string neroshop::timestamp::get_most_recent_timestamp(const std::string& timestamp1, const std::string& timestamp2) {
    std::tm tm1{};
    std::istringstream ss1(timestamp1);
    ss1 >> std::get_time(&tm1, "%Y-%m-%dT%H:%M:%SZ");
    std::time_t time1 = std::mktime(&tm1);

    std::tm tm2{};
    std::istringstream ss2(timestamp2);
    ss2 >> std::get_time(&tm2, "%Y-%m-%dT%H:%M:%SZ");
    std::time_t time2 = std::mktime(&tm2);

    if (time1 > time2) {
        return timestamp1;
    } else {
        return timestamp2;
    }
}

// Convert Unix timestamp to UTC time
std::tm neroshop::timestamp::unix_timestamp_to_utc(time_t unix_timestamp) {
    std::tm utc_time;
    gmtime_r(&unix_timestamp, &utc_time);
    return utc_time;
}

// Convert UTC time to Unix timestamp
time_t neroshop::timestamp::utc_to_unix_timestamp(const std::string& utc_time_str) {
    std::tm utc_time = {};
    std::istringstream ss(utc_time_str);
    ss >> std::get_time(&utc_time, "%Y-%m-%dT%H:%M:%SZ");
    return timegm(&utc_time);
}
