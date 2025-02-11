#include "brick.hpp"
#include "engine/app.hpp"

void Brick::start() {
    m_renderer = (render::MeshRenderer*) getEntity()->findComponent(render::ComponentType::MeshRenderer);

    managers::AssetManager* pAssetManager = engine::App::getInstance()->getAssetManager();

    m_regularBrickMaterial.shader = m_renderer->material.shader;
    m_regularBrickMaterial.name = "RegularBrick";
    m_regularBrickMaterial.ambient = { 1,1,1 };
    m_regularBrickMaterial.diffuse = { 1,1,0 };
    m_regularBrickMaterial.roughness = 1;
    m_regularBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_regularBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("brdf_lut.png");

    m_strongBrickMaterial.shader = m_renderer->material.shader;
    m_strongBrickMaterial.name = "StrongBrick";
    m_strongBrickMaterial.ambient = { 1,1,1 };
    m_strongBrickMaterial.diffuse = { 1,0,1 };
    m_strongBrickMaterial.roughness = 1;
    m_strongBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_strongBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("brdf_lut.png");

    m_indestructableBrickMaterial.shader = m_renderer->material.shader;
    m_indestructableBrickMaterial.name = "IndestructableBrick";
    m_indestructableBrickMaterial.ambient = { 1,1,1 };
    m_indestructableBrickMaterial.diffuse = { 0,1,1 };
    m_indestructableBrickMaterial.roughness = 1;
    m_indestructableBrickMaterial.matcapTex = pAssetManager->fetchTexture("hdri_matcap.png");
    m_indestructableBrickMaterial.brdfLutTex = pAssetManager->fetchTexture("brdf_lut.png");

    updateBrick(BrickType::Regular);
}

void Brick::sleep() {

}

void Brick::update(float deltaTime) {

}

void Brick::randomlySelectBrickType() {

}

void Brick::updateBrick(BrickType type) {
    switch (type) {
    case BrickType::Regular:
    {
        m_renderer->material = m_regularBrickMaterial;
        break;
    }
    case BrickType::Strong:
    {

        m_renderer->material = m_strongBrickMaterial;
        break;
    }
    case BrickType::Indestructable:
    {
        m_renderer->material = m_indestructableBrickMaterial;

        break;
    }
    default:
    {
        LOG_WARN("Unknown brick type {}! Ignoring...", (int) type);
        break;
    }
    }
}