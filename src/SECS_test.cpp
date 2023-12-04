#include <iostream>

#include "SECS.hpp"

const State Gaming;

struct Gamer : public Component {
    std::string name;

    Gamer(const std::string& name) : name(name) {}
};

void testy(Param<Stage> stage) {
    if (stage.value == SECS::Enter) {
        std::cout << "LET'S GO GAMERS!!!" << std::endl;
    } else {
        std::cout << "---------" << std::endl;
    }
}

void startup(Commands cmd) {
    auto gamer = new Gamer("PewDiePie");
    cmd.spawn(gamer);

    gamer = new Gamer("Markiplier");
    cmd.spawn(gamer);
}

static std::size_t counter = 1;

void game(Commands cmd, Query<With<Gamer>> query) {
    for (const auto& entity : query) {
        const auto& name = entity->expect<Gamer>()->name;
        std::cout << "GAMING!!! from " << name;

        if (counter == 5 && name == "Markiplier") {
            std::cout << " - Aaaand he died";
            cmd.del(entity);
        }

        if (counter == 7 && name == "PewDiePie") {
            std::cout << " - Aaaand he's stripped of his gamer title";
            cmd.remove<Gamer>(entity);
        }

        std::cout << std::endl;
    }

    counter++;
}

int main(int, char**) {
    SECS::add(testy).on(Gaming).on(SECS::Enter).on(SECS::Update);
    SECS::add(startup).on(Gaming).on(SECS::Enter);
    SECS::add(game).on(Gaming).on(SECS::Update);

    for (std::size_t i = 0; i < 12; i++) {
        SECS::tick(Gaming);
    }

    return 0;
}
