#pragma once

#include "engine/gpu/idevice.hpp"

namespace render {

    enum class SkyboxType : uint8_t {
        Procedural,
        HDRI,
        Count,
    };

    struct Skybox
    {
        SkyboxType type = SkyboxType::Procedural;
        gpu::ITexture* m_skyTexture = nullptr; // skybox HDRI texture

        // @TODO: Skybox params
    };
}