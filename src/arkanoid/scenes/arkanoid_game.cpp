#include "arkanoid/arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"

#include <imgui.h>

void ArkanoidLayer::initGameScene() {

    constexpr uint32_t k_BRICKS_COLUMNS = 10;
    constexpr uint32_t k_BRICKS_ROWS = 4;
    constexpr float k_depthOffset = -10;

    using namespace ::render;

    // Metadata
    m_gameScene.sceneName = "Menu";
    m_gameScene.lightingParams.skybox = {
        .type = render::SkyboxType::Procedural,
    };


    // construct scene
    m_gameScene.push_back(
        EntityBuilder().withName("Camera")
        .withPosition({ 0,0,30 })
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
            .withPosition({ 0, 1.020f, k_depthOffset })
            .withScale({ 1000, 50, 2 })
            .withPhysics({
                .bodyType = physics::PhysicsBodyType::Static,
                .shape = {
                    .shape = physics::PhysicsShape::Box,
                    .box = {
                        .size = { 1000, 50 }
                    }
                }
                })
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
            .withPosition({ -1.450f, 0, k_depthOffset })
            .withScale({ 50, 1000, 2 })
            .withPhysics({
                .bodyType = physics::PhysicsBodyType::Static,
                .shape = {
                    .shape = physics::PhysicsShape::Box,
                    .box = {
                        .size = { 50, 1000 }
                    }
                }
                })
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
            .withPosition({ 1.450f, 0, k_depthOffset })
            .withScale({ 50, 1000, 2 })
            .withPhysics({
                .bodyType = physics::PhysicsBodyType::Static,
                .shape = {
                    .shape = physics::PhysicsShape::Box,
                    .box = {
                        .size = { 50, 1000 }
                    }
                }
                })
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
        .withPosition({ 0, -13.850f, k_depthOffset })
        .withPhysics({
            .bodyType = physics::PhysicsBodyType::Static,
            .shape = {
                .shape = physics::PhysicsShape::Box,
                .box = {
                    .size = { 10, 1.5f }
                }
            }
            })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("box.obj"),
            .material = {
                .shader = m_shader,
                .name = "ColliderViz",
            }
            })
    );

    m_gameScene.push_back(
        EntityBuilder().withName("Ball")
        .withPosition({ 0, 0, k_depthOffset })
        .withPhysics({
            .density = 1.0f,
            .friction = 0.1f,
            .bodyType = physics::PhysicsBodyType::Rigidbody,
            .shape = {
                .shape = physics::PhysicsShape::Circle,
                .circle = {
                    .centre = { 0, 0 },
                    .radius = 1.0f,
                }
            }
            })
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
        .withPosition({-17.5f, 8.5f, 0});
    for (int x = 0; x < k_BRICKS_COLUMNS; x++) {
        for (int y = 0; y < k_BRICKS_ROWS; y++) {
            brickContainer.withChild(
                EntityBuilder().withName(fmt::format("Brick_{}_{}", x, y))
                .withPosition({ x * 3.5f, y * 2.0f, k_depthOffset })
                .withMeshRenderer({
                    .mesh = getAssetManager()->fetchMesh("brick.obj"),
                    .material = {
                        .shader = m_shader,
                        .name = "Brick",
                        .ambient = {0.1f, 0.1f, 0.1f},
                        .diffuse = {1,1,1},
                    } })
                .withPhysics({
                    .bodyType = physics::PhysicsBodyType::Kinematic,
                    .shape = {
                        .shape = physics::PhysicsShape::Box,
                        .box = {
                            .size = { 3.0f, 1.5f }
                        }
                    }
                })
            );
        }
    }
    
    m_gameScene.push_back(brickContainer);
}