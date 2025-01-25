#include "scene_renderer.hpp"
#include "engine/log.hpp"

namespace render {

    void Renderer::init() {
        // prepare renderer state

        // 1. load skybox

        // 2. create global shared resources states, passed to the draw functions
    }

    void Renderer::drawSkybox(const Scene& scene, Camera* cameraComponent) {
        switch (scene.lightingParams.skybox.type) {
        case SkyboxType::Procedural:
        {
            // @TODO: Draw procedural skybox
            break;
        }
        case SkyboxType::HDRI:
        {
            // @TODO: Draw HDRI skybox (read: cubemap)
            break;
        }
        default:
            LOG_WARN("Unknown skybox type {}! Not drawing skybox...", (uint8_t) scene.lightingParams.skybox.type);
            break;
        }
    }

    void Renderer::draw(const Entity& entity) {

    }

    void Renderer::draw(const Scene& scene) {

        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        // Find the camera
        // @TODO: Consider caching the camera entity??
        Entity* cameraEntity = scene.findEntityWithType(ComponentType::Camera);
        Camera* cameraComponent = (Camera*) cameraEntity->findComponent(ComponentType::Camera);
        
        drawSkybox(scene, cameraComponent);

        // scene.entities.

    }
}