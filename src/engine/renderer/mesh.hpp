#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include <vector>

namespace render {

    // Commonly used vertex layouts
    struct PositionColorVertex {
        float position[3] = {};
        float color[3] = {};
        float uv[2] = {};
    };

    struct PositionNormalTexcoordVertex {
        float position[3] = {};
        float normal[3] = {};
        float uv[2] = {};
    };

    // Collection of mesh data
    struct Mesh {
    public:
        gpu::IBuffer* vertexBuffer = nullptr;
        gpu::IBuffer* indexBuffer = nullptr;
        gpu::IInputLayout* vertexLayout = nullptr; // WHY IS VAO TIED TO THE VERTEX BUFFER?????
        size_t triangleCount = 0;
    };
}