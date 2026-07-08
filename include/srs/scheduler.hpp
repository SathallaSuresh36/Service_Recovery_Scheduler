#ifndef SRS_SCHEDULER_HPP
#define SRS_SCHEDULER_HPP

#include <string>
#include <string_view>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "srs/recovery_action.hpp"
#include "srs/service.hpp"

namespace srs {

// True if F can be called as (const ServiceState&, const ActionResult&).
template <class F>
inline constexpr bool is_action_observer_v =
    std::is_invocable_v<F, const ServiceState&, const ActionResult&>;

#if SRS_HAS_CONCEPTS
// C++20 form of the observer contract.
template <class F>
concept ActionObserver = is_action_observer_v<F>;
#  define SRS_ACTION_OBSERVER ::srs::ActionObserver
#else
#  define SRS_ACTION_OBSERVER class
#endif

// Builds a service and its recovery sequence with chained with<T>() calls.
class ServiceBuilder {
public:
    explicit ServiceBuilder(std::string name);

    template <SRS_RECOVERY_ACTION T, class... Args>
    ServiceBuilder& with(Args&&... args) {
        sequence_.push_back(make_action<T>(std::forward<Args>(args)...));
        return *this;
    }

    Service build();

private:
    std::string name_;
    std::vector<std::unique_ptr<IRecoveryAction>> sequence_;
};

// Holds the registered services and runs the right action on a failure.
// Thread-safe.
class Scheduler {
public:
    // Registers a service. Throws std::invalid_argument on a duplicate name.
    void register_service(Service service);

    [[nodiscard]] bool has_service(std::string_view name) const;
    [[nodiscard]] std::size_t size() const {
        const std::lock_guard<std::mutex> lock(mutex_);
        return services_.size();
    }

    // Reports a failure for a service and returns the action taken.
    // Throws std::out_of_range if the service is unknown.
    ActionResult report_failure(std::string_view name);

    // Overload that also calls an observer with the resulting state.
    template <SRS_ACTION_OBSERVER Observer>
    ActionResult report_failure(std::string_view name, Observer&& obs) {
        static_assert(is_action_observer_v<Observer>,
                      "Observer must be callable as "
                      "void(const ServiceState&, const ActionResult&)");
        ActionResult result = report_failure(name);
        std::forward<Observer>(obs)(state(name), result);
        return result;
    }

    // Marks a service recovered and resets its escalation.
    void mark_recovered(std::string_view name);

    [[nodiscard]] ServiceState state(std::string_view name) const;
    [[nodiscard]] std::vector<ServiceState> snapshot() const;

private:
    [[nodiscard]] Service& find(std::string_view name);
    [[nodiscard]] const Service& find(std::string_view name) const;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Service> services_;
};

}  // namespace srs

#endif  // SRS_SCHEDULER_HPP
