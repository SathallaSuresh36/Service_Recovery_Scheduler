#ifndef SRS_RECOVERY_FACTORY_HPP
#define SRS_RECOVERY_FACTORY_HPP

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "srs/recovery_action.hpp"

namespace srs {

// Action types the factory can create.
enum class ActionType {
    Restart,
    Stop,
    Disable,
    Notify,
};

// Returns the name of an ActionType.
[[nodiscard]] std::string_view to_string(ActionType type) noexcept;

// Creates a recovery action from a type chosen at runtime.
class RecoveryFactory {
public:
    // Creates the action for `type` (NotifyAction uses its default target).
    // Throws std::invalid_argument for an unknown type.
    [[nodiscard]] static std::unique_ptr<IRecoveryAction> create(
        ActionType type);

    // Same as above, but sets the target for a NotifyAction.
    [[nodiscard]] static std::unique_ptr<IRecoveryAction> create(
        ActionType type, std::string notify_target);

    // Parses an action name (case-insensitive). Returns nullopt if unknown.
    [[nodiscard]] static std::optional<ActionType> parse(
        std::string_view name);
};

}  // namespace srs

#endif  // SRS_RECOVERY_FACTORY_HPP
