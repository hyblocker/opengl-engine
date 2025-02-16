#include "arkanoid/arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"

#include <imgui.h>

void ArkanoidLayer::initMenuScene(render::Scene& outScene) {

    constexpr uint32_t k_BRICKS_COLUMNS = 10;
    constexpr uint32_t k_BRICKS_ROWS = 4;
    constexpr float k_depthOffset = -10;

    using namespace ::render;

    // Metadata
    outScene.sceneName = "Menu";
    outScene.lightingParams.skybox = {
        .type = render::SkyboxType::Procedural,
    };


    // construct scene
    outScene.push_back(
        EntityBuilder().withName("Camera")
        .withPosition({0,0,30})
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

    outScene.push_back(
        EntityBuilder().withName("Suzanne")
        .withPosition({0, 1.900, -5})
        .withEnabled(false)
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("test.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "Suzanne",
                .ambient = hlslpp::float3(0.0352941176f, 0.0745098039f, 0.1215686275f),
                .diffuseTex = getAssetManager()->fetchTexture("brick_wall.png")
            }
            })
    );

    outScene.push_back(
        EntityBuilder().withName("UICanvas")
        .withUiCanvas()
        .withChild(
            EntityBuilder().withName("Logo")
            .withUiText({
                .posX = -2.970f,
                .posY = -0.960f,
                .text = "BREAKANOID",
                .textScale = 5,
                })
                )
        .withChild(
            EntityBuilder().withName("PlayButton")
            .withUiSprite({
                .posX = 52.3f,
                .posY = -86.89f,
                .sizeX = 268,
                .sizeY = 145,
                .texture = getAssetManager()->fetchTexture("brick_wall.png"),
                }).withChild(
                EntityBuilder().withName("PlayButton_Text")
                .withUiText({
                    .posX = -0.570f,
                    .posY = 1.160f,
                    .text = "Play",
                    .textScale = 2.550f,
                    }))
            )
    );

}