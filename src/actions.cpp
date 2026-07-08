#include "srs/actions.hpp"

#include <ostream>
#include <string>
#include <utility>

namespace srs {

std::string_view to_string(ActionKind kind) noexcept {
    switch (kind) {
        case ActionKind::Restart:
            return "Restart";
        case ActionKind::Stop:
            return "Stop";
        case ActionKind::Disable:
            return "Disable";
        case ActionKind::Notify:
            return "Notify";
    }
    return "Unknown";
}

std::ostream& operator<<(std::ostream& os, ActionKind kind) {
    return os << to_string(kind);
}

namespace {

std::string with_service(std::string_view verb, const ActionContext& ctx) {
    return std::string(verb) + " service '" + std::string(ctx.service_name) +
           "' (attempt " + std::to_string(ctx.failure_count) + ")";
}

}  // namespace

// --- RestartAction ---------------------------------------------------------
std::string_view RestartAction::name() const noexcept { return "restart"; }
ActionKind RestartAction::kind() const noexcept { return ActionKind::Restart; }
ActionResult RestartAction::execute(const ActionContext& ctx) {
    return ActionResult{ActionKind::Restart, std::string(name()), true,
                        with_service("restarting", ctx)};
}

// --- StopAction ------------------------------------------------------------
std::string_view StopAction::name() const noexcept { return "stop"; }
ActionKind StopAction::kind() const noexcept { return ActionKind::Stop; }
ActionResult StopAction::execute(const ActionContext& ctx) {
    return ActionResult{ActionKind::Stop, std::string(name()), true,
                        with_service("stopping", ctx)};
}

// --- DisableAction ---------------------------------------------------------
std::string_view DisableAction::name() const noexcept { return "disable"; }
ActionKind DisableAction::kind() const noexcept { return ActionKind::Disable; }
ActionResult DisableAction::execute(const ActionContext& ctx) {
    return ActionResult{ActionKind::Disable, std::string(name()), true,
                        with_service("disabling", ctx)};
}

// --- NotifyAction ----------------------------------------------------------
NotifyAction::NotifyAction(std::string target) : target_(std::move(target)) {}
std::string_view NotifyAction::name() const noexcept { return "notify"; }
ActionKind NotifyAction::kind() const noexcept { return ActionKind::Notify; }
ActionResult NotifyAction::execute(const ActionContext& ctx) {
    return ActionResult{
        ActionKind::Notify, std::string(name()), true,
        "notifying '" + target_ + "' about service '" +
            std::string(ctx.service_name) + "' (attempt " +
            std::to_string(ctx.failure_count) + ")"};
}

}  // namespace srs
