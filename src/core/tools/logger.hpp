#pragma once
#ifndef LOGGER_HPP_NEROSHOP
#define LOGGER_HPP_NEROSHOP

#include "../../neroshop_config.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip> // std::put_time
#include <mutex>
#include <memory>
#include <atomic>

#include <fmt/base.h> // fmt::print
#include <fmt/core.h>  // fmt::format
#include <fmt/color.h>  // fmt::color

namespace neroshop {

// ANSI color codes
constexpr const char* color_red = "\033[1;91m";
constexpr const char* color_green = "\033[1;32m";
constexpr const char* color_yellow = "\033[1;33m";
constexpr const char* color_blue = "\033[1;34m";
constexpr const char* color_magenta = "\033[1;35m";
constexpr const char* color_cyan = "\033[1;36m";
constexpr const char* color_white = "\033[1;37m";
constexpr const char* color_reset = "\033[0m";
// log.info("This is {}blue{} text.\n", color_blue, color_reset);
    
inline std::string colorize(const std::string& color_code, const std::string& text) {
    return color_code + text + color_reset;
}

enum class LogLevel {
    Trace, Debug, Info, Warn, Error, Critical
};
    
class Logger {
public:
    // Singleton access
    static Logger& instance() {
        static Logger logger;
        return logger;
    } 
    // Usage (static global reference):
    /*
        static auto& log = neroshop::Logger::instance();
        
        log.info("Hello {}", "world");
        log.warn("Low disk space");
    */

    // Set log level
    void set_level(LogLevel level) { level_.store(level); }
    
    void set_log_path(const std::string& path) {
        std::scoped_lock lock(config_mutex_);
        log_path_ = path;
    }
    
    void set_log_to_file(bool enabled) {
        std::scoped_lock lock(config_mutex_);
        log_to_file_ = enabled;
    }

    // Log methods
    template <typename... Args>
    void trace(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Debug, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void info(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Error, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void critical(fmt::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
    }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::atomic<LogLevel> level_{LogLevel::Info};
    
    std::mutex console_mutex_;    // For printing to terminal
    std::mutex file_mutex_;       // For writing to log file
    std::mutex config_mutex_;     // For shared settings like path + file flag
    
    std::string log_path_ = NEROSHOP_DEFAULT_CONFIGURATION_PATH + "/" + NEROSHOP_LOG_FILENAME; // default path
    bool log_to_file_ = true;               // file logging enabled by default

    template <typename... Args>
    void log(LogLevel level, fmt::format_string<Args...> fmt_str, Args&&... args) {
        if (level < level_.load()) return;

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now); // current time
	    std::stringstream ss;
	    ss << std::put_time(std::localtime(&in_time_t), std::string("[%Y-%m-%d %H:%M:%S]").c_str()); // %I = 12h, %p = AM/PM
        std::string datetime = ss.str();
        
        const char* color = "";
        const char* prefix = "";
        
        switch(level) {
            case LogLevel::Trace:    color = color_cyan;    prefix = "[TRACE]";    break;
            case LogLevel::Debug:    color = color_blue;    prefix = "[DEBUG]";    break;
            case LogLevel::Info:     color = color_green;   prefix = "[INFO]";     break;
            case LogLevel::Warn:     color = color_yellow;  prefix = "[WARN]";     break;
            case LogLevel::Error:    color = color_red;     prefix = "[ERROR]";    break;
            case LogLevel::Critical: color = color_magenta; prefix = "[CRITICAL]"; break;
        }

        // --- Console output ---
        // Terminal output (with color)
        {
            std::scoped_lock lock(console_mutex_);
            fmt::print("{}{}{}{}{} ", color_white, datetime, color, prefix, color_reset);
            fmt::print(fmt_str, std::forward<Args>(args)...);
            fmt::print("\n");
        }
        
        // --- File output ---
        std::string path_copy;
        bool to_file;
        {
            std::scoped_lock lock(config_mutex_);
            path_copy = log_path_;
            to_file = log_to_file_;
        }

         // Append to log file (no color)
        if (to_file) {
            std::scoped_lock lock(file_mutex_);
            std::ofstream logfile(path_copy.c_str(), std::ios_base::app);
            if (logfile.is_open()) {
                logfile << datetime << prefix << " ";
                logfile << fmt::format(fmt_str, std::forward<Args>(args)...) << std::endl;
            }
        }
    }
};
    /*// for C++20 or later, you can even define templated function aliases
    #if defined(__cplusplus) && (__cplusplus >= 202002L)
    template <typename... Args>
    void log(LogLevel level, fmt::format_string<Args...> fmt, Args&&... args) {
        Logger::instance().log(level, fmt, std::forward<Args>(args)...);
    }
    #endif*/
    // Usage: neroshop::log(neroshop::LogLevel::Info, "Something happened: {}", 42);
    
    #if defined(__cplusplus) && (__cplusplus >= 202002L)
    inline void log_trace(auto&&... args)    { Logger::instance().trace(std::forward<decltype(args)>(args)...); }
    inline void log_debug(auto&&... args)    { Logger::instance().debug(std::forward<decltype(args)>(args)...); }
    inline void log_info(auto&&... args)     { Logger::instance().info(std::forward<decltype(args)>(args)...); }
    inline void log_warn(auto&&... args)     { Logger::instance().warn(std::forward<decltype(args)>(args)...); }
    inline void log_error(auto&&... args)    { Logger::instance().error(std::forward<decltype(args)>(args)...); }
    inline void log_critical(auto&&... args) { Logger::instance().critical(std::forward<decltype(args)>(args)...); }
    #else
    template <typename... Args>
    inline void log_trace(Args&&... args) {
        Logger::instance().trace(std::forward<Args>(args)...);
    }
    template <typename... Args>
    inline void log_debug(Args&&... args) {
        Logger::instance().debug(std::forward<Args>(args)...);
    }
    template <typename... Args>
    inline void log_info(Args&&... args) {
        Logger::instance().info(std::forward<Args>(args)...);
    }
    template <typename... Args>
    inline void log_warn(Args&&... args) {
        Logger::instance().warn(std::forward<Args>(args)...);
    }
    template <typename... Args>
    inline void log_error(Args&&... args) {
        Logger::instance().error(std::forward<Args>(args)...);
    }
    template <typename... Args>
    inline void log_critical(Args&&... args) {
        Logger::instance().critical(std::forward<Args>(args)...);
    }
    #endif
    // Usage: neroshop::log_info("User {} logged in", "bob");  neroshop::log_error("File not found: {}", "config.json");
    
/*#define LOG_TRACE(...)    Logger::instance().trace(__VA_ARGS__)
#define LOG_DEBUG(...)    Logger::instance().debug(__VA_ARGS__)
#define LOG_INFO(...)     Logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...)     Logger::instance().warn(__VA_ARGS__)
#define LOG_ERROR(...)    Logger::instance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) Logger::instance().critical(__VA_ARGS__)*/

}

#endif
