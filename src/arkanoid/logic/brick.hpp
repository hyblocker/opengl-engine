#pragma once

#include "engine/renderer/scene_graph.hpp"

class Brick : public render::IBehaviour {
public:
    Brick(render::Entity* parent) : IBehaviour(parent) {}
    ~Brick() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;
};