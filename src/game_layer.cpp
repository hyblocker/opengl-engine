#include "game_layer.hpp"

struct PositionColorVertex {
    float position[3];
    float color[3];
};

#define SHADER_HEADER "#version 330 core"

constexpr const char shader_vert[] = SHADER_HEADER R"(
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColor;

out vec3 color;

void main()
{
    gl_Position = vec4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    color = iColor;
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
    { .position = { -1, -1, 0 }, .color = { 1, 0, 0 } },
    { .position = {  1, -1, 0 }, .color = { 0, 1, 0 } },
    { .position = {  1,  1, 0 }, .color = { 0, 0, 1 } },
    { .position = { -1,  1, 0 }, .color = { 0, 0, 1 } },
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
    gpu::VertexAttributeDesc vDesc[2] = {
        {.name = "POSITION", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 0, .offset = offsetof(PositionColorVertex, position), .elementStride = sizeof(PositionColorVertex)},
        {.name = "COLOR", .format = gpu::GpuFormat::RGB8_TYPELESS, .bufferIndex = 1, .offset = offsetof(PositionColorVertex, color), .elementStride = sizeof(PositionColorVertex)}
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

    // Prepare cbuffer to populate it with transform matrices
    m_cbufferData = {
        // .model = hlslpp::float4x4::look_at(pos, target, up) :
    };
}

GameLayer::~GameLayer() {
}

void GameLayer::update(double timeElapsed, double deltaTime) {

}

void GameLayer::render(double deltaTime) {

    getDevice()->clearColor({ 0, 0, 0, 1 });

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