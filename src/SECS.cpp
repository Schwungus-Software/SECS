#include "SECS.hpp"

// TODO: replace with the max value for `std::size_t`.
std::size_t SECS::prev_state_idx = 1 << 16;

Entities SECS::entities;
CommandQueue SECS::cmd_queue;

void SECS::tick(std::size_t state_idx) {
    for (const auto& sys : systems) {
        const auto can_startup = prev_state_idx != sys->state_idx;
        const auto can_update = prev_state_idx == sys->state_idx;

        const auto tick_startup = sys->stage == Stage::STARTUP && can_startup;
        const auto tick_update = sys->stage == Stage::UPDATE && can_update;

        const auto tick = tick_startup || tick_update;

        if (tick) {
            sys->tick();
        }
    }

    for (const auto& command : cmd_queue) {
        command->perform();
    }

    cmd_queue.clear();

    prev_state_idx = state_idx;
}
