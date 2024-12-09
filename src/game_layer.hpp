#include <stdio.h>

#include "engine/app.hpp"
#include <hlsl++.h>

struct CBuffer {
    // hlslpp::float4x4 model;
    // hlslpp::float4x4 view;
    // hlslpp::float4x4 projection;

    hlslpp::float4   color;
};

class GameLayer : public engine::ILayer {
public:
    GameLayer(gpu::DeviceManager* deviceManager);
    ~GameLayer() override;

    void update(double timeElapsed, double deltaTime) override;
    void render(double deltaTime) override;
    void backBufferResizing() override;
    void backBufferResized(uint32_t width, uint32_t height, uint32_t samples) override;

private:

    CBuffer m_cbufferData;

    gpu::BufferHandle m_vertexBuffer;
    gpu::BufferHandle m_indexBuffer;
    gpu::InputLayoutHandle m_vertexLayout;
    gpu::ShaderHandle m_shader;

    gpu::BufferHandle m_cbuffer;

    gpu::TextureHandle m_texture;
    gpu::TextureSamplerHandle m_trillinearClampSampler;
};