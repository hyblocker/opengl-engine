#include "brick.hpp"
#include "engine/app.hpp"

// only one for program lifetime, this ensures we don't have collisions
constexpr uint32_t k_HEALTH_INDESTRUCTABLE = 0xFFFFFFFF;

void Brick::start() {
    m_renderer = (render::MeshRenderer*) getEntity()->findComponent(render::ComponentType::MeshRenderer);

    managers::AssetManager* pAssetManager = engine::App::getInstance()->getAssetManager();

    m_regularBrickMaterial.shader = m_renderer->material.shader;
    m_regularBrickMaterial.name = "RegularBrick";
    m_regularBrickMaterial.ambient = { 0.5,0.5,0.5 };
    m_regularBrickMaterial.diffuse = { 1,1,1 };
    m_regularBrickMaterial.metallic = 1;
    m_regularBrickMaterial.roughness = 1;
    m_regularBrickMaterial.diffuseTex = pAssetManager->fetchTexture("brick_regular_albedo.png");
    m_regularBrickMaterial.metaTex = pAssetManager->fetchTexture("brick_regular_meta.png");
    m_regularBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_regularBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("dfg.hdr");

    m_strongBrickMaterial.shader = m_renderer->material.shader;
    m_strongBrickMaterial.name = "StrongBrick";
    m_strongBrickMaterial.ambient = { 0.5,0.5,0.5 };
    m_strongBrickMaterial.diffuse = { 1,1,1 };
    m_strongBrickMaterial.metallic = 1;
    m_strongBrickMaterial.roughness = 1;
    m_strongBrickMaterial.diffuseTex = pAssetManager->fetchTexture("brick_strong_albedo.png");
    m_strongBrickMaterial.metaTex = pAssetManager->fetchTexture("brick_strong_meta.png");
    m_strongBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_strongBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("dfg.hdr");

    m_indestructableBrickMaterial.shader = m_renderer->material.shader;
    m_indestructableBrickMaterial.name = "IndestructableBrick";
    m_indestructableBrickMaterial.ambient = { 0.5,0.5,0.5 };
    m_indestructableBrickMaterial.diffuse = { 1,1,1 };
    m_indestructableBrickMaterial.metallic = 1;
    m_indestructableBrickMaterial.roughness = 1;
    m_indestructableBrickMaterial.diffuseTex = pAssetManager->fetchTexture("brick_indestructable_albedo.png");
    m_indestructableBrickMaterial.metaTex = pAssetManager->fetchTexture("brick_indestructable_meta.png");
    m_indestructableBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_indestructableBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("dfg.hdr");

    updateBrick(BrickType::Regular);
}

void Brick::sleep() {

}

void Brick::update(float deltaTime) {

}

BrickType Brick::randomlySelectBrickType() {
    return BrickType::Regular;
}

void Brick::updateBrick(BrickType type) {
    switch (type) {
    case BrickType::Regular:
    {
        m_renderer->material = m_regularBrickMaterial;
        m_totalHealth = 1;
        break;
    }
    case BrickType::Strong:
    {
        // strong bricks may require anywhere from 3 to 6 hits to break
        m_renderer->material = m_strongBrickMaterial;
        m_totalHealth = engine::RandomNumberGenerator::getRangedInt(3, 6);

        break;
    }
    case BrickType::Indestructable:
    {
        m_renderer->material = m_indestructableBrickMaterial;
        m_totalHealth = k_HEALTH_INDESTRUCTABLE;
        break;
    }
    default:
    {
        LOG_WARN("Unknown brick type {}! Ignoring...", (int) type);
        break;
    }
    }
    m_health = m_totalHealth;
    m_type = type;
}

void Brick::onHit() {
    if (m_health > 0 && m_type != BrickType::Indestructable) {
        m_health--;
    }
}

uint32_t Brick::getPoints() {
    switch (m_type) {
    case BrickType::Regular:
    {
        return 50;
    }
    case BrickType::Strong:
    {
        // only reward 200 points on last hit, otherwise reward 100 points
        if (m_health == 0) {
            return 200;
        } else {
            return 100;
        }
    }
    }
    return 0;
}

bool Brick::shouldSpawnPowerup() {
    switch (m_type) {
    case BrickType::Strong:
        // @TODO: Random chance brick has powerup?
        return false;
    }
    return false;
}

bool Brick::isDestroyed() {
    switch (m_type) {
    case BrickType::Indestructable:
        return false;
    }
    return m_health <= 0;
}

bool Brick::shouldExistInScene() {
    switch (m_type) {
    case BrickType::Indestructable:
        return true;
    }

    return m_health > 0;
}