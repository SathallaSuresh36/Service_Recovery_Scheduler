#ifndef SRS_SERVICE_HPP
#define SRS_SERVICE_HPP

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "srs/recovery_action.hpp"

namespace srs {

// Snapshot of a service's state.
struct ServiceState {
    std::string name;
    std::size_t failure_count = 0;   // failures since the last reset
    std::size_t level = 0;           // index of the last action run
    std::size_t sequence_size = 0;
    bool at_max_level = false;       // true once the last action is reached
    std::optional<std::string> last_action;
    std::optional<ActionKind> last_action_kind;
    std::optional<std::string> next_action;       // action the next failure runs
    std::optional<ActionKind> next_action_kind;
    std::vector<std::string> sequence;   // names of the actions, in order
};

// A service with its ordered list of recovery actions.
// The Nth failure runs the action at index min(N-1, size-1); once at the last
// action it stays there. reset() goes back to the start.
class Service {
public:
    Service(std::string name,
            std::vector<std::unique_ptr<IRecoveryAction>> sequence);

    // Owns unique_ptrs: non-copyable, movable.
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    Service(Service&&) noexcept = default;
    Service& operator=(Service&&) noexcept = default;

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] std::size_t sequence_size() const noexcept {
        return sequence_.size();
    }

    // Records a failure, runs the next action and escalates.
    ActionResult on_failure();

    // Marks the service recovered and restarts from the first action.
    void reset() noexcept;

    [[nodiscard]] ServiceState state() const;

private:
    std::string name_;
    std::vector<std::unique_ptr<IRecoveryAction>> sequence_;
    std::size_t failure_count_ = 0;
    std::optional<std::size_t> current_level_;
    std::optional<std::string> last_action_;
    std::optional<ActionKind> last_action_kind_;
};

}  // namespace srs

#endif  // SRS_SERVICE_HPP
