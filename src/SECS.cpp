#include "SECS.hpp"

Systems SECS::systems;
std::vector<std::shared_ptr<Entity>> SECS::entities;

const Stage SECS::Enter(0), SECS::Update(1);

CommandQueue SECS::cmd_queue;

ExecutionContext SECS::exec_context{
    .stage = SECS::Enter,
    .state = State{},
};

void SECS::tick(State state) {
    exec_context.stage =
        exec_context.state == state ? SECS::Update : SECS::Enter;
    exec_context.state = state;

    for (const auto& sys : systems) {
        sys->tick();
    }

    for (const auto& command : cmd_queue) {
        command->perform();
    }

    cmd_queue.clear();
}

SystemBuilder SECS::add(System system) {
    return SystemBuilder(system);
}
