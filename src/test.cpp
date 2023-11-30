#include <iostream>

#include "SECS.hpp"

enum State : std::size_t {
    GAMING,
};

struct Gamer : Component {
    std::string name;

    Gamer(const std::string name) : name(name) {}
};

void startup(Commands cmd) {
    auto gamer = new Gamer("PewDiePie");
    cmd.push(new Spawn(gamer));

    gamer = new Gamer("Markiplier");
    cmd.push(new Spawn(gamer));

    std::cout << "HERE WE GO GAMERS" << std::endl;
}

static std::size_t counter = 1;

void game(Commands cmd, Query<With<Gamer>> query) {
    std::cout << "---------" << std::endl;

    for (const auto& entity : query) {
        const auto& name = entity->expect<Gamer>()->name;
        std::cout << "GAMING!!! from " << name;

        if (counter == 5 && name == "Markiplier") {
            std::cout << " - Aaaand he died";
            cmd.push(new Delete(entity));
        }

        if (counter == 7 && name == "PewDiePie") {
            std::cout << " - Aaaand he's stripped of his gamer title";
            cmd.push(new Remove<Gamer>(entity));
        }

        std::cout << std::endl;
    }

    counter++;
}

const Systems SECS::systems{
    {State::GAMING, Stage::STARTUP, startup},
    {State::GAMING, Stage::UPDATE, game},
};

int main(int, char**) {
    for (std::size_t i = 0; i < 12; i++) {
        SECS::tick(State::GAMING);
    }

    return 0;
}
