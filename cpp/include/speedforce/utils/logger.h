#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace speedforce {
namespace utils {

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class Logger {
public:
    static Logger& get_instance() {
        static Logger instance;
        return instance;
    }

    void set_level(LogLevel level) {
        std::lock_guard<std::mutex> lock(_mutex);
        _level = level;
    }

    template <typename... Args>
    void log(LogLevel level, Args... args) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (level < _level) return;

        std::stringstream ss;
        print_timestamp(ss);
        print_level(ss, level);
        ((ss << args), ...); // Fold expression (C++17)
        ss << "\n";

        if (level == LogLevel::ERROR) {
            std::cerr << ss.str();
        } else {
            std::cout << ss.str();
        }
    }

    template <typename... Args>
    void info(Args... args) { log(LogLevel::INFO, args...); }

    template <typename... Args>
    void warn(Args... args) { log(LogLevel::WARN, args...); }

    template <typename... Args>
    void error(Args... args) { log(LogLevel::ERROR, args...); }

    template <typename... Args>
    void debug(Args... args) { log(LogLevel::DEBUG, args...); }

private:
    Logger() : _level(LogLevel::INFO) {}
    
    LogLevel _level;
    std::mutex _mutex;

    void print_timestamp(std::stringstream& ss) {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    }

    void print_level(std::stringstream& ss, LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: ss << "[DEBUG] "; break;
            case LogLevel::INFO:  ss << "[INFO]  "; break;
            case LogLevel::WARN:  ss << "[WARN]  "; break;
            case LogLevel::ERROR: ss << "[ERROR] "; break;
        }
    }
};

} // namespace utils
} // namespace speedforce

// Global convenience macros
#define SF_LOG_INFO(...)  speedforce::utils::Logger::get_instance().info(__VA_ARGS__)
#define SF_LOG_WARN(...)  speedforce::utils::Logger::get_instance().warn(__VA_ARGS__)
#define SF_LOG_ERROR(...) speedforce::utils::Logger::get_instance().error(__VA_ARGS__)
#define SF_LOG_DEBUG(...) speedforce::utils::Logger::get_instance().debug(__VA_ARGS__)
