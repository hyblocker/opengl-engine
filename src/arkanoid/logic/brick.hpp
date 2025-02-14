#pragma once

#include "engine/renderer/scene_graph.hpp"
#include "engine/renderer/mesh.hpp"
#include <box2d/box2d.h>

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

    // returns what brick type this became
    BrickType randomlySelectBrickType();
    // turns this brick into the given brick type. use on level init only!
    void updateBrick(BrickType type);

    // order of execution on physics collision event:
    //   onHit  -  processes hit logic on brick
    //   getPoints  -  give the player points
    //   shouldSpawnPowerup  -  based on result spawn a powerup
    //   isDestroyed  -  decrease the number of bricks the player has to break before advancing to the next level
    //   shouldExistInScene  -  hides the brick from the renderer and physics system for a new level
    void onHit();

    uint32_t getPoints();
    bool shouldSpawnPowerup();
    // if the brick can be considered as destroyed logically. indestructable ones return true always
    bool isDestroyed();
    // if the level handler should "destroy" the brick in the scene graph
    bool shouldExistInScene();

    inline b2BodyId getBrickId() { return m_physicsId; }
    inline void setBrickId(b2BodyId newId) { m_physicsId = newId; }

    inline int getPosX() { return m_posX; };
    inline int getPosY() { return m_posY; };
private:
    render::MeshRenderer* m_renderer = nullptr;
    int m_posX = 0;
    int m_posY = 0;

    // Caches for the materials so that they're recycled
    render::Material m_regularBrickMaterial;
    render::Material m_strongBrickMaterial;
    render::Material m_indestructableBrickMaterial;

    uint32_t m_health = 1;
    uint32_t m_totalHealth = 1;
    BrickType m_type = BrickType::Regular;

    bool m_hasPowerUp = false;

    b2BodyId m_physicsId = b2_nullBodyId;
};