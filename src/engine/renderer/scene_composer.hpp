#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <vector>
#include <memory>

#include "engine/gpu/idevice.hpp"
#include "engine/renderer/skybox.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "engine/physics/physics_components.hpp"
#include "scene_graph.hpp"

namespace render {

    class EntityBuilder {
        friend class Scene;
        friend class Entity;
    public:

        inline EntityBuilder& withName(const std::string& name) {
            m_entity->name = name;
            return *this;
        }

        inline EntityBuilder& withEnabled(bool enabled) {
            m_entity->enabled = enabled;
            return *this;
        }

        inline EntityBuilder& withChild(const EntityBuilder& child) {
            std::shared_ptr<Entity> childEntity = child.build();
            m_entity->push_back(childEntity);
            return *this;
        }

        // TRANSFORM

        inline EntityBuilder& withPosition(hlslpp::float3 position) {
            m_entity->transform.setPosition(position);
            return *this;
        }

        // ZXY order
        inline EntityBuilder& withRotationEulerAngles(hlslpp::float3 eulerAngles) {
            m_entity->transform.setRotation(hlslpp::quaternion::rotation_euler_zxy(eulerAngles));
            return *this;
        }

        inline EntityBuilder& withRotation(hlslpp::quaternion position) {
            m_entity->transform.setRotation(position);
            return *this;
        }

        inline EntityBuilder& withScale(hlslpp::float3 position) {
            m_entity->transform.setScale(position);
            return *this;
        }

        // COMPONENTS
        struct CameraCreateParams {
            bool enabled = true;
            render::CameraProjection projection = render::CameraProjection::Perspective;
            float fov = 60.0f; // degrees
            float nearPlane = 0.01f;
            float farPlane = 100.0f; // Unused if infinite far is enabled
            bool infiniteFar = false; // infinite far perspective matrix
        };
        EntityBuilder& withCamera(CameraCreateParams params);

        struct LightCreateParams {
            bool enabled = true;
            ::render::LightType type = ::render::LightType::Directional;

            // RGB colour
            hlslpp::float3 colour = { 1.0f, 1.0f, 1.0f };
            float intensity = 1.0f;
            float innerRadius = 0.5f; // For spot lights
            float outerRadius = 1.0f; // For spot lights
        };
        EntityBuilder& withLight(LightCreateParams params);

        struct PhysicsCreateParams {
            bool enabled = true;

            float density = 1.0f;
            float friction = 0.3f;
            physics::PhysicsBodyType bodyType = physics::PhysicsBodyType::Rigidbody;
            physics::ShapeParams shape = {};
        };
        EntityBuilder& withPhysics(PhysicsCreateParams params);

        struct MeshRendererCreateParams {
            bool enabled = true;
            render::Mesh mesh{};
            render::Material material;
        };
        EntityBuilder& withMeshRenderer(MeshRendererCreateParams params);

    private:
        inline std::shared_ptr<Entity> build() {
            return m_entity;
        }
        inline const std::shared_ptr<Entity> build() const {
            return m_entity;
        }
    private:
        std::shared_ptr<Entity> m_entity = std::make_shared<Entity>();
    };
}