#include "camera.hpp"
#include "engine/app.hpp"

namespace render {

    using namespace ::hlslpp;

    void Camera::setProjection(CameraProjection newProjection) {
        m_projection = newProjection;
        m_isPerspectiveDirty = true;
    }
    void Camera::setFov(float newFov) {
        m_fov = newFov;
        m_isPerspectiveDirty = true;
    }
    void Camera::setNearPlane(float newNearPlane) {
        m_nearPlane = newNearPlane;
        m_isPerspectiveDirty = true;
    }
    void Camera::setFarPlane(float newFarPlane) {
        m_farPlane = newFarPlane;
        m_isPerspectiveDirty = true;
    }
    void Camera::setInfiniteFar(bool newInfiniteFarPlane) {
        m_infiniteFar = newInfiniteFarPlane;
        m_isPerspectiveDirty = true;
    }
    void Camera::setAspect(float newAspect) {
        if (newAspect != m_aspect) {
            m_aspect = newAspect;
            m_isPerspectiveDirty = true;
        }
    }

    const float4x4 Camera::getProjectionMatrix() {
        if (m_isPerspectiveDirty) {

            // Temp
            hlslpp::projection projectionProps = hlslpp::projection(frustum(0,0,0,0),zclip::minus_one);

            float fovRadians = radians(float1(m_fov));
            float frustumHeight = 2.0f * tan(fovRadians / 2.0f) * m_nearPlane;
            float frustumWidth = frustumHeight * m_aspect;

            if (m_infiniteFar) {
                projectionProps = hlslpp::projection(
                    frustum(
                        /* width */ frustumWidth,
                        /* height */ frustumHeight,
                        /* near_z */ m_nearPlane,
                        /* far_z */ m_farPlane),
                    zclip::minus_one, zdirection::reverse, zplane::infinite);
            }
            else {
                projectionProps = hlslpp::projection(
                    frustum(
                        /* width */ frustumWidth,
                        /* height */ frustumHeight,
                        /* near_z */ m_nearPlane,
                        /* far_z */ m_farPlane),
                    zclip::minus_one, zdirection::reverse, zplane::finite);
            }

            if (m_projection == CameraProjection::Perspective) {
                m_matPerspective = float4x4::perspective(projectionProps);
            } else {
                m_matPerspective = float4x4::orthographic(projectionProps);
            }

            // Use this to remap depth NDC from -1 - +1 (DX11) to 0 - +1 (GL)
            // https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
            const float4x4 normalize_range = float4x4(
                float4(1.0f, 0.0f, 0.0f, 0.0f),
                float4(0.0f, 1.0f, 0.0f, 0.0f),
                float4(0.0f, 0.0f, 0.5f, 0.0f),
                float4(0.0f, 0.0f, 0.5f, 1.0f)
            );

            m_matPerspective = mul(m_matPerspective, normalize_range);

            m_isPerspectiveDirty = false;
        }
        return m_matPerspective;
    }
    
    const float4x4 Camera::getViewMatrix() {
        if (m_isViewDirty || getEntity()->transform.m_isDirty) {

            float3 position = getEntity()->transform.m_position;

            float3 forward = normalize(mul(getEntity()->transform.m_rotation, float3(0.0f, 0.0f, -1.0f)));
            // float3 up = normalize(mul(getEntity()->transform.m_rotation, float3(0.0f, 1.0f, 0.0f)));

            // 2nd arg is target vector, a point in 3d space relative to position
            // idk hlslpp doesn't let you feed in a direction vector because this just makes the look vector lose precision so i think this is a dumb choice
            m_matView = float4x4::look_at(position, position + forward, hlslpp::float3(0.0f, 1.0f, 0.0f));
            m_isViewDirty = false;

            // Update model matrix on the camera component if necessary
            getEntity()->transform.getModel();
        }
        return m_matView;
    }
}