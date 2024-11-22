#include "game_layer.hpp"

struct PositionColorVertex {
    float position[3];
    float color[3];
    float uv[2];
};

#define SHADER_HEADER "#version 330 core"

constexpr const char shader_vert[] = SHADER_HEADER R"(
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColor;
layout (location = 2) in vec2 iUv;

layout(std140) uniform DataBuffer
{
    vec4 coolColor;
};

out vec3 color;

void main()
{
    gl_Position = vec4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    color = iColor * coolColor.rgb;
}
)";
constexpr const char shader_pixel[] = SHADER_HEADER R"(
precision mediump float;

in vec3 color;

void main()
{
    gl_FragColor = vec4(color, 1.0f);
} 
)";

PositionColorVertex vertices[] = {
    { .position = { -1, -1, 0 }, .color = { 1, 0, 0 }, .uv = { 0, 0, } },
    { .position = {  1, -1, 0 }, .color = { 0, 1, 0 }, .uv = { 1, 0, } },
    { .position = {  1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 1, 1, } },
    { .position = { -1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 0, 1, } },
};
uint16_t indices[] = { 0, 1, 2, 2, 3, 0 };

GameLayer::GameLayer(gpu::DeviceManager* deviceManager)
    : ILayer(deviceManager) {

    getDevice()->setViewport({
        .left = 0,
        .right = App::getInstance()->getWindow()->getWidth(),
        .top = 0,
        .bottom = App::getInstance()->getWindow()->getHeight(),
        });

    getDevice()->clearColor({ 0, 0, 0, 1 });

    // Vertex buffer
    m_vertexBuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Default });
    getDevice()->writeBuffer(m_vertexBuffer, sizeof(vertices), vertices);

    // Triangle indices (connectivity)
    m_indexBuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::IndexBuffer, .usage = gpu::Usage::Default, .format = gpu::GpuFormat::Uint16_TYPELESS });
    getDevice()->writeBuffer(m_indexBuffer, sizeof(indices), indices);

    // VertexLayout
    gpu::VertexAttributeDesc vDesc[] = {
        {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(PositionColorVertex, position), .elementStride = sizeof(PositionColorVertex)},
        {.name = "COLOR", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(PositionColorVertex, color), .elementStride = sizeof(PositionColorVertex)},
        {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 2, .offset = offsetof(PositionColorVertex, uv), .elementStride = sizeof(PositionColorVertex)}
    };
    m_vertexLayout = getDevice()->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));

    // Make a shader but awesome
    m_shader = getDevice()->makeShader({
        .debugName = "triangle",
        .VS { .byteCode = (uint8_t*) shader_vert, .entryFunc = "main" },
        .PS { .byteCode = (uint8_t*) shader_pixel, .entryFunc = "main" },
        .graphicsState = {
            .depthState = gpu::COMPARE::GREATER_OR_EQUAL, // inverse Z
            .vertexLayout = m_vertexLayout,
    }   });
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
    m_cbuffer = getDevice()->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic });
    getDevice()->writeBuffer(m_cbuffer, sizeof(m_cbufferData), nullptr); // Reserve size

    // Load texture

    // Define trilinnear aniso 16 texture sampler
    m_texture = getDevice()->makeTextureSampler({});
}

GameLayer::~GameLayer() {
}

void GameLayer::update(double timeElapsed, double deltaTime) {

}

void GameLayer::render(double deltaTime) {

    getDevice()->clearColor({ 0, 0, 0, 1 });

    // Terrible test
    CBuffer* cbufferView = nullptr;
    getDevice()->mapBuffer(m_cbuffer, 0, sizeof(CBuffer), gpu::MapAccessFlags::Write, reinterpret_cast<void**>(&cbufferView));
    if (cbufferView != nullptr) {
        cbufferView->color = hlslpp::float4(1, deltaTime, 0.5f, 1);
        getDevice()->unmapBuffer(m_cbuffer);
    }
    getDevice()->setConstantBuffer(m_cbuffer, 0);

    getDevice()->drawIndexed({
        .vertexBufer = m_vertexBuffer,
        .indexBuffer = m_indexBuffer,
        .shader = m_shader,
        }, (sizeof(indices) / sizeof(indices[0])));

    // Present
    getDevice()->present();
}

void GameLayer::backBufferResizing() {

}

void GameLayer::backBufferResized(uint32_t width, uint32_t height, uint32_t samples) {

}