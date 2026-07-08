#ifndef SRS_RECOVERY_ACTION_HPP
#define SRS_RECOVERY_ACTION_HPP

#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

#include "srs/action_types.hpp"

// Use C++20 concepts when available, otherwise fall back to type traits.
#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#  define SRS_HAS_CONCEPTS 1
#  include <concepts>
#else
#  define SRS_HAS_CONCEPTS 0
#endif

namespace srs {

// Base interface for a recovery action.
class IRecoveryAction {
public:
    virtual ~IRecoveryAction() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual ActionKind kind() const noexcept = 0;
    [[nodiscard]] virtual ActionResult execute(const ActionContext& ctx) = 0;
};

// True if T is a concrete type deriving from IRecoveryAction.
template <class T>
inline constexpr bool is_recovery_action_v =
    std::is_base_of_v<IRecoveryAction, T> && !std::is_abstract_v<T>;

#if SRS_HAS_CONCEPTS
// C++20 contract for a concrete recovery action; also checks member signatures.
template <class T>
concept RecoveryAction =
    is_recovery_action_v<T> &&
    requires(T action, const ActionContext& ctx) {
        { action.name() } -> std::convertible_to<std::string_view>;
        { action.kind() } -> std::same_as<ActionKind>;
        { action.execute(ctx) } -> std::same_as<ActionResult>;
    };
#  define SRS_RECOVERY_ACTION ::srs::RecoveryAction
#else
#  define SRS_RECOVERY_ACTION class
#endif

// Creates an action, checking the type at compile time.
template <SRS_RECOVERY_ACTION T, class... Args>
[[nodiscard]] std::unique_ptr<IRecoveryAction> make_action(Args&&... args) {
    static_assert(is_recovery_action_v<T>,
                  "T must be a non-abstract type deriving from IRecoveryAction");
    return std::make_unique<T>(std::forward<Args>(args)...);
}

}  // namespace srs

#endif  // SRS_RECOVERY_ACTION_HPP
