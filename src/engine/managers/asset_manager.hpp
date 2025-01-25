#pragma once
#include <inttypes.h>
#include <hlsl++.h>
#include <vector>
#include <string>
#include <unordered_map>
#include "engine/gpu/idevice.hpp"
#include "engine/renderer/mesh.hpp"

namespace managers {

    // helper
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

    class AssetManager {
    public:
        AssetManager(gpu::IDevice* device);
        ~AssetManager();

        struct FetchShaderParams {
            gpu::GraphicsState graphicsState;
            std::string vertShader;
            std::string fragShader;
            std::string vertShaderEntryFunction = "main";
            std::string fragShaderEntryFunction = "main";
            std::string debugName = "";
        };
        
        render::Mesh& fetchMesh(const std::string& meshPath);
        gpu::IShader* fetchShader(const FetchShaderParams& params);
        gpu::ITexture* fetchTexture(const std::string& texturePath, const bool genMipmaps = true);
        
    private:
        void initialiseErrorData();
        std::string getExecutableDir();

    private:

        // Keeps track of mesh data for caching and memory management purposes
        class MeshTracker_t {
        public:
            // Keep the handles for free memory management
            gpu::BufferHandle vertexBuffer;
            gpu::BufferHandle indexBuffer;
            gpu::InputLayoutHandle vertexLayout;
            // Pointers, passed to "userland" code
            render::Mesh mesh;
        };

        std::unordered_map<std::string, gpu::ShaderHandle> m_shaders;
        std::unordered_map<std::string, gpu::TextureHandle> m_textures;
        std::unordered_map<std::string, MeshTracker_t> m_meshes;

        gpu::ShaderHandle m_errorShader;
        gpu::TextureHandle m_errorTexture;
        MeshTracker_t m_errorMesh;

        bool m_intialisedDefaultAssets = false;
        gpu::IDevice* m_device;

        std::string m_applicationRootPath;
    };
}