#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/physics/physics_components.hpp"

constexpr float k_PADDLE_VELOCITY = 1.0f;

class Paddle : public render::IBehaviour {
public:
    Paddle(render::Entity* parent) : IBehaviour(parent) {}
    ~Paddle() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;

private:
    physics::PhysicsComponent* m_physicsComponent = nullptr;
};