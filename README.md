# SECS

A **s**equential **E**ntity-**C**omponent-**S**ystem implementation in C++. Intended to be used with raylib, but can be easily used without. Though, not very usable in its current state regardless of your choice of libraries.

See the [usage example](src/SECS_test.cpp) for a quick-start.

If you decide to use this in your project, make sure to clone the `barely-usable` branch. Don't ask why I called it that. (Though, it should be pretty obvious.)

**NOTE**: The docs below may get outdated really quickly since this is a rapidly evolving project.

## The Basics

This library was heavily inspired by the [Bevy game engine](https://github.com/bevyengine/bevy). Therefore, it shares a lot of important concepts:

- Systems are plain functions, optionally taking a bunch of special parameters. Currently implemented are `Query` and `Commands` (with their own, non-Bevy APIs).
- Entities are storage cells for components. C++ smart pointers are used to handle reference-counting, but they could be wrapped in an easier-to-use custom type later.
- Since there are no derive-macros in C++, you have to extend the `Component` struct in order to create a component type. There is no way to avoid severely blanketed compiler diagnostics if you do something wrong.

## API

For each new SECS project:

- Include the `SECS.hpp` header wherever required.
- Define `const Systems SECS::systems` once, e.g. in your `main.cpp` file.
- Populate it with systems using an initializer list.
- Call `SECS::tick(...)` every in-game tick.

`SECS::tick` and `SECS::systems` use numbered game states to define entry/update rules. So make sure to pass the current game state to `SECS::tick` (e.g., `0`).

## Tips and tricks

- As the name suggests, this is a _sequential_ ECS implementation. As such, all the systems you define in `SECS::systems` are checked and evaluated _in the order they are defined_. Make sure to put rendering systems below (after) state-mutating systems, "startup" systems above "update" ones, etc.
- Most of the implemented commands have a shortcut method inside `Commands`. E.g., instead of `cmd.push(new Spawn(new Gamer))`, write `cmd.spawn(new Gamer)`.
- Since there is no multithreading involved, `static` variables and globals are OK to use.
