#pragma once

#include "scene_graph.hpp"
#include "engine/gpu/idevice.hpp"
#include "engine/managers/asset_manager.hpp"

namespace render {

    class Light;

    struct GeometryCBuffer {
        hlslpp::float4x4 model;
        hlslpp::float4x4 view;
        hlslpp::float4x4 projection;
        hlslpp::float3 cameraPos;
    };

    struct MaterialCBuffer {
        hlslpp::float3 ambient = { 0.2, 0.2, 0.2 };
        hlslpp::float3 diffuse = { 1, 1, 1 };
        hlslpp::float3 specular = { 1, 1, 1 };
        hlslpp::float3 emissionColour = { 0, 0, 0 };
        float emissionIntensity = 1.0f;
    };

    class SceneRenderer {
    public:
        void init(gpu::IDevice* pDevice, managers::AssetManager* pAssetManager);
        void draw(Scene& scene, const float aspect);
    private:
        void buildForwardRenderGraph(Entity* entity);
        void drawRenderList(std::vector<MeshRenderer*>& drawables, Camera* cameraComponent);
        void drawSkybox(Scene& scene, Camera* camera);

        gpu::IDevice* m_pDevice = nullptr;
        managers::AssetManager* m_pAssetManager = nullptr;

        gpu::TextureSamplerHandle m_trillinearAniso16ClampSampler;

        gpu::BufferHandle m_geometryCbuffer;
        gpu::BufferHandle m_materialCbuffer;

        gpu::IShader* m_skyboxTexShader;
        gpu::IShader* m_skyboxProceduralShader;
        Mesh m_skyboxQuad;

        gpu::BlendStateHandle m_opaque_BlendState;
        gpu::BlendStateHandle m_alphaBlend_BlendState;

        std::vector<Light*> m_lights;
        std::vector<MeshRenderer*> m_forwardOpaqueList;
        std::vector<MeshRenderer*> m_forwardTransparentList;
    };
}