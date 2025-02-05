#include "game_layer.hpp"

#include <imgui.h>

// Test-bed for the engine features without breaking the game

void GameLayer::event(engine::events::Event& event) {
    engine::events::EventDispatcher dispatcher(event);
    dispatcher.dispatch<engine::events::WindowResizeEvent>(EVENT_BIND_FUNC(GameLayer::windowResized));
}

/*
render::PositionColorVertex vertices[] = {
    { .position = { -1, -1, 0 }, .color = { 1, 0, 0 }, .uv = { 0, 0 } },
    { .position = {  1, -1, 0 }, .color = { 0, 1, 0 }, .uv = { 1, 0 } },
    { .position = {  1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 1, 1 } },
    { .position = { -1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 0, 1 } },
};
uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };
*/

GameLayer::GameLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager)
    : ILayer(deviceManager, assetManager, "GameLayer") {

    getDevice()->setViewport({
        .left = 0,
        .right = engine::App::getInstance()->getWindow()->getWidth(),
        .top = 0,
        .bottom = engine::App::getInstance()->getWindow()->getHeight(),
        });

    getDevice()->clearColor({ 0, 0, 0, 1 });

    /*
    // Vertex buffer
    m_vertexBuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Default });
    getDevice()->writeBuffer(m_vertexBuffer, sizeof(vertices), vertices);

    // Triangle indices
    m_indexBuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::IndexBuffer, .usage = gpu::Usage::Default, .format = gpu::GpuFormat::Uint16_TYPELESS });
    getDevice()->writeBuffer(m_indexBuffer, sizeof(indices), indices);

    // VertexLayout
    gpu::VertexAttributeDesc vDesc[] = {
        {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(render::PositionColorVertex, position), .elementStride = sizeof(render::PositionColorVertex)},
        {.name = "COLOR", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(render::PositionColorVertex, color), .elementStride = sizeof(render::PositionColorVertex)},
        {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 2, .offset = offsetof(render::PositionColorVertex, uv), .elementStride = sizeof(render::PositionColorVertex)}
    };

    m_vertexLayout = getDevice()->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));
    */

    m_testMesh = getAssetManager()->fetchMesh("test.obj");

    m_shader = getAssetManager()->fetchShader({
        .graphicsState = {
            // opaque rendering
        },
        .vertShader = "vert.glsl",
        .fragShader = "frag.glsl",
        .debugName = "Simple"
    });
    // getDevice()->setBufferBinding(m_shader, "DataBuffer", 0);

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

GameLayer::~GameLayer() {
}

void GameLayer::update(double timeElapsed, double deltaTime) {
    // @TODO: Box2D state update
    if (engine::input::InputManager::getInstance()->keyReleased(engine::input::Keycode::W)) {
        LOG_INFO("W!");
    }
}

void GameLayer::render(double deltaTime) {

    getDevice()->clearColor({ 0, 0, 0, 1 });

    // Terrible test
    CBuffer* cbufferView = nullptr;
    getDevice()->mapBuffer(m_cbuffer, 0, sizeof(CBuffer), gpu::MapAccessFlags::Write, reinterpret_cast<void**>(&cbufferView));
    if (cbufferView != nullptr) {
        // cbufferView->color = hlslpp::float4(1, deltaTime, 0.5f, 1);
        memcpy(cbufferView, &m_cbufferData, sizeof(CBuffer)); // copy data to gpu
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

bool GameLayer::windowResized(const engine::events::WindowResizeEvent& event) {

    getDevice()->setViewport({
        .left = 0,
        .right = event.width,
        .top = 0,
        .bottom = event.height,
        });

    // @TODO: Signal post-processing stack resize fbos

    return false;
}

void GameLayer::imguiDraw() {
    ImGui::ShowDemoWindow();

    ImGui::Begin("Debug");
    ImGui::ColorEdit3("triangleColour", m_cbufferData.color.f32,
        ImGuiColorEditFlags_PickerHueWheel | // colour wheel picker
        ImGuiColorEditFlags_Float | // display floats as f32 for easy copy paste when hardcoding values
        ImGuiColorEditFlags_DisplayRGB | // preview as RGB values
        ImGuiColorEditFlags_InputRGB // output in RGB
    );
    ImGui::SliderFloat("colorBlendFac", &m_cbufferData.colorBlendFac, 0, 1, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::End();
}