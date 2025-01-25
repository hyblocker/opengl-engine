#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <vector>

#include "engine/gpu/idevice.hpp"
#include "engine/renderer/skybox.hpp"

namespace render {

    class Camera;
    class Entity;

    struct Transform {
        friend class Camera;

        inline const hlslpp::float3 getPosition() const { return m_position;}
        inline const hlslpp::quaternion getRotation() const { return m_rotation; }
        inline const hlslpp::float3 getScale() const { return m_scale; }

        void setPosition(hlslpp::float3 newPosition);
        void setRotation(hlslpp::quaternion newRotation);
        void setScale(hlslpp::float3 newScale);

        const hlslpp::float4x4 getModel();

    private:

        hlslpp::float3 m_position = { 0.0f, 0.0f, 0.0f };
        hlslpp::quaternion m_rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // euler angles are useful to work with, but error-prone, so we expose helper functions to interact with them
        hlslpp::float3 m_scale = { 1.0f, 1.0f, 1.0f };

        bool m_isDirty = true;
        hlslpp::float4x4 m_model = hlslpp::float4x4::identity();
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
        friend class Entity;
        friend class Scene;
    public:
        IComponent(Entity* parent) : m_parent (parent) {}
        bool enabled = true;

        inline ComponentType getComponentType() const { return componentType; }
        inline Entity* getEntity() const { return m_parent; }
    protected:
        void setParent(Entity* entity) { m_parent = entity; }
        ComponentType componentType = ComponentType::Unknown;
        Entity* m_parent = nullptr;
    };

    // One is supposed to inherit from this and extend the functions attached here to define custom behaviour on entities
    class IBehaviour : public IComponent {
    public:
        IBehaviour(Entity* parent) : IComponent(parent) {
            componentType = ComponentType::UserBehaviour;
        }
        ~IBehaviour() = default;

        virtual void start() {}; // called on scene load
        virtual void sleep() {}; // called on scene unload
        virtual void update(const float deltaTime) {}; // called every update tick
        virtual void render() {}; // called every frame

    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };

    class Entity {
    public:
        std::string name = "";
        bool enabled = true;
        Entity* parent = nullptr; // If null, assume this is a root node, or leaked entity
        Transform transform;
        std::vector<Entity> children;

        // optional "layers", to allow one to attach arbitrary data to an entity
        std::vector<IComponent> components;

        // finds an entity with a given type in the list of child entities
        Entity* findEntityWithType(ComponentType type) const;
        IComponent* findComponent(ComponentType type) const;
        void push_back(Entity& entity);
        void push_back(IComponent& component);
    };

    class Scene {
    public:
        std::string sceneName = "";
        std::vector<Entity> entities;
        struct LightingParameters {
            Entity* sunLight = nullptr; // Reference to the entity whose Light component represents the sun, data passed onto skybox shader
            Skybox skybox;
        } lightingParams;

        void push_back(Entity& entity);
        Entity* findEntityWithType(ComponentType type) const;
        IComponent* findComponent(ComponentType type) const;
    };
}