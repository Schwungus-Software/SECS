#include <iostream>

#include "SECS.hpp"

struct Gamer : Component {
    std::string name;

    Gamer(const std::string name) : name(name) {}
};

void main_startup(Commands cmd) {
    const auto gamer = new Gamer("PewDiePie");
    cmd.push(new Spawn(gamer));

    std::cout << "HERE WE GO GAMERS" << std::endl;
}

void main_update(Query<All<Gamer>> query) {
    for (const auto& entity : query) {
        const auto& name = entity->expect<Gamer>()->name;
        std::cout << "GAMING!!! from " << name << std::endl;
    }
}

const Systems SECS::systems{
    {0, Stage::STARTUP, main_startup}, {0, Stage::UPDATE, main_update}};

int main(int argc, char* argv[]) {
    for (std::size_t i = 0; i < 10; i++) {
        SECS::tick(0);
    }
}
