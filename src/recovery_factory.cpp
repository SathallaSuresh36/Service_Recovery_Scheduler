#include "srs/recovery_factory.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

#include "srs/actions.hpp"

namespace srs {

std::string_view to_string(ActionType type) noexcept {
    switch (type) {
        case ActionType::Restart:
            return "Restart";
        case ActionType::Stop:
            return "Stop";
        case ActionType::Disable:
            return "Disable";
        case ActionType::Notify:
            return "Notify";
    }
    return "Unknown";
}

std::unique_ptr<IRecoveryAction> RecoveryFactory::create(ActionType type) {
    switch (type) {
        case ActionType::Restart:
            return std::make_unique<RestartAction>();
        case ActionType::Stop:
            return std::make_unique<StopAction>();
        case ActionType::Disable:
            return std::make_unique<DisableAction>();
        case ActionType::Notify:
            return std::make_unique<NotifyAction>();
    }
    throw std::invalid_argument("unknown ActionType");
}

std::unique_ptr<IRecoveryAction> RecoveryFactory::create(
    ActionType type, std::string notify_target) {
    if (type == ActionType::Notify) {
        return std::make_unique<NotifyAction>(std::move(notify_target));
    }
    return create(type);
}

std::optional<ActionType> RecoveryFactory::parse(std::string_view name) {
    std::string lowered(name);
    std::transform(lowered.begin(), lowered.end(), lowered.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });

    if (lowered == "restart") {
        return ActionType::Restart;
    }
    if (lowered == "stop") {
        return ActionType::Stop;
    }
    if (lowered == "disable") {
        return ActionType::Disable;
    }
    if (lowered == "notify") {
        return ActionType::Notify;
    }
    return std::nullopt;
}

}  // namespace srs
