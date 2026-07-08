#ifndef SRS_LOGGER_HPP
#define SRS_LOGGER_HPP

#include <iosfwd>
#include <string_view>

namespace srs {

// Log severity levels.
enum class LogLevel {
    Info,
    Warn,
    Error,
};

// Returns the name of a LogLevel.
[[nodiscard]] std::string_view to_string(LogLevel level) noexcept;

// Writes timestamped lines like: [YYYY-MM-DD HH:MM:SS] [LEVEL] message.
// The output stream is passed in so it can be swapped out in tests.
class Logger {
public:
    explicit Logger(std::ostream& out);

    void log(LogLevel level, std::string_view message);
    void info(std::string_view message);
    void warn(std::string_view message);
    void error(std::string_view message);

private:
    std::ostream& out_;
};

}  // namespace srs

#endif  // SRS_LOGGER_HPP
