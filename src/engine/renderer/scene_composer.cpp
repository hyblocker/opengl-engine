#include "scene_composer.hpp"

namespace render {

    using namespace ::hlslpp;

    EntityBuilder& EntityBuilder::withCamera(CameraCreateParams params) {
        std::shared_ptr<Camera> camera = std::make_shared<Camera>(m_entity.get());
        
        camera->enabled = params.enabled;
        camera->setProjection(params.projection);
        camera->setFov(params.fov);
        camera->setNearPlane(params.nearPlane);
        camera->setFarPlane(params.farPlane);
        camera->setInfiniteFar(params.infiniteFar);

        m_entity->push_back(camera);
        return *this;
    }

    EntityBuilder& EntityBuilder::withLight(LightCreateParams params) {
        std::shared_ptr<Light> light = std::make_shared<Light>(m_entity.get());

        light->enabled = params.enabled;
        light->type = params.type;
        light->attenuation = params.attenuation;
        light->position = params.position;
        light->direction = params.direction;
        light->colour = params.colour;
        light->intensity = params.intensity;
        light->radius = params.radius;

        m_entity->push_back(light);
        return *this;
    }

    EntityBuilder& EntityBuilder::withMeshRenderer(MeshRendererCreateParams params) {
        std::shared_ptr<MeshRenderer> renderer = std::make_shared<MeshRenderer>(m_entity.get());

        renderer->enabled = params.enabled;
        renderer->material = params.material;
        renderer->mesh = params.mesh;

        m_entity->push_back(renderer);
        return *this;
    }
}