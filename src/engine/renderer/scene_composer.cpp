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
        light->colour = params.colour;
        light->intensity = params.intensity;
        light->innerRadius = params.innerRadius;
        light->outerRadius = params.outerRadius;

        m_entity->push_back(light);
        return *this;
    }

    EntityBuilder& EntityBuilder::withPhysics(PhysicsCreateParams params) {
        std::shared_ptr<physics::PhysicsComponent> physicsComponent = std::make_shared<physics::PhysicsComponent>(m_entity.get());

        physicsComponent->enabled = params.enabled;
        physicsComponent->density = params.density;
        physicsComponent->friction = params.friction;
        physicsComponent->bounciness = params.bounciness;
        physicsComponent->gravityScale = params.gravityScale;
        physicsComponent->fixedRotation = params.fixedRotation;
        physicsComponent->bodyType = params.bodyType;
        physicsComponent->shape.shape = params.shape.shape;
        physicsComponent->shape.box = params.shape.box;
        physicsComponent->shape.circle = params.shape.circle;
        physicsComponent->shape.capsule = params.shape.capsule;

        m_entity->push_back(physicsComponent);
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

    EntityBuilder& EntityBuilder::withParticleSystem(ParticleSystemCreateParams params) {
        std::shared_ptr<ParticleSystem> particleSystem = std::make_shared<ParticleSystem>(m_entity.get());

        particleSystem->enabled = params.enabled;
        particleSystem->material = params.material;
        particleSystem->blendState = params.blendState;
        particleSystem->particleTextureCount = params.particleTextureCount;

        m_entity->push_back(particleSystem);
        return *this;
    }

    EntityBuilder& EntityBuilder::withUiCanvas(bool enabled) {
        std::shared_ptr<UICanvas> uiCanvas = std::make_shared<UICanvas>(m_entity.get());

        uiCanvas->enabled = enabled;

        m_entity->push_back(uiCanvas);
        return *this;
    }

    EntityBuilder& EntityBuilder::withUiSprite(UiSpriteCreateParams params) {
        std::shared_ptr<UIElement> uiElement = std::make_shared<UIElement>(m_entity.get());

        uiElement->enabled = params.enabled;
        uiElement->uiType = render::UIElementType::Sprite;
        uiElement->texture = params.texture;
        uiElement->textureTint = params.textureTint;

        m_entity->push_back(uiElement);
        return *this;
    }

    EntityBuilder& EntityBuilder::withUiText(UiTextCreateParams params) {
        std::shared_ptr<UIElement> uiElement = std::make_shared<UIElement>(m_entity.get());

        uiElement->enabled = params.enabled;
        uiElement->uiType = render::UIElementType::Text;
        uiElement->text = params.text;
        uiElement->textColour = params.textColour;
        uiElement->outlineColour = params.outlineColour;
        uiElement->outlineWidth = params.outlineWidth;

        m_entity->push_back(uiElement);
        return *this;
    }
}