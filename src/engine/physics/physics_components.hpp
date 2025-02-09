#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <box2d/box2d.h>

#include "engine/gpu/idevice.hpp"
#include "engine/renderer/scene_graph.hpp"

namespace render {
    class SceneUpdater;
}

namespace physics {

    constexpr int k_PHYSICS_SUBSTEP_COUNT = 4;
    constexpr int k_MAX_FORCES_PER_COMPONENT_PER_FRAME = 8;

    enum class PhysicsShape {
        Box,
        Circle,
        Capsule,
        Count,
    };

    enum class PhysicsBodyType {
        Static, // No mass, used for static parts of the scene e.g. ground
        Kinematic, // No mass, velocity updated by user, handled by solver
        Rigidbody, // Has mass, physics updated by solver
    };

    struct ShapeParams {
        PhysicsShape shape = PhysicsShape::Box;
        struct ShapeParams_Box {
            hlslpp::float2 size = { 1.0, 1.0 };
        } box;
        struct ShapeParams_Circle {
            hlslpp::float2 centre;
            float radius;
        } circle;
        struct ShapeParams_Capsule {
            hlslpp::float2 p1;
            hlslpp::float2 p2;
            float radius;
        } capsule;
    };

    class PhysicsComponent : public render::IComponent {
        friend class ::render::SceneUpdater;
    public:
        PhysicsComponent(render::Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::Physics;
        }
        ~PhysicsComponent() = default;

        float density = 1.0f;
        float friction = 0.3f;
        PhysicsBodyType bodyType = PhysicsBodyType::Rigidbody;
        ShapeParams shape = {};

        // queus a force into the physics system
        void addForce(const hlslpp::float2 force);

    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;

        // id of this component in the physics system
        b2BodyId m_physicsId = b2_nullBodyId;

        hlslpp::float2 m_pendingForces[k_MAX_FORCES_PER_COMPONENT_PER_FRAME] = {};
        uint32_t m_forceQueueSize = 0; // size of the forces buffer
    };
}