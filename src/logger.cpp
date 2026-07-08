#include "srs/logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>

namespace srs {

std::string_view to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

namespace {

void write_timestamp(std::ostream& os) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32) && !defined(__CYGWIN__)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    os << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
}

}  // namespace

Logger::Logger(std::ostream& out) : out_(out) {}

void Logger::log(LogLevel level, std::string_view message) {
    out_ << '[';
    write_timestamp(out_);
    out_ << "] [" << to_string(level) << "] " << message << '\n';
}

void Logger::info(std::string_view message) { log(LogLevel::Info, message); }
void Logger::warn(std::string_view message) { log(LogLevel::Warn, message); }
void Logger::error(std::string_view message) { log(LogLevel::Error, message); }

}  // namespace srs
