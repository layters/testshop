#include "timestamp.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <boost/date_time.hpp> // only 'cause monero uses boost

std::string neroshop::timestamp::get_utc_timestamp_after_duration(int duration, const std::string& time_unit) {
    // Get the current UTC time
    boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();

    // Define the time units and their respective durations
    std::unordered_map<std::string, boost::posix_time::time_duration> time_units = {
        {"second", boost::posix_time::seconds(duration)},
        {"minute", boost::posix_time::minutes(duration)},
        {"hour", boost::posix_time::hours(duration)},
        {"day", boost::posix_time::hours(duration * 24)},
        {"month", boost::posix_time::not_a_date_time},  // Placeholder for months
        {"year", boost::posix_time::not_a_date_time}    // Placeholder for years
    };

    if (time_units.find(time_unit) == time_units.end()) {
        throw std::invalid_argument("Invalid time unit.");
    }

    if (time_unit == "month" || time_unit == "year") {
        // Extract the date and time components
        boost::gregorian::date now_date = now.date();
        boost::posix_time::time_duration now_time = now.time_of_day();

        if (time_unit == "month") {
            // Calculate the target time using Boost Date Time's months_duration
            boost::gregorian::date later_date = now_date + boost::gregorian::months(duration);
            boost::posix_time::ptime later(later_date, now_time);
            return boost::posix_time::to_iso_extended_string(later) + "Z";
        } else if (time_unit == "year") {
            // Calculate the target time using Boost Date Time's years_duration
            boost::gregorian::date later_date = now_date + boost::gregorian::years(duration);
            boost::posix_time::ptime later(later_date, now_time);
            return boost::posix_time::to_iso_extended_string(later) + "Z";
        }
    }

    // Calculate the target time using the specified duration
    boost::posix_time::ptime later = now + time_units[time_unit];

    // Format the time
    std::ostringstream oss;
    oss << boost::posix_time::to_iso_extended_string(later) << "Z";

    return oss.str();
}

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

std::string neroshop::timestamp::get_duration_from_now(const std::string& timestamp) {
    // Parse the ISO 8601 timestamp
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    if (ss.fail()) {
        std::cerr << "Failed to parse time" << std::endl;
        return "";
    }
    auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    // Calculate the duration from now to the given timestamp
    auto now = std::chrono::system_clock::now();
    auto duration = time_point - now;

    // Define custom duration types for days and weeks
    using days = std::chrono::duration<int, std::ratio<86400>>;
    using weeks = std::chrono::duration<int, std::ratio<604800>>;

    // Convert the duration to different units
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count();
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    auto total_days = std::chrono::duration_cast<days>(duration).count();
    auto total_weeks = std::chrono::duration_cast<weeks>(duration).count();
    
    // Calculate months and years manually
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::gmtime(&now_time_t);
    int years = tm.tm_year - now_tm.tm_year;
    int months = tm.tm_mon - now_tm.tm_mon;

    if (months < 0) {
        months += 12;
        years -= 1;
    }

    // Return the duration
    if(years > 0) { return ((std::to_string(years) + " years, ") + (std::to_string(months) + " months")); }
    if(total_weeks > 0) { return (std::to_string(total_weeks) + " weeks"); } // and maybe a few days or so?
    if(total_days > 0) { return (std::to_string(total_days) + " days"); }
    if(hours > 0) { return (std::to_string(hours) + " hours"); }
    if(minutes > 0) { return (std::to_string(minutes) + " minutes"); }
    return "";
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
