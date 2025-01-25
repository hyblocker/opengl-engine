#include <stdio.h>

#include "engine/app.hpp"
#include "engine/renderer/scene_renderer.hpp"
#include <hlsl++.h>

struct BreakanoidCBuffer {
    // hlslpp::float4x4 model;
    // hlslpp::float4x4 view;
    // hlslpp::float4x4 projection;

    hlslpp::float4   color = { 0.0f,1.0f,1.0f,1.0f };
};

class BreakanoidLayer : public engine::ILayer {
public:
    BreakanoidLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager);
    ~BreakanoidLayer() override;

    void update(double timeElapsed, double deltaTime) override;
    void render(double deltaTime) override;
    void event(engine::events::Event& event) override;
    void imguiDraw() override;

private:
    bool windowResized(const engine::events::WindowResizeEvent& event);

private:

    gpu::TextureSamplerHandle m_trillinearAniso16ClampSampler;
    
    BreakanoidCBuffer m_cbufferData;

    render::Mesh m_testMesh;

    gpu::IShader* m_shader;

    gpu::BufferHandle m_cbuffer;

    gpu::ITexture* m_texture;

    render::Scene m_menuScene;
    render::Scene m_gameScene;
};