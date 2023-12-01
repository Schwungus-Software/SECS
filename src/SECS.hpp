#pragma once

#include <concepts>
#include <functional>
#include <initializer_list>
#include <memory>
#include <set>

enum class Stage {
    STARTUP,
    UPDATE,
};

struct Entity;
struct Command;

struct Component {
    Component() = default;
    virtual ~Component() = default;
};

using Entities = std::vector<std::shared_ptr<Entity>>;
using ComponentSet = std::set<std::shared_ptr<Component>>;
using CommandQueue = std::vector<std::unique_ptr<Command>>;

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

struct SystemBase {
    const std::size_t state_idx;
    const Stage stage;

    SystemBase(const std::size_t state_idx, const Stage stage)
        : state_idx(state_idx), stage(stage) {}

    virtual ~SystemBase() = default;

    virtual void tick() = 0;
};

template <typename... Params>
struct SystemDef : public SystemBase {
    using Fn = void (*const)(Params...);

    const Fn functor;

    SystemDef(const std::size_t state_idx, const Stage stage, Fn functor)
        : SystemBase(state_idx, stage), functor(functor) {}

    void tick() override {
        functor(Params()...);
    }
};

struct AnySystem {
    const std::shared_ptr<SystemBase> inner;

    template <typename... Params>
    AnySystem(
        const std::size_t state_idx, const Stage stage,
        void (*const functor)(Params...)
    )
        : inner(new SystemDef<Params...>(state_idx, stage, functor)) {}

    template <typename... Params>
    AnySystem(const SystemDef<Params...>& def)
        : AnySystem(def.state_idx, def.stage, def.functor) {}

    void tick() const {
        inner->tick();
    }

    const std::shared_ptr<SystemBase> operator->() const {
        return inner;
    }
};

using Systems = std::vector<AnySystem>;

struct Command {
    Command() = default;
    virtual ~Command() = default;

    virtual void perform() const = 0;
};

namespace SECS {
    extern const Systems systems;
    extern Entities entities;
    extern CommandQueue cmd_queue;

    void tick(std::size_t);
}; // namespace SECS

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
    std::shared_ptr<Entity> entity;

    Delete(std::shared_ptr<Entity> entity) : Command(), entity(entity) {}

    void perform() const override {
        std::erase(SECS::entities, entity);
    }
};

template <typename Comp>
struct Insert : public Command {
    std::shared_ptr<Entity> entity;
    Comp* component;

    Insert(std::shared_ptr<Entity> entity, Comp* component)
        : Command(), entity(entity), component(component) {}

    void perform() const override {
        entity->components.insert(std::unique_ptr<Component>(component));
    }
};

template <typename Comp>
struct Remove : public Command {
    std::shared_ptr<Entity> entity;

    Remove(std::shared_ptr<Entity> entity) : Command(), entity(entity) {}

    void perform() const override {
        std::erase_if(entity->components, [this](const auto& other) {
            return dynamic_cast<Comp*>(other.get()) != nullptr;
        });
    }
};

struct Commands {
    std::vector<std::unique_ptr<Command>> queue;

    Commands() {}

    void push(Command* cmd) {
        queue.push_back(std::unique_ptr<Command>(cmd));
    }

    template <typename Component>
    void insert(std::shared_ptr<Entity> target, Component* component) {
        push(new Insert(target, component));
    }

    template <typename Component>
    void remove(std::shared_ptr<Entity> target) {
        push(new Remove<Component>(target));
    }

    template <typename... Components>
    void spawn(Components*... components) {
        push(new Spawn<Components...>(components...));
    }

    void del(std::shared_ptr<Entity> entity) {
        push(new Delete(entity));
    }

    ~Commands() {
        for (auto& command : queue) {
            SECS::cmd_queue.push_back(std::move(command));
        }

        queue.clear();
    }
};

using Filter = Entities (*const)(const Entities&);

inline Entities All(const Entities& input) {
    return input;
}

template <Filter Has = All>
struct Query {
    Entities results;

    Query() {
        results = Has(SECS::entities);
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
bool contains(const std::shared_ptr<Entity>& ent) {
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
