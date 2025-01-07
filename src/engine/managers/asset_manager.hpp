#pragma once
#include <inttypes.h>
#include <hlsl++.h>
#include <vector>
#include <set>
#include <unordered_map>
#include "engine/gpu/idevice.hpp"
#include "engine/renderer/mesh.hpp"

namespace managers {
    class AssetManager {
        inline static AssetManager* getInstance() {
            return nullptr;
        }

        // render::Mesh& fetchMesh(const std::string& meshPath);
        // gpu::IShader& fetchShader(const std::string& shaderPath);
        // gpu::ITexture& fetchTexture(const std::string& texturePath);
        
    private:
        std::unordered_map<std::string, gpu::ShaderHandle> m_shaders;
        std::unordered_map<std::string, gpu::TextureHandle> m_textures;
        std::unordered_map<std::string, render::Mesh> m_meshes;

        gpu::ShaderHandle m_shaders;
        gpu::TextureHandle m_textures;
        render::Mesh m_errorMesh;
    };
}