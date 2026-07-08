#ifndef SRS_ACTIONS_HPP
#define SRS_ACTIONS_HPP

#include <string>
#include <string_view>

#include "srs/recovery_action.hpp"

namespace srs {

// Restarts the service.
class RestartAction final : public IRecoveryAction {
public:
    std::string_view name() const noexcept override;
    ActionKind kind() const noexcept override;
    ActionResult execute(const ActionContext& ctx) override;
};

// Stops the service.
class StopAction final : public IRecoveryAction {
public:
    std::string_view name() const noexcept override;
    ActionKind kind() const noexcept override;
    ActionResult execute(const ActionContext& ctx) override;
};

// Disables the service.
class DisableAction final : public IRecoveryAction {
public:
    std::string_view name() const noexcept override;
    ActionKind kind() const noexcept override;
    ActionResult execute(const ActionContext& ctx) override;
};

// Notifies a target, such as an operator or team.
class NotifyAction final : public IRecoveryAction {
public:
    explicit NotifyAction(std::string target = "ops-team");

    std::string_view name() const noexcept override;
    ActionKind kind() const noexcept override;
    ActionResult execute(const ActionContext& ctx) override;

    [[nodiscard]] const std::string& target() const noexcept { return target_; }

private:
    std::string target_;
};

}  // namespace srs

#endif  // SRS_ACTIONS_HPP
