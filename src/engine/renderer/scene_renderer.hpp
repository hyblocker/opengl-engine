#pragma once

#include "scene_graph.hpp"
#include "engine/gpu/idevice.hpp"
#include "engine/managers/asset_manager.hpp"
#include "particle_system.hpp"
#include "text_renderer.hpp"
#include "ui_components.hpp"

namespace render {

    class Light;

    constexpr uint32_t k_MAX_LIGHTS = 4;
    constexpr uint32_t k_MAX_PARTICLES = 800;

    struct GeometryCBuffer {
        hlslpp::float4x4 model;
        hlslpp::float4x4 view;
        hlslpp::float4x4 projection;
        hlslpp::float4 cameraPosTime;
    };

    struct MaterialCBuffer {
        hlslpp::float3 ambient = { 0.2, 0.2, 0.2 };
        hlslpp::float3 diffuse = { 1, 1, 1 };
        hlslpp::float3 specular = { 1, 1, 1 };
        hlslpp::float4 emissionColour_glintFactor = { 0, 0, 0, 0 };
        float roughness = 1;
        float metallic = 0;
        float emissionIntensity = 1.0f;
    };

    struct LightRenderData {
        // These 4 floats would be aligned into a float4, meaning a single light occupies 16 bytes
        uint32_t type;
        float intensity;
        float innerRadius;
        float outerRadius;

        // Ignored for dir lights
        hlslpp::float3 position;
        hlslpp::float3 direction;

        // RGB colour
        hlslpp::float3 colour;
    };

    struct LightsCbuffer {
        LightRenderData light[k_MAX_LIGHTS];
    };

    struct RenderParticleElement {
        hlslpp::float3 position;
        hlslpp::float3 velocity;
        hlslpp::float4 colourBegin;
        hlslpp::float4 colourEnd;

        // packed as float4
        float sizeBegin;
        float sizeEnd;
        float life;
        float particleTextureCount;
    };

    struct ParticlesCBuffer {
        RenderParticleElement particles[k_MAX_PARTICLES];
    };

    struct UiCBuffer {
        hlslpp::float4x4 model;
        hlslpp::float4x4 view;
        hlslpp::float4x4 projection;
        hlslpp::float4 textureTint;
        hlslpp::float4 sizePosition;
        hlslpp::float4 screenSize;
    };

    class SceneRenderer {
    public:
        void init(gpu::IDevice* pDevice, managers::AssetManager* pAssetManager);
        void draw(Scene& scene, const float aspect, float deltaTime);
    private:

        struct RenderListElement {
            ComponentType componentType;
            union {
                MeshRenderer* pMeshRenderer;
                ParticleSystem* pParticleSystem;
                UIElement* pUiElement;
            };
            hlslpp::float4x4 parentMatrix;
        };

        void buildForwardRenderGraph(Entity* entity, hlslpp::float4x4 parentMatrix);
        void buildUiRenderGraph(Entity* entity);
        void drawRenderList(std::vector<RenderListElement>& drawables, Camera* cameraComponent, Light* sunLight, gpu::IBlendState* blendState);
        void drawSkybox(Scene& scene, Camera* camera, Light* sunLight);

        gpu::IDevice* m_pDevice = nullptr;
        managers::AssetManager* m_pAssetManager = nullptr;

        gpu::TextureSamplerHandle m_trillinearAniso16ClampSampler;

        gpu::BufferHandle m_geometryCbuffer;
        gpu::BufferHandle m_materialCbuffer;
        gpu::BufferHandle m_lightsCbuffer;
        gpu::BufferHandle m_particlesCbuffer;
        gpu::BufferHandle m_UiCbuffer;

        gpu::IShader* m_skyboxTexShader = nullptr;
        gpu::IShader* m_skyboxProceduralShader = nullptr;
        gpu::IShader* m_uiShader = nullptr;
        Mesh m_skyboxSphere;
        Mesh m_particleQuad;

        gpu::BlendStateHandle m_opaque_BlendState;
        gpu::BlendStateHandle m_alphaBlend_BlendState;

        std::vector<Light*> m_lights;
        std::vector<RenderListElement> m_forwardOpaqueList;
        std::vector<RenderListElement> m_forwardTransparentList;
        std::vector<RenderListElement> m_uiRenderList;

        float m_elapsedTime = 0;

        FontRenderer m_fontRenderer;
        FontData m_fontData;
    };
}