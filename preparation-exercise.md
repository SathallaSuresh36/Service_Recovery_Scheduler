# C++ Take-Home Exercise — Service Recovery Scheduler

Build a small C++ library (plus a minimal runnable entry point) that manages recovery actions for a set of monitored services.

## Core idea

Each service has a name and an ordered list of recovery actions (e.g. `RESTART → RESTART → STOP → DISABLE`). When a service reports a failure, the scheduler selects and executes the next action in its sequence. Consecutive failures escalate through the list.

## Requirements

- Services and their recovery sequences ar registered at startup
- The scheduler receives failure notifications per service and dispatches the appropriate action
- At least 3 distinct recovery actions must be supported; their implementations are dummies — no real system calls expected
- Querying the current state of a service (current level, last action taken) should be supported
- How services are registered and how failures are fed in is your choice — justify it briefly

## Deliverables

- Production quality, buildable, modern C++ code
- Unit tests — production quality
- A short README (what it does, how to build, any design decisions worth noting)
- A small git repository showing your work in commits — not a single squash

## Notes

- AI tools are fine and expected; what we evaluate is your standards, not AI output alone
- Loose requirements are intentional — note your assumptions and decisions briefly
- Estimated effort: 1–2 hours
