#include "asset_manager.hpp"
#include "engine/log.hpp"

#include <filesystem>

#include <stb_image.h>
#include <fmt/format.h>
#include <windows.h>
#include <stb_include.h>

#define SHADER_HEADER "#version 460 core\n\n"

namespace managers {
    AssetManager::AssetManager(gpu::IDevice* device) 
        : m_device(device), m_intialisedDefaultAssets(false)
    {
        m_applicationRootPath = getExecutableDir();

        initialiseErrorData();
    }

    AssetManager::~AssetManager() {}

    // hopefully cross-platform manner of getting exe dir
    // https://stackoverflow.com/a/55579815
    std::string AssetManager::getExecutableDir() {
        // Get exe path
#ifdef _WIN32
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return std::filesystem::path(path).parent_path().string();
#else
        // @NOTE: Untested
        char result[PATH_MAX] = {};
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string path std::string(result, (count > 0) ? count : 0);
        return std::filesystem::path(path).parent_path().string();
#endif
    }

    void AssetManager::initialiseErrorData() {

        // Initialises error mesh, error shader and texture

        // Init errTex
        uint8_t texData[] = {
            0xFF, 0x00, 0xFF, 0xFF,
            0x00, 0x00, 0x00, 0xFF,
            0x00, 0x00, 0x00, 0xFF,
            0xFF, 0x00, 0xFF, 0xFF,
        };
        m_errorTexture = m_device->makeTexture({
            .width = (uint32_t)2,
            .height = (uint32_t)2,
            .generateMipmaps = false,
            .type = gpu::TextureType::Texture2D,
            }, texData);

        // Init errShader

        constexpr const char shader_vert[] = SHADER_HEADER R"(
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColor;
layout (location = 2) in vec2 iUv;

out vec3 color;
out vec2 uv;

void main()
{
    gl_Position = vec4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    color = iColor;
    uv = iUv.xy;
}
)";
        constexpr const char shader_pixel[] = SHADER_HEADER R"(
precision mediump float;

// gl_FragColor is deprecated in GLSL 4.4+
layout (location = 0) out vec4 fragColor;
layout (binding = 0) uniform sampler2D texUnknown;

in vec3 color;
in vec2 uv;

void main()
{
    fragColor = texture(texUnknown, uv).rgba;
} 
)";

        gpu::VertexAttributeDesc vDesc[] = {
            {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(render::PositionColorVertex, position), .elementStride = sizeof(render::PositionColorVertex)},
            {.name = "COLOR", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(render::PositionColorVertex, color), .elementStride = sizeof(render::PositionColorVertex)},
            {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 2, .offset = offsetof(render::PositionColorVertex, uv), .elementStride = sizeof(render::PositionColorVertex)}
        };
        auto vertexLayoutHandle = m_device->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));

        // Make a shader but awesome
        m_errorShader = m_device->makeShader({
            .debugName = "ErrorShader",
            .VS {.byteCode = (uint8_t*)shader_vert, .entryFunc = "main" },
            .PS {.byteCode = (uint8_t*)shader_pixel, .entryFunc = "main" },
            .graphicsState = {
                .depthState = gpu::CompareFunc::LessOrEqual, // inverse Z
                .vertexLayout = vertexLayoutHandle,
        } });

        m_intialisedDefaultAssets = true;
    }

    render::Mesh& AssetManager::fetchMesh(const std::string& meshPath) {

        if (!m_intialisedDefaultAssets) {
            initialiseErrorData();
        }



        return m_errorMesh;
    }
    gpu::IShader* AssetManager::fetchShader(const FetchShaderParams& params) {
        
        if (!m_intialisedDefaultAssets) {
            initialiseErrorData();
        }

        // Check if the shader was loaded before
        std::string shaderKey = params.vertShader + params.fragShader;
        if (m_shaders.find(shaderKey) != m_shaders.end()) {
            return m_shaders[shaderKey];
        }

        // Shader pair is not in cache, try loading from disk

        std::string filePathVert = fmt::format("{}/assets/shaders/{}", m_applicationRootPath, params.vertShader);
        std::string filePathFrag = fmt::format("{}/assets/shaders/{}", m_applicationRootPath, params.fragShader);

        char stbError[256] = {};
        std::string vertContents = SHADER_HEADER;
        std::string fragContents = SHADER_HEADER;
        
        // stb_include is used to be able to separate common buffers between shaders
        // e.g. light data, camera data, etc
        {
            char* vertContentsRaw = stb_include_file(filePathVert.data(), nullptr, fmt::format("{}/assets/shaders", m_applicationRootPath).data(), stbError);
            if (vertContentsRaw != nullptr) {
                // Sucessfully loaded the file
                vertContents += vertContentsRaw;
                free(vertContentsRaw);
            } else {
                LOG_ERROR("An error occured when loading the vertex shader {}. Got error: {}", params.vertShader, stbError);
                return m_errorShader;
            }
        }
        {
            char* fragContentsRaw = stb_include_file(filePathFrag.data(), nullptr, fmt::format("{}/assets/shaders", m_applicationRootPath).data(), stbError);
            if (fragContentsRaw != nullptr) {
                // Sucessfully loaded the file
                fragContents += fragContentsRaw;
                free(fragContentsRaw);
            } else {
                LOG_ERROR("An error occured when loading the fragment shader {}. Got error: {}", params.fragShader, stbError);
                return m_errorShader;
            }
        }

        auto shaderHandle = m_device->makeShader({
            .debugName = params.debugName,
            .VS {.byteCode = (uint8_t*)vertContents.c_str(), .entryFunc = params.vertShaderEntryFunction },
            .PS {.byteCode = (uint8_t*)fragContents.c_str(), .entryFunc = params.fragShaderEntryFunction },
            .graphicsState = params.graphicsState });

        if (shaderHandle.Get() != nullptr) {
            // Cache
            m_shaders.emplace(shaderKey, shaderHandle);
            return shaderHandle;
        } else {
            return m_errorShader;
        }
    }
    gpu::ITexture* AssetManager::fetchTexture(const std::string& texturePath, bool genMipmaps) {

        if (!m_intialisedDefaultAssets) {
            initialiseErrorData();
        }

        // Check if the texture was loaded before
        if (m_textures.find(texturePath) != m_textures.end()) {
            return m_textures[texturePath];
        }

        int texWidth, texHeight, nrChannels;
        unsigned char* texData = stbi_load(fmt::format("{}/assets/textures/{}", m_applicationRootPath, texturePath).c_str(), &texWidth, &texHeight, &nrChannels, STBI_rgb_alpha);

        gpu::TextureHandle textureHandle = m_device->makeTexture({
            .width = (uint32_t)texWidth,
            .height = (uint32_t)texHeight,
            .generateMipmaps = genMipmaps,
            .type = gpu::TextureType::Texture2D,
            }, texData);

        if (textureHandle.Get() != nullptr) {
            // Cache
            m_textures.emplace(texturePath, textureHandle);
            return textureHandle;
        } else {
            return m_errorTexture;
        }
    }
}
