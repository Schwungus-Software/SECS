# SECS

A **s**equential **E**ntity-**C**omponent-**S**ystem implementation in C++. Intended to be used with raylib, but can be easily used without. Though, not very usable in its current state regardless of your choice of libraries.

See the [usage example](src/SECS_test.cpp) for a quick-start.

**NOTE**: The docs below may get outdated really quickly since this is a rapidly evolving project.

## The Basics

This library was heavily inspired by the [Bevy game engine](https://github.com/bevyengine/bevy). Therefore, it shares a lot of important concepts:

- Systems are plain functions, optionally taking a bunch of special parameters. Currently implemented are `Query` and `Commands` (with their own, non-Bevy APIs).
- Entities are storage cells for components. C++ smart pointers are used to handle reference-counting, but they could be wrapped in an easier-to-use custom type later.
- Since there are no derive-macros in C++, you have to extend the `Component` struct in order to create component types.

As a final note, there is no way to avoid severely blanketed compiler diagnostics if you do something wrong. That's what template magic does to a person.

## API

For each new SECS project:

- Include the `SECS.hpp` header wherever required.
- Define your game states like `const State Menu, Play`, etc.
- Populate the list of systems with `SECS::add`. It returns a system builder finalized upon destruction; there is no need to call `.build()` or such.
- Call `SECS::tick(...)` every in-game tick. In the example above, you can pass it either `Menu` or `Play`.

## Tips and tricks

- As the name suggests, this is a _sequential_ ECS implementation. As such, any systems you define are checked and evaluated _in the order they are added_. Make sure to put rendering systems below (after) state-mutating systems, "startup" systems above "update" ones, etc.
- Most of the implemented commands have a shortcut method inside `Commands`. E.g., instead of `cmd.push(new Spawn(new Gamer))`, write `cmd.spawn(new Gamer)`.
- Since there is no multithreading involved, `static` variables and globals are OK to use.
