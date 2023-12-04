#pragma once

#include <concepts>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

// Forward declarations.

template <typename T>
concept SystemParam = requires() { T(); };

struct State;
struct Stage;
struct SystemBase;
struct Command;
struct Entity;
struct Component;
struct ID;
struct System;
struct ExecutionContext;
struct SystemBuilder;

template <SystemParam... Params>
struct SystemDef;

using Systems = std::vector<std::unique_ptr<SystemBase>>;

using Entities = std::vector<ID>;
using ComponentSet = std::set<std::shared_ptr<Component>>;
using CommandQueue = std::vector<std::shared_ptr<Command>>;

using Filter = Entities (*const)(const Entities&);

// The main namespace.

namespace SECS {
    extern Systems systems;
    extern std::vector<std::shared_ptr<Entity>> entities;
    extern CommandQueue cmd_queue;
    extern ExecutionContext exec_context;
    extern const Stage Enter, Update;

    void tick(State);
    SystemBuilder add(System);
}; // namespace SECS

// Definitions.

struct Stage {
  private:
    std::size_t id;

  public:
    Stage(std::size_t id) : id(id) {}

    bool operator==(const Stage& another) const {
        return id == another.id;
    }

    // Needed for `std::set` operations to work.
    bool operator<(const Stage& another) const {
        return id < another.id;
    }
};

struct State {
  private:
    std::size_t id;

  public:
    State() {
        static std::size_t last_id = 0;
        id = last_id++;
    }

    bool operator==(const State& another) const {
        return id == another.id;
    }

    // Needed for `std::set` operations to work.
    bool operator<(const State& another) const {
        return id < another.id;
    }
};

struct ExecutionContext {
    Stage stage;
    State state;
};

template <typename T>
struct Param {
    Param() = delete;
};

template <>
struct Param<Stage> {
    Stage value;

    Param() : value(SECS::exec_context.stage) {}
};

template <>
struct Param<State> {
    State value;

    Param() : value(SECS::exec_context.state) {}
};

struct Component {
    Component() = default;
    virtual ~Component() = default;
};

struct Entity {
    ComponentSet components;

    Entity() = default;
    ~Entity() = default;

    template <typename T>
    const std::weak_ptr<T> get() const {
        for (const auto& ptr : components) {
            if (dynamic_cast<T*>(ptr.get()) != nullptr) {
                return std::static_pointer_cast<T>(ptr);
            }
        }

        return std::weak_ptr<T>();
    }

    template <typename T>
    const std::shared_ptr<T> expect() const {
        return get<T>().lock();
    }

    template <typename... Components>
    const std::tuple<const std::shared_ptr<Components>...> tup() const {
        return {expect<Components>()...};
    }
};

struct ID {
  private:
    std::weak_ptr<Entity> ptr;

  public:
    ID(const ID& id) : ptr(id.ptr) {}
    ID(const std::shared_ptr<Entity>& ptr) : ptr(ptr) {}
    ~ID() = default;

    bool valid() const {
        return !ptr.expired();
    }

    Entity& operator*() {
        return *ptr.lock();
    }

    const Entity& operator*() const {
        return *ptr.lock();
    }

    const std::shared_ptr<Entity> operator->() const {
        return ptr.lock();
    }

    bool operator==(const ID& other) const {
        if (valid() && other.valid()) {
            return ptr.lock() == other.ptr.lock();
        } else {
            return valid() == other.valid();
        }
    }

    bool operator==(const std::shared_ptr<Entity>& other) const {
        return *this == const_cast<const ID&&>(ID(other));
    }
};

struct SystemBase {
    SystemBase() = default;
    virtual ~SystemBase() = default;

    virtual void tick() = 0;

    virtual SystemBase& on(const Stage&) = 0;
    virtual SystemBase& on(const State&) = 0;

  protected:
    virtual std::unique_ptr<SystemBase> build() = 0;
    friend SystemBuilder;
};

struct System : SystemBase {
    const std::shared_ptr<SystemBase> inner;

    template <typename... Params>
    System(SystemDef<Params...> def) : System(def.functor) {}

    template <typename... Params>
    System(void (*functor)(Params...))
        : inner(std::make_shared<SystemDef<Params...>>(functor)) {}

    template <typename... Params>
    System(std::function<void(Params...)> functor)
        : inner(std::make_shared<SystemDef<Params...>>(functor)) {}

    std::unique_ptr<SystemBase> build() override {
        return std::unique_ptr<SystemBase>(new System(*this));
    }

    void tick() override {
        inner->tick();
    }

    SystemBase& on(const Stage& stage) override {
        return inner->on(stage);
    }

    SystemBase& on(const State& state) override {
        return inner->on(state);
    }

    const std::shared_ptr<SystemBase> operator->() const {
        return inner;
    }
};

template <SystemParam... Params>
struct SystemDef : public SystemBase {
    using Fn = std::function<void(Params...)>;

    const Fn functor;

    std::set<Stage> allowed_stages;
    std::set<State> allowed_states;

    SystemDef(Fn functor) : SystemBase(), functor(functor) {}
    SystemDef(void (*functor)(Params...)) : SystemBase(), functor(functor) {}

    std::unique_ptr<SystemBase> build() override {
        return std::unique_ptr<SystemBase>(new SystemDef<Params...>(*this));
    }

    void tick() override {
        const auto allowed_stage =
            allowed_stages.contains(SECS::exec_context.stage);

        const auto allowed_state =
            allowed_states.contains(SECS::exec_context.state);

        if (allowed_stage && allowed_state) {
            functor(Params()...);
        }
    }

    SystemBase& on(const Stage& stage) override {
        allowed_stages.insert(stage);
        return *this;
    }

    SystemBase& on(const State& state) override {
        allowed_states.insert(state);
        return *this;
    }
};

struct Command {
    Command() = default;
    virtual ~Command() = default;

    virtual void perform() const = 0;
};

template <typename... Components>
struct Spawn : public Command {
    std::shared_ptr<Entity> entity;

    Spawn(Components*... args) : Command(), entity(std::make_shared<Entity>()) {
        entity->components = {std::shared_ptr<Components>(args)...};
    }

    void perform() const override {
        SECS::entities.push_back(entity);
    }
};

struct Delete : public Command {
    ID entity;

    Delete(ID entity) : Command(), entity(entity) {}

    void perform() const override {
        if (!entity.valid()) {
            return;
        }

        std::erase_if(SECS::entities, [this](const auto& other) {
            return entity == other;
        });
    }
};

template <typename Comp>
struct Insert : public Command {
    ID entity;
    Comp* component;

    Insert(ID entity, Comp* component)
        : Command(), entity(entity), component(component) {}

    void perform() const override {
        if (entity.valid()) {
            entity->components.insert(std::unique_ptr<Component>(component));
        }
    }
};

template <typename Comp>
struct Remove : public Command {
    ID entity;

    Remove(ID entity) : Command(), entity(entity) {}

    void perform() const override {
        if (!entity.valid()) {
            return;
        }

        std::erase_if(entity->components, [this](const auto& other) {
            return dynamic_cast<Comp*>(other.get()) != nullptr;
        });
    }
};

struct Commands {
    CommandQueue queue;

    Commands() {}

    void push(Command* cmd) {
        queue.push_back(std::shared_ptr<Command>(cmd));
    }

    template <typename Component>
    void insert(ID target, Component* component) {
        push(new Insert(target, component));
    }

    template <typename Component>
    void remove(ID target) {
        push(new Remove<Component>(target));
    }

    template <typename... Components>
    void spawn(Components*... components) {
        push(new Spawn<Components...>(components...));
    }

    void del(ID entity) {
        push(new Delete(entity));
    }

    ~Commands() {
        for (auto& command : queue) {
            SECS::cmd_queue.push_back(std::move(command));
        }

        queue.clear();
    }
};

inline Entities All(const Entities& input) {
    return input;
}

template <Filter Has = All>
struct Query {
    Entities results;

    Query() {
        Entities ids;

        for (const auto& entity : SECS::entities) {
            ids.push_back(entity);
        }

        results = Has(ids);
    }

    ~Query() {}

    Entities::iterator begin() {
        return results.begin();
    }

    Entities::iterator end() {
        return results.end();
    }
};

template <typename T>
bool contains(const ID& ent) {
    if (!ent.valid()) {
        return false;
    }

    for (const auto& comp : ent->components) {
        if (dynamic_cast<T*>(comp.get()) != nullptr) {
            return true;
        }
    }

    return false;
}

template <typename... Components>
Entities With(const Entities& input) {
    Entities output;

    for (const auto& it : input) {
        const auto contains_all = (contains<Components>(it) && ...);

        if (contains_all) {
            output.push_back(it);
        }
    }

    return output;
}

struct SystemBuilder {
    System sys;

  private:
    SystemBuilder(System sys) : sys(sys) {}
    friend SystemBuilder SECS::add(System sys);

  public:
    SystemBuilder& on(const Stage& stage) {
        sys.on(stage);
        return *this;
    }

    SystemBuilder& on(const State& state) {
        sys.on(state);
        return *this;
    }

    ~SystemBuilder() {
        SECS::systems.push_back(sys.build());
    }
};
