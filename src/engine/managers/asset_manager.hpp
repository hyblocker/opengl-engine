#pragma once
#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>
#include <vector>
#include <set>
#include <unordered_map>

namespace managers {
    class AssetManager {
        inline static AssetManager* getInstance() {
            return nullptr;
        }

        // std::set < gpu::IBuffer* /*vbuf*/ , gpu::IBuffer* /*ibuf*/ > fetchMesh(const std::string& meshPath);
        // gpu::IShader* fetchShader(const std::string& shaderPath);
        // gpu::ITexture* fetchTexture(const std::string& texturePath);
        
    private:
        std::unordered_map<std::string, gpu::ShaderHandle> m_shaders;
        std::unordered_map<std::string, gpu::TextureHandle> m_textures;
    };
}