#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include <vector>

namespace render {

    // Commonly used vertex layouts
    struct PositionColorVertex {
        float position[3];
        float color[3];
        float uv[2];
    };

    struct Mesh {
    public:
        gpu::IBuffer* vertexBuffer;
        gpu::IBuffer* indexBuffer;
    };
}