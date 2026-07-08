# Service Recovery Scheduler

This is a small C++ library with a demo program. It helps recover services when
they fail.

Every service has a list of actions to try when something goes wrong. The first
time a service fails, it runs the first action. If it keeps failing, it moves on
to the next action in the list. So the response gets stronger each time.

For example, a service might be set up like this:

```text
RESTART -> RESTART -> STOP -> DISABLE
```

The first two failures try a restart. The third one stops the service. The
fourth one disables it. If it fails again after that, it just keeps disabling.

## Technologies used

| Area | Technology |
| --- | --- |
| Language | C++17, C++20 |
| Build system | CMake 3.14 |
| Unit testing | GoogleTest and GoogleMock |
| Code coverage | gcovr and gcov |
| Reporting | GoogleTest JUnit XML plus HTML reports for tests and coverage |
| Compiler | Any C++17 or C++20 compiler |
| Standard library | `std::unique_ptr`, `std::optional`, `std::string_view`, `std::unordered_map`, `std::mutex`, `std::chrono` |

## Design patterns

- **Strategy**: each recovery action implements `IRecoveryAction`. A service
  keeps a list of them and runs them without knowing the exact type.
- **Factory**: `RecoveryFactory::create(ActionType)` makes an action from a
  choice made at runtime, for example during interactive registration. There is
  also a compile-time helper, `make_action<T>()`, used by `ServiceBuilder`.
- **Builder**: `ServiceBuilder` puts a service and its action list together with
  a simple, chained `with<T>()` call.
- **Dependency Injection**: `Logger` takes the output stream as a reference, so
  it does not depend on `std::cout` and is easy to test.
- **Observer**: `Scheduler::report_failure(name, observer)` calls back with the
  new state, so logging or metrics can be added without changing the library.

This also keeps the design close to SOLID. New actions can be added without
changing old code, each class does one job, and the scheduler depends on the
`IRecoveryAction` interface instead of the concrete actions.
