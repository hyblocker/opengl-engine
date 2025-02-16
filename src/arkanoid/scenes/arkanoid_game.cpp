#include "arkanoid/arkanoid_layer.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/renderer/scene_composer.hpp"

#include "arkanoid/logic/paddle.hpp"
#include "arkanoid/logic/brick.hpp"
#include "arkanoid/logic/level_handler.hpp"
#include "arkanoid/logic/graphics_mode.hpp"

#include "arkanoid/logic/ui_stuff.hpp"

#include <imgui.h>

void ArkanoidLayer::initGameScene(render::Scene& outScene) {

    constexpr uint32_t k_BRICKS_COLUMNS = 10;
    constexpr uint32_t k_BRICKS_ROWS = 4;

    using namespace ::render;

    m_ballParticleBlendState = getDevice()->makeBlendState({
        .blendEnable = true,
        .srcFactor = gpu::BlendFactor::SrcAlpha,
        .dstFactor = gpu::BlendFactor::OneMinusSrcColour,
        .blendOp = gpu::BlendOp::Add,
    });

    // Metadata
    outScene.sceneName = "Game";
    outScene.lightingParams.skybox = {
        .type = render::SkyboxType::Procedural,
    };
    outScene.physicsParams.gravity = { 0, 0 };


    // construct scene
    outScene.push_back(
        EntityBuilder().withName("Camera")
        .withPosition({ 0,0,30 })
        .withCamera({
            .infiniteFar = true
            })
    );

    Entity* sunEntity = outScene.push_back(
        EntityBuilder().withName("Sun")
        // .withRotationEulerAngles({ -62.908f * DEG2RAD, 11.1206f * DEG2RAD, 0.731718f * DEG2RAD })
        .withRotation(hlslpp::quaternion(0.849023f, -0.5183f, 0.08597f, 0.055981f))
        .withLight({
            .colour = {1,1,1}
            })
    );

    // Assign light to the scene
    outScene.lightingParams.sunLight = (Light*)sunEntity->findComponent(ComponentType::Light);

    outScene.push_back(
        EntityBuilder().withName("Suzanne")
        .withEnabled(false)
        .withPosition({ 0, 1.900, -5 })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("test.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "Suzanne",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuseTex = getAssetManager()->fetchTexture("brick_wall.png"),
                .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
            }
            })
        .withBehaviour<GraphicsMode>(true, m_shaderClassic)
    );

    // Add colliders around the edges of the screen
    outScene.push_back(
        EntityBuilder().withName("ScreenBoundary")
        .withChild(
            EntityBuilder().withName("Top")
            .withPosition({ 0, 17.420f, 0 })
            .withScale({ 100, 2, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "ColliderViz",
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
        .withChild(
            EntityBuilder().withName("Left")
            .withPosition({ -28.120f, 0, 0 })
            .withScale({ 2, 100, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "ColliderViz",
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
        .withChild(
            EntityBuilder().withName("Right")
            .withPosition({ 28.120f, 0, 0 })
            .withScale({ 2, 100, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "ColliderViz",
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
        .withChild(
            EntityBuilder().withName("Bottom")
            .withPosition({ 0, -19.00f, 0 })
            .withScale({ 100, 2, 2 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "ColliderViz",
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
    );

    outScene.push_back(
        EntityBuilder().withName("Paddle")
        .withPosition({ 0, -13.850f, 0 })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("paddle.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "Paddle",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuse = {1.0f, 1.0f, 1.0f},
                .metallic = 1,
                .roughness = 1,
                .diffuseTex = getAssetManager()->fetchTexture("paddle_albedo.png"),
                .metaTex = getAssetManager()->fetchTexture("paddle_meta.png"),
                .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
            }
            })
        .withBehaviour<GraphicsMode>(true, m_shaderClassic)
    );

    outScene.push_back(
        EntityBuilder().withName("Ball")
        .withPosition({ 0, -14.6f, 0 })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("ball.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "PaddleBall",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuse = {1.0f, 1.0f, 1.0f},
                .roughness = 0.2f,
                .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
            }
        })
        .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        .withParticleSystem({
            .material = {
                .shader = m_shaderParticle,
                .diffuse = {1,1,1},
                .diffuseTex = getAssetManager()->fetchTexture("particle_texture.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr"),
                .drawOrder = k_drawOrder_Transparent,
            },
            .blendState = m_ballParticleBlendState
        })
    );

    outScene.push_back(
        EntityBuilder().withName("FlipperLeft")
        .withPosition({ -22.760f, -6.130f, 0 })
        .withRotationEulerAngles({ 0.0f * DEG2RAD, 180.0f * DEG2RAD, 0.0f * DEG2RAD })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("flipper.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "Flipper",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuse = {1.0f, 1.0f, 1.0f},
                .metallic = 1,
                .roughness = 1,
                .diffuseTex = getAssetManager()->fetchTexture("flipper_albedo.png"),
                .metaTex = getAssetManager()->fetchTexture("flipper_meta.png"),
                .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
            }
            })
        .withBehaviour<GraphicsMode>(true, m_shaderClassic)
    );

    outScene.push_back(
        EntityBuilder().withName("FlipperRight")
        .withPosition({ 22.760f, -6.130f, 0 })
        .withRotationEulerAngles({ 0.0f * DEG2RAD, 0.0f * DEG2RAD, 0.0f * DEG2RAD })
        .withMeshRenderer({
            .mesh = getAssetManager()->fetchMesh("flipper.obj"),
            .material = {
                .shader = m_shaderModernOpaque,
                .name = "Flipper",
                .ambient = {0.5f, 0.5f, 0.5f},
                .diffuse = {1.0f, 1.0f, 1.0f},
                .metallic = 1,
                .roughness = 1,
                .diffuseTex = getAssetManager()->fetchTexture("flipper_albedo.png"),
                .metaTex = getAssetManager()->fetchTexture("flipper_meta.png"),
                .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
            }
            })
        .withBehaviour<GraphicsMode>(true, m_shaderClassic)
    );

    EntityBuilder brickContainer;
    brickContainer.withName("BrickContainer")
        .withPosition({ -15.75f, 5.980f, 0 })
        .withParticleSystem({
            .material = {
                .shader = m_shaderParticle,
                .diffuse = {1,1,1},
                .diffuseTex = getAssetManager()->fetchTexture("particles/dirt_particles.png"),
                .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr"),
                .drawOrder = k_drawOrder_Transparent,
            },
            .blendState = m_ballParticleBlendState,
            .particleTextureCount = 3,
        });
    for (int x = 0; x < k_BRICKS_COLUMNS; x++) {
        for (int y = 0; y < k_BRICKS_ROWS; y++) {
            brickContainer.withChild(
                EntityBuilder().withName(fmt::format("Brick_{}_{}", x, y))
                .withPosition({ x * 3.5f, y * 2.0f, 0 })
                .withMeshRenderer({
                    .mesh = getAssetManager()->fetchMesh("brick.obj"),
                    .material = {
                        .shader = m_shaderModernOpaque,
                        .name = "Brick",
                        .ambient = {0.5f, 0.5f, 0.5f},
                        .diffuse = {1.0f, 1.0f, 1.0f},
                        .metallic = 0.0f,
                        .roughness = 1.0f,
                        .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                        .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                } })
                .withBehaviour<Brick>(true, x, y)
                .withBehaviour<GraphicsMode>(true, m_shaderClassic)
                );
        }
    }

    outScene.push_back(brickContainer);

    outScene.push_back(
        EntityBuilder().withName("PowerupsContainer")
    );

    outScene.push_back(
        EntityBuilder().withName("EnemiesContainer")
        .withEnabled(false)
        .withChild(
            EntityBuilder().withName("Smile")
            .withPosition({ 0, 0, 0 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("smile.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "HorrorsBeyondMyImagination",
                    .ambient = {0.5f, 0.5f, 0.5f},
                    .diffuse = {1.0f, 1.0f, 1.0f},
                    .roughness = 1.0f,
                    .diffuseTex = getAssetManager()->fetchTexture("smile_albedo.png"),
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
    );

    outScene.push_back(
        EntityBuilder().withName("ObstacleContainer")
        .withChild(
            EntityBuilder().withName("Bumper")
            .withEnabled(false)
            .withPosition({ 0,0,0 })
            .withRotationEulerAngles({ 0 * DEG2RAD, 0 * DEG2RAD, 0 * DEG2RAD })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("bumper.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "Bumper",
                    .ambient = {0.5f, 0.5f, 0.5f},
                    .diffuse = {1.0f, 1.0f, 1.0f},
                    .metallic = 1.0f,
                    .roughness = 1.0f,
                    .diffuseTex = getAssetManager()->fetchTexture("bumper_albedo.png"),
                    .metaTex = getAssetManager()->fetchTexture("bumper_meta.png"),
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
        .withChild(
            EntityBuilder().withName("RampLeft")
            .withEnabled(false)
            .withPosition({ 0,0,0 })
            .withRotationEulerAngles({ 0,0,0 })
            .withMeshRenderer({
                .mesh = getAssetManager()->fetchMesh("box.obj"),
                .material = {
                    .shader = m_shaderModernOpaque,
                    .name = "Ramp",
                    .ambient = {0.5f, 0.5f, 0.5f},
                    .diffuse = {1.0f, 1.0f, 1.0f},
                    .metallic = 1.0f,
                    .roughness = 1.0f,
                    .diffuseTex = getAssetManager()->fetchTexture("bumper_albedo.png"),
                    .metaTex = getAssetManager()->fetchTexture("bumper_meta.png"),
                    .matcapTex = getAssetManager()->fetchTexture("hdri_matcap.png"),
                    .brdfLutTex = getAssetManager()->fetchTexture("dfg.hdr")
                }
                })
            .withBehaviour<GraphicsMode>(true, m_shaderClassic)
        )
    );

    outScene.push_back(
        EntityBuilder().withName("GameManager")
        .withBehaviour<LevelHandler>(true, m_shaderClassic)
    );

    outScene.push_back(
        EntityBuilder().withName("UICanvas")
        .withUiCanvas()
        .withChild(
            EntityBuilder().withName("LivesUI")
            .withUiText({
                .posX = -17.500f,
                .posY = -9.8f,
                .text = "<LIVES PLACEHOLDER>",
                .textScale = 1.25f,
                })
            )
        .withChild(
            EntityBuilder().withName("LevelsUI")
            .withUiText({
                .posX = 13.700f,
                .posY = -9.8f,
                .text = "<LEVELS PLACEHOLDER>",
                .textScale = 1.25f,
                })
            )
        .withChild(
            EntityBuilder().withName("ScoreUI")
            .withUiText({
                .posX = -1.850f,
                .posY = -9.8f,
                .text = "<SCORE PLACEHOLDER>",
                .textScale = 1.25f,
                })
            )

        .withChild(
            EntityBuilder().withName("GameOverScreen")
            .withUiSprite({
                .posX = 0,
                .posY = 0,
                .sizeX = 4000,
                .sizeY = 4000,
                .textureTint = hlslpp::float4(0,0,0,0.25f),
                .texture = getAssetManager()->fetchWhiteTexture(),
                })
                .withChild(EntityBuilder().withName("Text")
                    .withUiText({
                        .posX = -3.13f,
                        .posY = -2.17f,
                        .text = "GAME OVER!",
                        .textScale = 2.17f,
                        }))
                .withChild(EntityBuilder().withName("GameOver_Username_Tooltip")
                    .withUiText({
                        .posX = -10.82f,
                        .posY = 0.08f,
                        .text = "Insert your name:",
                        .textScale = 1.1f,
                        }))
                .withChild(EntityBuilder().withName("GameOver_Username_TextBuffer")
                    .withUiText({
                        .posX = -1.11f,
                        .posY = 0.11f,
                        .text = "YOUR NAME HERE",
                        .textScale = 1.25f,
                        }))
                .withChild(
                EntityBuilder().withName("RetryButton")
                .withUiSprite({
                    .posX = 200,
                    .posY = 280,
                    .sizeX = 200,
                    .sizeY = 72,
                    .texture = getAssetManager()->fetchTexture("ui/button_play.png"),
                    })
                    .withBehaviour<GameplayUiInteractions>(true, outScene.layer, GameplayButtonClass::Restart)
                    .withChild(
                    EntityBuilder().withName("RetryButton_Text")
                    .withUiText({
                        .posX = 2.29f,
                        .posY = 5.39f,
                        .text = "Retry",
                        .textScale = 1.46f,
                        }))
            ).withChild(
                EntityBuilder().withName("QuitButton")
                .withUiSprite({
                    .posX = -200,
                    .posY = 280,
                    .sizeX = 200,
                    .sizeY = 72,
                    .texture = getAssetManager()->fetchTexture("ui/button_play.png"),
                    })
                    .withBehaviour<GameplayUiInteractions>(true, outScene.layer, GameplayButtonClass::ReturnToMenu)
                    .withChild(
                    EntityBuilder().withName("QuitButton_Text")
                    .withUiText({
                        .posX = -4.72f,
                        .posY = 5.39f,
                        .text = "Quit",
                        .textScale = 1.46f,
                        }))
            )
            .withChild(EntityBuilder().withName("GameOver_Leaderboard")
                .withUiText({
                    .posX = -1.590f,
                    .posY = 0.0f,
                    .text = "leaderboard placeholder",
                    .textScale = 0.9f,
                    }))
        )

        .withChild(
            EntityBuilder().withName("WinScreen")
            .withUiSprite({
                .posX = 0,
                .posY = 0,
                .sizeX = 4000,
                .sizeY = 4000,
                .textureTint = hlslpp::float4(0,0,0,0.25f),
                .texture = getAssetManager()->fetchWhiteTexture(),
                })
                .withChild(EntityBuilder().withName("Text")
                    .withUiText({
                        .posX = -2.03f,
                        .posY = -2.17f,
                        .text = "VICTORY!",
                        .textScale = 2.17f,
                        }))
                .withChild(EntityBuilder().withName("Victory_Username_Tooltip")
                    .withUiText({
                        .posX = -10.82f,
                        .posY = 0.08f,
                        .text = "Insert your name:",
                        .textScale = 1.1f,
                        }))
                .withChild(EntityBuilder().withName("Victory_Username_TextBuffer")
                    .withUiText({
                        .posX = -1.11f,
                        .posY = 0.11f,
                        .text = "YOUR NAME HERE",
                        .textScale = 1.25f,
                        }))
                .withChild(
                EntityBuilder().withName("RetryButton")
                .withUiSprite({
                    .posX = 200,
                    .posY = 280,
                    .sizeX = 260,
                    .sizeY = 72,
                    .texture = getAssetManager()->fetchTexture("ui/button_play.png"),
                    })
                    .withBehaviour<GameplayUiInteractions>(true, outScene.layer, GameplayButtonClass::Restart)
                    .withChild(
                    EntityBuilder().withName("RetryButton_Text")
                    .withUiText({
                        .posX = 1.79f,
                        .posY = 6.58f,
                        .text = "Play Again",
                        .textScale = 1.18f,
                        }))
            ).withChild(
                EntityBuilder().withName("QuitButton")
                .withUiSprite({
                    .posX = -200,
                    .posY = 280,
                    .sizeX = 200,
                    .sizeY = 72,
                    .texture = getAssetManager()->fetchTexture("ui/button_play.png"),
                    })
                    .withBehaviour<GameplayUiInteractions>(true, outScene.layer, GameplayButtonClass::ReturnToMenu)
                    .withChild(
                    EntityBuilder().withName("QuitButton_Text")
                    .withUiText({
                        .posX = -4.72f,
                        .posY = 5.39f,
                        .text = "Quit",
                        .textScale = 1.46f,
                        }))
            )
            .withChild(EntityBuilder().withName("Victory_Leaderboard")
            .withUiText({
                .posX = -1.590f,
                .posY = 0.0f,
                .text = "leaderboard placeholder",
                .textScale = 0.9f,
            })))
    );
}