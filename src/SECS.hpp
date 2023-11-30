#pragma once

#include <concepts>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <set>
#include <tuple>
#include <variant>

enum class Stage {
    ENTER,
    UPDATE,
};

struct ID {
    // Placeholder for entity IDs.
};

template <typename GS>
struct SECS;

template <typename GS, typename T>
concept SystemParam = requires(SECS<GS>& ctx) { T(ctx); };

template <typename GS>
struct SystemBase {
    const GS state;
    const Stage stage;

    SystemBase(const GS state, const Stage stage)
        : state(state), stage(stage) {}

    virtual void tick(SECS<GS>&) = 0;
    virtual ~SystemBase() = default;
};

template <typename GS, SystemParam<GS>... Params>
struct SystemDef : public SystemBase<GS> {
    using Fn = void (*)(Params...);

    const Fn functor;

    SystemDef(const GS state, const Stage stage, const Fn functor)
        : SystemBase<GS>(state, stage), functor(functor) {}

    void tick(SECS<GS>& ctx) override {
        functor(Params(ctx)...);
    }
};

template <typename GS>
struct AnySystem {
    const std::shared_ptr<SystemBase<GS>> inner;

    template <SystemParam<GS>... Params>
    AnySystem(
        const GS state, const Stage stage, void (*const functor)(Params...)
    )
        : inner(new SystemDef<GS, Params...>(state, stage, functor)) {}

    void tick(SECS<GS>& ctx) const {
        inner->tick(ctx);
    }
};

template <typename GS>
struct SECS {
    const std::vector<AnySystem<GS>> systems;

    GS prev_state;
    bool just_started;

    SECS(GS init_state, const std::initializer_list<AnySystem<GS>>&& systems)
        : systems(systems), prev_state(init_state), just_started(true) {}

    void tick(GS state) {
        for (const auto& any_sys : systems) {
            const auto& sys = any_sys.inner;

            const auto can_enter = just_started || state != prev_state;
            const auto can_tick = state == prev_state;

            const auto trigger_enter = sys->stage == Stage::ENTER && can_enter;
            const auto trigger_tick = sys->stage == Stage::UPDATE && can_tick;

            if (trigger_enter || trigger_tick) {
                sys->tick(*this);
            }
        }

        prev_state = state;
        just_started = false;
    }
};
