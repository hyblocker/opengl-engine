#include "arkanoid/arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"

#include "arkanoid/logic/paddle.hpp"
#include "arkanoid/logic/brick.hpp"
#include "arkanoid/logic/level_handler.hpp"

#include <imgui.h>

void ArkanoidLayer::initGameScene() {

    constexpr uint32_t k_BRICKS_COLUMNS = 10;
    constexpr uint32_t k_BRICKS_ROWS = 4;

    using namespace ::render;

    // Metadata
    m_gameScene.sceneName = "Game";
    m_gameScene.lightingParams.skybox = {
        .type = render::SkyboxType::Procedural,
    };
    m_gameScene.physicsParams.gravity = { 0, 0 };


    // construct scene
    m_gameScene.push_back(
        EntityBuilder().withName("Camera")
        .withPosition({ 0,0,30 })
        .withCamera({
            .infiniteFar = true
            })
    );

    Entity* sunEntity = m_gameScene.push_back(
        EntityBuilder().withName("Sun")
        .withLight({
            .colour = {1,1,1}
            })
    );

    // Assign light to the scene
    m_gameScene.lightingParams.sunLight = (Light*)sunEntity->findComponent(ComponentType::Light);

    m_gameScene.push_back(
        EntityBuilder().withName("Suzanne")
        .withPosition({ 0, 1.900, -5 })
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

    // Add colliders around the edges of the screen
    m_gameScene.push_back(
        EntityBuilder().withName("ScreenBoundary")
        .withChild(
            EntityBuilder().withName("Top")
            .withPosition({ 0, 17.420f, 0 })
            .withScale({ 100, 2, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shader,
                    .name = "ColliderViz",
                }
                })
        )
        .withChild(
            EntityBuilder().withName("Left")
            .withPosition({ -30.120f, 0, 0 })
            .withScale({ 2, 100, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shader,
                    .name = "ColliderViz",
                }
                })
        )
        .withChild(
            EntityBuilder().withName("Right")
            .withPosition({ 30.120f, 0, 0 })
            .withScale({ 2, 100, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shader,
                    .name = "ColliderViz",
                }
                })
        )
        .withChild(
            EntityBuilder().withName("Bottom")
            .withPosition({ 0, -19.00f, 0 })
            .withScale({ 100, 2, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shader,
                    .name = "ColliderViz",
                }
                })
        )
    );

    m_gameScene.push_back(
        EntityBuilder().withName("Paddle")
        .withPosition({ 0, -13.850f, 0 })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("paddle.obj"),
            .material = {
                .shader = m_shader,
                .name = "Paddle",
            }
            })
    );

    m_gameScene.push_back(
        EntityBuilder().withName("Ball")
        .withPosition({ 0, -14.6f, 0 })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("ball.obj"),
            .material = {
                .shader = m_shader,
                .name = "PaddleBall",
                .ambient = {0.1f, 0.1f, 0.1f},
                .diffuse = {1,1,1},
            }
            })
    );

    EntityBuilder brickContainer;
    brickContainer.withName("BrickContainer")
        .withPosition({ -15.75f, 5.980f, 0 });
    for (int x = 0; x < k_BRICKS_COLUMNS; x++) {
        for (int y = 0; y < k_BRICKS_ROWS; y++) {
            brickContainer.withChild(
                EntityBuilder().withName(fmt::format("Brick_{}_{}", x, y))
                .withPosition({ x * 3.5f, y * 2.0f, 0 })
                .withMeshRenderer({
                    .mesh = getAssetManager()->fetchMesh("brick.obj"),
                    .material = {
                        .shader = m_shader,
                        .name = "Brick",
                        .ambient = {0.1f, 0.1f, 0.1f},
                        .diffuse = {1,1,1},
                } })
                );
        }
    }

    m_gameScene.push_back(brickContainer);

    m_gameScene.push_back(
        EntityBuilder().withName("GameManager")
        .withBehaviour<LevelHandler>()
    );
}