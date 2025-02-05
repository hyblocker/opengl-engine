#include "arkanoid/arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"

#include <imgui.h>

void ArkanoidLayer::initMenuScene() {

    using namespace ::render;

    // Metadata
    m_menuScene.sceneName = "Menu";
    m_menuScene.lightingParams.skybox = {
        .type = render::SkyboxType::Procedural,
    };


    // construct scene
    m_menuScene.push_back(
        EntityBuilder().withName("Camera")
        .withCamera({
            .infiniteFar = true
            })
        .withChild(
            EntityBuilder().withName("Light")
            .withLight({
                .colour = {1,1,1}
                })
        )
        );

    m_menuScene.push_back(
        EntityBuilder().withName("Suzanne")
        .withPosition({0,1.900,-5})
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("test.obj"),
            .material = {
                .shader = m_shader,
                .name = "Suzanne",
                .ambient = hlslpp::float3(0.0352941176f, 0.0745098039f, 0.1215686275f),
                .diffuseTex = getAssetManager()->fetchTexture("brick_wall.png")
            }
            })
    );
}