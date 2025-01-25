#include "breakanoid_layer.hpp"

#include <imgui.h>

void BreakanoidLayer::event(engine::events::Event& event) {
    engine::events::EventDispatcher dispatcher(event);
    dispatcher.dispatch<engine::events::WindowResizeEvent>(EVENT_BIND_FUNC(BreakanoidLayer::windowResized));
}

BreakanoidLayer::BreakanoidLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager)
    : ILayer(deviceManager, assetManager, "BreakanoidLayer") {

    getDevice()->setViewport({
        .left = 0,
        .right = engine::App::getInstance()->getWindow()->getWidth(),
        .top = 0,
        .bottom = engine::App::getInstance()->getWindow()->getHeight(),
        });

    getDevice()->clearColor({ 0, 0, 0, 1 });

    m_testMesh = getAssetManager()->fetchMesh("test.obj");

    m_shader = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
        },
        .vertShader = "vert.glsl",
        .fragShader = "frag.glsl",
        .debugName = "Simple"
        });
    getDevice()->setBufferBinding(m_shader, "DataBuffer", 0);

    // Prepare cbuffer to populate it with transform matrices
    m_cbufferData = {
        /*
        .model = hlslpp::float4x4::identity(),
        .view = hlslpp::float4x4::identity(),
        .projection = hlslpp::float4x4::perspective(
            hlslpp::projection(
                hlslpp::frustum(
                    App::getInstance()->getWindow()->getWidth(),
                    App::getInstance()->getWindow()->getHeight(),
                    0.01f,
                    1000.0f),
                hlslpp::zclip::t::minus_one)
        ),
        */
        .color = hlslpp::float4(0,0,.5f,1),
    };
    // Allocate buffer on the gpu
    m_cbuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic, .debugName = "Cbuffer" });
    getDevice()->writeBuffer(m_cbuffer, sizeof(m_cbufferData), nullptr); // Reserve size

    // Load texture

    // Define tri-linear aniso 16 texture sampler
    {
        m_texture = getAssetManager()->fetchTexture("brick_wall.png");

        m_trillinearAniso16ClampSampler = getDevice()->makeTextureSampler({ /* default (linear, wrap, 16x-aniso) */ });
    }

    auto fbo = getDevice()->makeFramebuffer({
        .colorDesc = {
            .width = engine::App::getInstance()->getWindow()->getWidth(),
            .height = engine::App::getInstance()->getWindow()->getHeight(),
            .samples = 4,
            .format = gpu::TextureFormat::RGB10_A2,
        },
        .depthStencilDesc = {
            .width = engine::App::getInstance()->getWindow()->getWidth(),
            .height = engine::App::getInstance()->getWindow()->getHeight(),
            .samples = 1,
            .format = gpu::TextureFormat::Depth24_Stencil8,
        },
        .hasDepth = true,
        .debugName = "FBO"
        });
}

BreakanoidLayer::~BreakanoidLayer() {
}

void BreakanoidLayer::update(double timeElapsed, double deltaTime) {
    // @TODO: Box2D state update
}

void BreakanoidLayer::render(double deltaTime) {

    getDevice()->clearColor({ 0, 0, 0, 1 });

    // Terrible test
    BreakanoidCBuffer* cbufferView = nullptr;
    getDevice()->mapBuffer(m_cbuffer, 0, sizeof(BreakanoidCBuffer), gpu::MapAccessFlags::Write, reinterpret_cast<void**>(&cbufferView));
    if (cbufferView != nullptr) {
        memcpy(cbufferView, &m_cbufferData, sizeof(BreakanoidCBuffer)); // copy data to gpu
        getDevice()->unmapBuffer(m_cbuffer);
    }
    getDevice()->setConstantBuffer(m_cbuffer, 0);

    // Bind texture with trillinearAniso16ClampSampler at slot 0
    getDevice()->bindTexture(m_texture, m_trillinearAniso16ClampSampler, 0);

    getDevice()->drawIndexed({
        .vertexBufer = m_testMesh.vertexBuffer,
        .indexBuffer = m_testMesh.indexBuffer,
        .shader = m_shader,
        .vertexLayout = m_testMesh.vertexLayout,
        }, m_testMesh.triangleCount);
}

bool BreakanoidLayer::windowResized(const engine::events::WindowResizeEvent& event) {

    getDevice()->setViewport({
        .left = 0,
        .right = event.width,
        .top = 0,
        .bottom = event.height,
        });

    // @TODO: Signal post-processing stack resize fbos

    return false;
}

void BreakanoidLayer::imguiDraw() {
    ImGui::ShowDemoWindow();

    ImGui::Begin("Debug");
    ImGui::ColorEdit3("triangleColour", m_cbufferData.color.f32,
        ImGuiColorEditFlags_PickerHueWheel | // colour wheel picker
        ImGuiColorEditFlags_Float | // display floats as f32 for easy copy paste when hardcoding values
        ImGuiColorEditFlags_DisplayRGB | // preview as RGB values
        ImGuiColorEditFlags_InputRGB // output in RGB
    );
    ImGui::End();
}