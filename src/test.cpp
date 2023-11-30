#include <iostream>

#include "SECS.hpp"

enum State {
    MAIN,
};

void main_startup() {
    std::cout << "HERE WE GO GAMERS" << std::endl;
}

void main_update() {
    std::cout << "GAMING!!!" << std::endl;
}

int main(int argc, char* argv[]) {
    auto state = State::MAIN;

    SECS<State> ctx{
        state,
        {{State::MAIN, Stage::ENTER, main_startup},
         {State::MAIN, Stage::UPDATE, main_update}}};

    for (std::size_t i = 0; i < 10; i++) {
        ctx.tick(state);
    }
}
