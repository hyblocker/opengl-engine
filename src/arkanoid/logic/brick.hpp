#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/mesh.hpp"

enum class BrickType {
    Regular,
    Strong,
    Indestructable,
    Count,
};

class Brick : public render::IBehaviour {
public:
    Brick(render::Entity* parent, int x, int y)
        : IBehaviour(parent), m_posX(x), m_posY(y) {}
    ~Brick() = default;

    void start() override;
    void sleep() override;
    void update(float deltaTime) override;

    void randomlySelectBrickType();
    void updateBrick(BrickType type);

private:
    render::MeshRenderer* m_renderer = nullptr;
    int m_posX = 0;
    int m_posY = 0;

    // Caches for the materials so that they're recycled
    render::Material m_regularBrickMaterial;
    render::Material m_strongBrickMaterial;
    render::Material m_indestructableBrickMaterial;
};