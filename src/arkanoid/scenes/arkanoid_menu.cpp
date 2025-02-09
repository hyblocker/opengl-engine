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

    m_menuScene.push_back(
        EntityBuilder().withName("Suzanne")
        .withPosition({0, 1.900, -5})
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
    m_menuScene.push_back(
        EntityBuilder().withName("ScreenBoundary")
        .withChild(
            EntityBuilder().withName("Top")
            .withPosition({ 0, 1.020f, -10 })
            .withScale({1000, 50, 2})
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
            .withPosition({ -1.450f, 0, -10 })
            .withScale({50, 1000, 2})
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
            .withPosition({ 1.450f, 0, -10 })
            .withScale({50, 1000, 2})
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

    m_menuScene.push_back(
        EntityBuilder().withName("Paddle")
        .withPosition({ 0, -13.850f, -10 })
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

    m_menuScene.push_back(
        EntityBuilder().withName("Ball")
        .withPosition({ 0, 0, -10 })
        .withPhysics({
            .density = 1.0f,
            .friction = 0.1f,
            .bodyType = physics::PhysicsBodyType::Rigidbody,
            .shape = {
                .shape = physics::PhysicsShape::Circle,
                .circle = {
                    .radius = 1.0f
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
}