#ifndef SRS_ACTION_TYPES_HPP
#define SRS_ACTION_TYPES_HPP

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>

namespace srs {

// Kinds of recovery action.
enum class ActionKind {
    Restart,
    Stop,
    Disable,
    Notify,
};

// Returns the name of an ActionKind.
[[nodiscard]] std::string_view to_string(ActionKind kind) noexcept;

std::ostream& operator<<(std::ostream& os, ActionKind kind);

// Passed to an action when it runs.
struct ActionContext {
    std::string_view service_name;
    std::size_t failure_count;  // consecutive failures, starting at 1
    std::size_t level;          // index into the recovery sequence
};

// Result of running a recovery action.
struct ActionResult {
    ActionKind kind;
    std::string action_name;
    bool success;
    std::string message;
};

}  // namespace srs

#endif  // SRS_ACTION_TYPES_HPP
