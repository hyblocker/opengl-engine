#include "asset_manager.hpp"
#include "engine/log.hpp"

#include <filesystem>

#include <stb_image.h>
#include <fmt/format.h>
#include <tiny_obj_loader.h>
#include <windows.h>
#include <stb_include.h>

#define SHADER_HEADER "#version 460 core\n\n"

// @TODO: In a more production ready environment, replace this with a proper mesh, like the Source ERROR mesh
render::PositionNormalTexcoordVertex errorVertices[] = {
    // Front face
    {.position = { -1, -1,  1 }, .normal = {  0,  0,  1 }, .uv = { 0, 0 }},
    {.position = {  1, -1,  1 }, .normal = {  0,  0,  1 }, .uv = { 1, 0 }},
    {.position = {  1,  1,  1 }, .normal = {  0,  0,  1 }, .uv = { 1, 1 }},
    {.position = { -1,  1,  1 }, .normal = {  0,  0,  1 }, .uv = { 0, 1 }},

    // Back face
    {.position = {  1, -1, -1 }, .normal = {  0,  0, -1 }, .uv = { 0, 0 }},
    {.position = { -1, -1, -1 }, .normal = {  0,  0, -1 }, .uv = { 1, 0 }},
    {.position = { -1,  1, -1 }, .normal = {  0,  0, -1 }, .uv = { 1, 1 }},
    {.position = {  1,  1, -1 }, .normal = {  0,  0, -1 }, .uv = { 0, 1 }},

    // Left face
    {.position = { -1, -1, -1 }, .normal = { -1,  0,  0 }, .uv = { 0, 0 }},
    {.position = { -1, -1,  1 }, .normal = { -1,  0,  0 }, .uv = { 1, 0 }},
    {.position = { -1,  1,  1 }, .normal = { -1,  0,  0 }, .uv = { 1, 1 }},
    {.position = { -1,  1, -1 }, .normal = { -1,  0,  0 }, .uv = { 0, 1 }},

    // Right face
    {.position = {  1, -1,  1 }, .normal = {  1,  0,  0 }, .uv = { 0, 0 }},
    {.position = {  1, -1, -1 }, .normal = {  1,  0,  0 }, .uv = { 1, 0 }},
    {.position = {  1,  1, -1 }, .normal = {  1,  0,  0 }, .uv = { 1, 1 }},
    {.position = {  1,  1,  1 }, .normal = {  1,  0,  0 }, .uv = { 0, 1 }},

    // Top face
    {.position = { -1,  1,  1 }, .normal = {  0,  1,  0 }, .uv = { 0, 0 }},
    {.position = {  1,  1,  1 }, .normal = {  0,  1,  0 }, .uv = { 1, 0 }},
    {.position = {  1,  1, -1 }, .normal = {  0,  1,  0 }, .uv = { 1, 1 }},
    {.position = { -1,  1, -1 }, .normal = {  0,  1,  0 }, .uv = { 0, 1 }},

    // Bottom face
    {.position = { -1, -1, -1 }, .normal = {  0, -1,  0 }, .uv = { 0, 0 }},
    {.position = {  1, -1, -1 }, .normal = {  0, -1,  0 }, .uv = { 1, 0 }},
    {.position = {  1, -1,  1 }, .normal = {  0, -1,  0 }, .uv = { 1, 1 }},
    {.position = { -1, -1,  1 }, .normal = {  0, -1,  0 }, .uv = { 0, 1 }},
};

uint32_t errorIndices[] = {
    // Front face
    0, 1, 2,  2, 3, 0,

    // Back face
    4, 5, 6,  6, 7, 4,

    // Left face
    8, 9, 10,  10, 11, 8,

    // Right face
    12, 13, 14,  14, 15, 12,

    // Top face
    16, 17, 18,  18, 19, 16,

    // Bottom face
    20, 21, 22,  22, 23, 20,
};


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

        m_device->debugMarkerPush(fmt::format("Initialising error data..."));
        // Initialises error mesh, error shader and texture

        // Init errMesh
        m_errorMesh.vertexBuffer = m_device->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Default, .debugName = "ErrorVertexBuffer"});
        m_device->writeBuffer(m_errorMesh.vertexBuffer, sizeof(errorVertices), errorVertices);

        // Triangle indices
        m_errorMesh.indexBuffer = m_device->makeBuffer({ .type = gpu::BufferType::IndexBuffer, .usage = gpu::Usage::Default, .format = gpu::GpuFormat::Uint32_TYPELESS, .debugName = "ErrorIndexBuffer"});
        m_device->writeBuffer(m_errorMesh.indexBuffer, sizeof(errorIndices), errorIndices);

        // Requires mesh to be initialised first
        gpu::VertexAttributeDesc vDesc[] = {
            {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(render::PositionNormalTexcoordVertex, position), .elementStride = sizeof(render::PositionNormalTexcoordVertex)},
            {.name = "NORMAL", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(render::PositionNormalTexcoordVertex, normal), .elementStride = sizeof(render::PositionNormalTexcoordVertex)},
            {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 2, .offset = offsetof(render::PositionNormalTexcoordVertex, uv), .elementStride = sizeof(render::PositionNormalTexcoordVertex)}
        };

        m_errorMesh.vertexLayout = m_device->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));
        
        // Mesh data
        m_errorMesh.mesh.vertexBuffer = m_errorMesh.vertexBuffer;
        m_errorMesh.mesh.indexBuffer = m_errorMesh.indexBuffer;
        m_errorMesh.mesh.vertexLayout = m_errorMesh.vertexLayout;
        m_errorMesh.mesh.triangleCount = (sizeof(errorIndices) / sizeof(errorIndices[0])) / 3;

        // Init errTex
        uint8_t texDataErr[] = {
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
            .debugName = "ErrorTexture"
            }, texDataErr);

        // Init whiteTex
        uint8_t texDataWhite[] = {
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF,
        };
        m_whiteTexture = m_device->makeTexture({
            .width = (uint32_t)2,
            .height = (uint32_t)2,
            .generateMipmaps = false,
            .type = gpu::TextureType::Texture2D,
            .debugName = "WhiteTexture"
            }, texDataWhite);

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

        // Make a shader but awesome
        m_errorShader = m_device->makeShader({
            .VS {.byteCode = (uint8_t*)shader_vert, .entryFunc = "main" },
            .PS {.byteCode = (uint8_t*)shader_pixel, .entryFunc = "main" },
            .graphicsState = {
                .depthState = gpu::CompareFunc::LessOrEqual, // inverse Z
            },
            .debugName = "ErrorShader",
        });
        m_device->debugMarkerPop();

        m_intialisedDefaultAssets = true;
    }

    render::Mesh& AssetManager::fetchMesh(const std::string& meshPath) {

        if (!m_intialisedDefaultAssets) {
            initialiseErrorData();
        }

        // Check if the shader was loaded before
        std::string meshKey = meshPath;
        if (m_meshes.find(meshKey) != m_meshes.end()) {
            return m_meshes[meshKey].mesh;
        }
        
        std::string filePathMesh = fmt::format("{}/assets/meshes/{}", m_applicationRootPath, meshPath);

        // @HACK: This is super simple and poor for more sophisticated development
        //        1. We should use a more sophisticated library like ASSIMP or a mixture of them to better support more widely
        //           used mesh file formats such as Autodesk FBX and GLTF.
        //        2. Dynamically determine the vertex layout based on the mesh data, currently this is hardcoded to only support position normal and UV0 data, what if I want to import tangent vectors or UV1?
        //        3. Introduce a "baked" asset pipeline, where assets are pre-processed ahead of time from builds. This allows us to better optimise assets.
        //        4. Use meshoptimizer. meshoptimizer is a library which better distributes the data to make it easier for the GPU to work with, and can generate LODs and similar too.

        // Triangulate quads, ignore vertex colours and search for MTLs in the same dir as objs
        tinyobj::ObjReaderConfig reader_config;
        reader_config.triangulate = true;
        reader_config.triangulation_method = "earcut";
        reader_config.vertex_color = false;
        reader_config.mtl_search_path = "";

        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(filePathMesh, reader_config)) {
            if (!reader.Error().empty()) {
                LOG_ERROR("TinyObjReader: {}", reader.Error());
            }
            LOG_WARNING("Failed to load mesh {}. Using error mesh...", meshPath);
            return m_errorMesh.mesh;
        }

        if (!reader.Warning().empty()) {
            LOG_WARN("TinyObjReader: {}", reader.Warning());
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();

        std::vector<render::PositionNormalTexcoordVertex> vertices;
        std::vector<uint32_t> indices;

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            size_t index_buffer_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    render::PositionNormalTexcoordVertex currentVertex{};

                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    currentVertex.position[0] = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    currentVertex.position[1] = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    currentVertex.position[2] = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        currentVertex.normal[0] = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        currentVertex.normal[1] = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        currentVertex.normal[2] = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0) {
                        currentVertex.uv[0] = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        currentVertex.uv[1] = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    }

                    // Optional: vertex colors
                    // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                    // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                    // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

                    vertices.push_back(currentVertex);
                }
                for (uint32_t i = 0; i < fv; i += 3) {
                    indices.push_back(index_buffer_offset + i + 0);
                    indices.push_back(index_buffer_offset + i + 1);
                    indices.push_back(index_buffer_offset + i + 2);
                }
                index_offset += fv;
                index_buffer_offset += fv;

                // per-face material
                // shapes[s].mesh.material_ids[f];
            }
        }

        m_device->debugMarkerPush(fmt::format("Loading mesh {}...", meshPath));

        gpu::BufferHandle vertexBufferHandle;
        gpu::BufferHandle indexBufferHandle;

        // Vertex buffer
        vertexBufferHandle = m_device->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Default, .debugName = fmt::format("{}_vertexBuffer", meshPath) });
        m_device->writeBuffer(vertexBufferHandle, vertices.size() * sizeof(render::PositionNormalTexcoordVertex), vertices.data());

        // Triangle indices
        indexBufferHandle = m_device->makeBuffer({ .type = gpu::BufferType::IndexBuffer, .usage = gpu::Usage::Default, .format = gpu::GpuFormat::Uint32_TYPELESS, .debugName = fmt::format("{}_indexBuffer", meshPath) });
        m_device->writeBuffer(indexBufferHandle, indices.size() * sizeof(uint32_t), indices.data());

        // the VAO is tied to the mesh, soooo
        gpu::VertexAttributeDesc vDesc[] = {
            {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(render::PositionNormalTexcoordVertex, position), .elementStride = sizeof(render::PositionNormalTexcoordVertex)},
            {.name = "NORMAL", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(render::PositionNormalTexcoordVertex, normal), .elementStride = sizeof(render::PositionNormalTexcoordVertex)},
            {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 2, .offset = offsetof(render::PositionNormalTexcoordVertex, uv), .elementStride = sizeof(render::PositionNormalTexcoordVertex)}
        };

        gpu::InputLayoutHandle vertexLayoutHandle = m_device->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));

        render::Mesh outputMesh{
            .vertexBuffer = vertexBufferHandle,
            .indexBuffer = indexBufferHandle,
            .vertexLayout = vertexLayoutHandle,
            .triangleCount = indices.size() // There are 3 vertices per triangle, so divide by 3
        };

        m_device->debugMarkerPop();

        if (vertexBufferHandle.Get() != nullptr && indexBufferHandle.Get() != nullptr) {
            // Cache
            MeshTracker_t tracker = {
                .vertexBuffer = vertexBufferHandle,
                .indexBuffer = indexBufferHandle,
                .vertexLayout = vertexLayoutHandle,
                .mesh = outputMesh,
            };
            m_meshes.emplace(meshKey, tracker);
            return outputMesh;
        }
        else {
            LOG_WARNING("Failed to load mesh {}. Using error mesh...", meshPath);
            return m_errorMesh.mesh;
        }

        LOG_WARNING("Failed to load mesh {}. Using error mesh...", meshPath);
        return m_errorMesh.mesh;
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

        m_device->debugMarkerPush(fmt::format("Loading shader {} , {}...", params.vertShader, params.fragShader));

        auto shaderHandle = m_device->makeShader({
            .VS {.byteCode = (uint8_t*)vertContents.c_str(), .entryFunc = params.vertShaderEntryFunction },
            .PS {.byteCode = (uint8_t*)fragContents.c_str(), .entryFunc = params.fragShaderEntryFunction },
            .graphicsState = params.graphicsState,
            .debugName = params.debugName,
        });

        m_device->debugMarkerPop();

        if (shaderHandle.Get() != nullptr) {
            // Cache
            m_shaders.emplace(shaderKey, shaderHandle);
            return shaderHandle;
        } else {
            LOG_CRITICAL("Failed loading shader {} , {}!", params.vertShader, params.fragShader);
            return m_errorShader;
        }
    }
    gpu::ITexture* AssetManager::fetchTexture(const std::string& texturePath, const bool genMipmaps) {

        if (!m_intialisedDefaultAssets) {   
            initialiseErrorData();
        }

        // Check if the texture was loaded before
        if (m_textures.find(texturePath) != m_textures.end()) {
            return m_textures[texturePath];
        }

        int texWidth, texHeight, nrChannels;
        unsigned char* texData = stbi_load(fmt::format("{}/assets/textures/{}", m_applicationRootPath, texturePath).c_str(), &texWidth, &texHeight, &nrChannels, STBI_rgb_alpha);

        m_device->debugMarkerPush(fmt::format("Loading texture {}...", texturePath));
        gpu::TextureHandle textureHandle = m_device->makeTexture({
            .width = (uint32_t)texWidth,
            .height = (uint32_t)texHeight,
            .generateMipmaps = genMipmaps,
            .type = gpu::TextureType::Texture2D,
            .debugName = texturePath,
            }, texData);
        m_device->debugMarkerPop();

        if (textureHandle.Get() != nullptr) {
            // Cache
            m_textures.emplace(texturePath, textureHandle);
            return textureHandle;
        } else {
            return m_errorTexture;
        }
    }
}
