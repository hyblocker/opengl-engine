#pragma once

#include <hlsl++.h>

namespace render {

    enum class CameraProjection : uint8_t {
        Perspective,
        Orthographic,
    };

    struct Camera {
        CameraProjection projection = CameraProjection::Perspective;
        float fov = 60.0f; // degrees
        float nearPlane = 0.01f;
        float farPlane = 100.0f; // Unused if infinite far is enabled
        bool infiniteFar = false; // infinite far perspective matrix

    };
}
