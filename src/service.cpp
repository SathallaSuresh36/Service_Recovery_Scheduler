#include "srs/service.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace srs {

Service::Service(std::string name,
                 std::vector<std::unique_ptr<IRecoveryAction>> sequence)
    : name_(std::move(name)), sequence_(std::move(sequence)) {
    if (name_.empty()) {
        throw std::invalid_argument("service name must not be empty");
    }
    if (sequence_.empty()) {
        throw std::invalid_argument("recovery sequence must not be empty");
    }
    for (const auto& action : sequence_) {
        if (action == nullptr) {
            throw std::invalid_argument("recovery action must not be null");
        }
    }
}

ActionResult Service::on_failure() {
    ++failure_count_;
    const std::size_t index =
        std::min(failure_count_ - 1, sequence_.size() - 1);

    const ActionContext ctx{name_, failure_count_, index};
    ActionResult result = sequence_[index]->execute(ctx);

    current_level_ = index;
    last_action_ = result.action_name;
    last_action_kind_ = result.kind;
    return result;
}

void Service::reset() noexcept {
    failure_count_ = 0;
    current_level_.reset();
    last_action_.reset();
    last_action_kind_.reset();
}

ServiceState Service::state() const {
    ServiceState s;
    s.name = name_;
    s.failure_count = failure_count_;
    s.sequence_size = sequence_.size();
    s.level = current_level_.value_or(0);
    s.at_max_level =
        current_level_.has_value() && *current_level_ == sequence_.size() - 1;
    s.last_action = last_action_;
    s.last_action_kind = last_action_kind_;

    // The action the next failure would run.
    const std::size_t next_index =
        std::min(failure_count_, sequence_.size() - 1);
    s.next_action = std::string(sequence_[next_index]->name());
    s.next_action_kind = sequence_[next_index]->kind();

    // Names of all actions in the sequence.
    s.sequence.reserve(sequence_.size());
    for (const auto& action : sequence_) {
        s.sequence.emplace_back(to_string(action->kind()));
    }
    return s;
}

}  // namespace srs
