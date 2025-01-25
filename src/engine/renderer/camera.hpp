#pragma once

#include <hlsl++.h>
#include "scene_graph.hpp"

namespace render {

    enum class CameraProjection : uint8_t {
        Perspective,
        Orthographic,
    };

    class Camera : public IComponent {
    public:
        Camera(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::Camera;
        }
        ~Camera() = default;

        const CameraProjection getProjection() const { return m_projection; }
        // degrees
        const float getFov() const { return m_fov; }
        const float getNearPlane() const { return m_nearPlane; }
        const float getFarPlane() const { return m_farPlane; }
        const bool getInfiniteFar() const { return m_infiniteFar; }
        const float getAspect() const { return m_aspect; }

        void setProjection(CameraProjection newProjection);
        void setFov(float newFov);
        void setNearPlane(float newNearPlane);
        void setFarPlane(float newFarPlane);
        void setInfiniteFar(bool newInfiniteFarPlane);
        void setAspect(float newAspect);

        const hlslpp::float4x4 getPerspective();
        const hlslpp::float4x4 getView();
    private:

        hlslpp::float4x4 m_matPerspective = hlslpp::float4x4::identity();
        hlslpp::float4x4 m_matView = hlslpp::float4x4::identity();

        CameraProjection m_projection = CameraProjection::Perspective;
        float m_fov = 60.0f; // degrees
        float m_nearPlane = 0.01f;
        float m_farPlane = 100.0f; // Unused if infinite far is enabled
        bool m_infiniteFar = false; // infinite far perspective matrix
        float m_aspect = 1.0f; // aspect ratio of buffer that's bound to the state machine. should be updated whenever it's resized (window resize)

        bool m_isPerspectiveDirty = true;
        bool m_isViewDirty = true;

        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
    };
}
