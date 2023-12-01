#include "SECS.hpp"

Entities SECS::entities;
CommandQueue SECS::cmd_queue;

void SECS::tick(std::size_t state_idx) {
    // TODO: replace with the max value for `std::size_t`.
    static std::size_t prev_state_idx = 1 << 16;

    for (const auto& sys : systems) {
        if (sys->state_idx != state_idx) {
            continue;
        }

        const auto can_startup = prev_state_idx != state_idx;
        const auto can_update = prev_state_idx == state_idx;

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
