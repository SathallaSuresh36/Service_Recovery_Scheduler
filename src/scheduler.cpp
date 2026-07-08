#include "srs/scheduler.hpp"

#include <mutex>
#include <stdexcept>
#include <utility>

namespace srs {

// --- ServiceBuilder --------------------------------------------------------
ServiceBuilder::ServiceBuilder(std::string name) : name_(std::move(name)) {}

Service ServiceBuilder::build() {
    return Service(std::move(name_), std::move(sequence_));
}

// --- Scheduler -------------------------------------------------------------
void Scheduler::register_service(Service service) {
    const std::lock_guard<std::mutex> lock(mutex_);
    std::string key = service.name();
    if (services_.find(key) != services_.end()) {
        throw std::invalid_argument("service already registered: " + key);
    }
    services_.emplace(std::move(key), std::move(service));
}

bool Scheduler::has_service(std::string_view name) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return services_.find(std::string(name)) != services_.end();
}

Service& Scheduler::find(std::string_view name) {
    auto it = services_.find(std::string(name));
    if (it == services_.end()) {
        throw std::out_of_range("unknown service: " + std::string(name));
    }
    return it->second;
}

const Service& Scheduler::find(std::string_view name) const {
    auto it = services_.find(std::string(name));
    if (it == services_.end()) {
        throw std::out_of_range("unknown service: " + std::string(name));
    }
    return it->second;
}

ActionResult Scheduler::report_failure(std::string_view name) {
    const std::lock_guard<std::mutex> lock(mutex_);
    return find(name).on_failure();
}

void Scheduler::mark_recovered(std::string_view name) {
    const std::lock_guard<std::mutex> lock(mutex_);
    find(name).reset();
}

ServiceState Scheduler::state(std::string_view name) const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return find(name).state();
}

std::vector<ServiceState> Scheduler::snapshot() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    std::vector<ServiceState> out;
    out.reserve(services_.size());
    for (const auto& entry : services_) {
        out.push_back(entry.second.state());
    }
    return out;
}

}  // namespace srs
