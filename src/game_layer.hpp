#include <stdio.h>

#include "engine/app.hpp"
#include <hlsl++.h>

struct CBuffer {
    // hlslpp::float4x4 model;
    // hlslpp::float4x4 view;
    // hlslpp::float4x4 projection;

    hlslpp::float4   color = { 0.0f,1.0f,1.0f,1.0f };
    float colorBlendFac = 1.0f;
};

class GameLayer : public engine::ILayer {
public:
    GameLayer(gpu::DeviceManager* deviceManager);
    ~GameLayer() override;

    void update(double timeElapsed, double deltaTime) override;
    void render(double deltaTime) override;
    void event(engine::events::Event& event) override;
    void imguiDraw() override;

private:
    bool windowResized(const engine::events::WindowResizeEvent& event);

private:

    CBuffer m_cbufferData;

    gpu::BufferHandle m_vertexBuffer;
    gpu::BufferHandle m_indexBuffer;
    gpu::InputLayoutHandle m_vertexLayout;
    gpu::ShaderHandle m_shader;

    gpu::BufferHandle m_cbuffer;

    gpu::TextureHandle m_texture;
    gpu::TextureSamplerHandle m_trillinearAniso16ClampSampler;
};