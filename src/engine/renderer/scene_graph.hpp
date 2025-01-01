#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <vector>

#include "engine/gpu/idevice.hpp"
#include "engine/renderer/skybox.hpp"

namespace render {

    struct Transform {
        hlslpp::float3 position = { 0.0f, 0.0f, 0.0f };
        hlslpp::quaternion rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // euler angles are useful to work with, but error-prone, so we expose helper functions to interact with them
        hlslpp::float3 scale = { 1.0f, 1.0f, 1.0f };

        hlslpp::float4x4 transformMatrix;
    };

    // Each of these is associated with a unique type of component. The components defined here are considered special cases and handled uniquely by the render loop
    enum class ComponentType {
        Unknown = 0,
        MeshRenderer,
        Light,
        ParticleSystem,
        Camera,
        UserBehaviour,
        UICanvas, // UI root object, converts matrix maths from world space to UI-space
        UIElement, // UI element, only allowed type in UICanvas objects
        Count,
    };

    // This operates on data, so that we avoid having multiple components on entities
    struct IComponent {
    public:
        bool enabled = true;
    protected:
        ComponentType componentType = ComponentType::Unknown;
    };

    // One is supposed to inherit from this and extend the functions attached here to define custom behaviour on entities
    class IBehaviour : public IComponent {
    public:
        IBehaviour() {
            componentType = ComponentType::UserBehaviour;
        }
        ~IBehaviour() = default;

        virtual void start() {}; // called on scene load
        virtual void sleep() {}; // called on scene unload
        virtual void update(float deltaTime) {}; // called every update tick
        virtual void render() {}; // called every frame

    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };

    class Entity {
    public:
        bool enabled = true;
        Entity* parent = nullptr; // If null, assume this is a root node, or leaked entity
        Transform transform;
        std::vector<Entity> children;

        // optional "layers", to allow one to attach arbitrary data to an entity
        std::vector<IComponent> components;
    };

    class Scene {
    public:
        std::vector<Entity> entities;
        struct LightingParameters {
            Entity* sunLight = nullptr; // Reference to the entity whose Light component represents the sun, data passed onto skybox shader
            Skybox skybox;
        } lightingParams;
    };
}