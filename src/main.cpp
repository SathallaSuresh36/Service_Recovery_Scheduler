#include <cctype>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "srs/logger.hpp"
#include "srs/recovery_action.hpp"
#include "srs/recovery_factory.hpp"
#include "srs/scheduler.hpp"

namespace {

using srs::ActionKind;
using srs::ActionResult;
using srs::ActionType;
using srs::IRecoveryAction;
using srs::Logger;
using srs::RecoveryFactory;
using srs::Scheduler;
using srs::Service;
using srs::ServiceState;

// Reads a line from stdin. Returns nullopt at end of input, so the loop
// stops cleanly when driven from a pipe or with Ctrl-D / Ctrl-Z.
std::optional<std::string> read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
        return std::nullopt;
    }
    return line;
}

std::string to_upper(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (const char c : text) {
        out.push_back(
            static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return out;
}

// Friendly status line, e.g. "Restarting CameraService...".
std::string action_flavour(ActionKind kind, const std::string& service) {
    switch (kind) {
        case ActionKind::Restart:
            return "Restarting " + service + "...";
        case ActionKind::Stop:
            return "Stopping " + service + "...";
        case ActionKind::Disable:
            return "Disabling " + service + "...";
        case ActionKind::Notify:
            return "Notifying operators about " + service + "...";
    }
    return "Handling " + service + "...";
}

void print_menu() {
    std::cout << "\n==================================\n"
              << " Service Recovery Scheduler\n"
              << "==================================\n"
              << "1. Register Service\n"
              << "2. Report Failure\n"
              << "3. Query Service\n"
              << "4. List Services\n"
              << "5. Reset Service\n"
              << "6. Exit\n"
              << "----------------------------------\n";
}

// Maps a menu key to an ActionType.
std::optional<ActionType> action_from_choice(const std::string& choice) {
    if (choice == "1") {
        return ActionType::Restart;
    }
    if (choice == "2") {
        return ActionType::Stop;
    }
    if (choice == "3") {
        return ActionType::Disable;
    }
    if (choice == "4") {
        return ActionType::Notify;
    }
    return std::nullopt;
}

void register_service(Scheduler& scheduler, Logger& log) {
    const auto name = read_line("Service name: ");
    if (!name) {
        return;
    }

    std::cout << "Add actions in order (blank line or 0 to finish):\n"
              << "  1 Restart   2 Stop   3 Disable   4 Notify\n";

    std::vector<std::unique_ptr<IRecoveryAction>> sequence;
    while (true) {
        const auto choice = read_line("  action> ");
        if (!choice || choice->empty() || *choice == "0") {
            break;
        }
        const auto type = action_from_choice(*choice);
        if (!type) {
            std::cout << "  (unknown action, try again)\n";
            continue;
        }
        if (*type == ActionType::Notify) {
            const auto target = read_line("    notify target: ");
            sequence.push_back(RecoveryFactory::create(
                *type, target && !target->empty() ? *target : "ops-team"));
        } else {
            sequence.push_back(RecoveryFactory::create(*type));
        }
    }

    try {
        scheduler.register_service(Service(*name, std::move(sequence)));
        log.info("registered service '" + *name + "'");
    } catch (const std::exception& ex) {
        log.error(std::string("registration failed: ") + ex.what());
    }
}

// Builds a service from an ordered list of action types.
Service make_service(const std::string& name,
                     const std::vector<ActionType>& types) {
    std::vector<std::unique_ptr<IRecoveryAction>> sequence;
    sequence.reserve(types.size());
    for (const ActionType type : types) {
        sequence.push_back(RecoveryFactory::create(type));
    }
    return Service(name, std::move(sequence));
}

// Registers the sample vehicle services and their recovery sequences.
void register_vehicle_services(Scheduler& scheduler, Logger& log) {
    const auto R = ActionType::Restart;
    const auto S = ActionType::Stop;
    const auto D = ActionType::Disable;

    scheduler.register_service(make_service("CameraService", {R, R, S, D}));
    scheduler.register_service(make_service("GPSService", {R, S, D}));
    scheduler.register_service(make_service("BluetoothService", {R, R, R, D}));
    scheduler.register_service(make_service("AudioService", {R, S, S, D}));

    log.info("registered 4 in-vehicle services");
}

void report_failure(Scheduler& scheduler, Logger& log) {
    const auto name = read_line("Enter Service Name: ");
    if (!name) {
        return;
    }
    try {
        const ActionResult result = scheduler.report_failure(*name);
        const ServiceState s = scheduler.state(*name);

        std::cout << "\nFailure received for " << s.name << '\n';
        // A clamped repeat happens once failures exceed the sequence length.
        if (s.at_max_level && s.failure_count > s.sequence_size) {
            std::cout << "Service already at maximum recovery level\n"
                      << "Executing Action : "
                      << to_upper(to_string(result.kind)) << " again\n";
        } else {
            std::cout << "Current Recovery Level : " << s.level << '\n'
                      << "Executing Action : "
                      << to_upper(to_string(result.kind)) << '\n';
        }
        std::cout << action_flavour(result.kind, s.name) << '\n'
                  << "Failure Count : " << s.failure_count << '\n';

        log.warn("failure on '" + s.name + "' -> " + result.message);
    } catch (const std::exception& ex) {
        log.error(ex.what());
    }
}

void query_service(const Scheduler& scheduler, Logger& log) {
    const auto name = read_line("Enter Service Name: ");
    if (!name) {
        return;
    }
    try {
        const ServiceState s = scheduler.state(*name);
        const std::string last =
            s.last_action_kind ? std::string(to_string(*s.last_action_kind))
                               : "None";
        const std::string next =
            s.next_action_kind ? std::string(to_string(*s.next_action_kind))
                               : "None";

        std::cout << "\nService Name  : " << s.name << '\n'
                  << "Current Level : " << s.failure_count << '\n'
                  << "Last Action   : " << last << '\n'
                  << "Next Action   : " << next << '\n'
                  << "Failure Count : " << s.failure_count << '\n';
    } catch (const std::exception& ex) {
        log.error(ex.what());
    }
}

void list_services(const Scheduler& scheduler) {
    const auto services = scheduler.snapshot();
    if (services.empty()) {
        std::cout << "(no services registered)\n";
        return;
    }

    std::cout << "\nRegistered Services\n";
    for (const ServiceState& s : services) {
        std::cout << "------------------------------------\n"
                  << s.name << "\n"
                  << "Recovery Sequence\n";
        for (const std::string& action : s.sequence) {
            std::cout << "  " << action << '\n';
        }
    }
    std::cout << "------------------------------------\n";
}

void reset_service(Scheduler& scheduler, Logger& log) {
    const auto name = read_line("Enter Service Name: ");
    if (!name) {
        return;
    }
    try {
        scheduler.mark_recovered(*name);
        std::cout << "\nReset successful.\n";
        log.info("service '" + *name + "' marked recovered");
    } catch (const std::exception& ex) {
        log.error(ex.what());
    }
}

}  // namespace

int main() {
    Scheduler scheduler;
    Logger log(std::cout);

    log.info("Service Recovery Scheduler started");
    register_vehicle_services(scheduler, log);

    while (true) {
        print_menu();
        const auto choice = read_line("Choice : ");
        if (!choice || *choice == "6") {
            break;
        }

        if (*choice == "1") {
            register_service(scheduler, log);
        } else if (*choice == "2") {
            report_failure(scheduler, log);
        } else if (*choice == "3") {
            query_service(scheduler, log);
        } else if (*choice == "4") {
            list_services(scheduler);
        } else if (*choice == "5") {
            reset_service(scheduler, log);
        } else {
            std::cout << "(unknown choice)\n";
        }
    }

    log.info("shutting down");
    return 0;
}
