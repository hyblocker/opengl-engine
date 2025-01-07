#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include <vector>

namespace render {
    struct Mesh {
    private:

        gpu::IBuffer* m_vertexBuffer;
        gpu::IBuffer* m_indexBuffer;
    };
}