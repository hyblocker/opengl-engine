#include "game_layer.hpp"
#include <stb_image.h>

struct PositionColorVertex {
    float position[3];
    float color[3];
    float uv[2];
};

#define SHADER_HEADER "#version 460 core"

constexpr const char shader_vert[] = SHADER_HEADER R"(
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iColor;
layout (location = 2) in vec2 iUv;

layout(std140) uniform DataBuffer
{
    vec4 coolColor;
};

out vec3 color;
out vec2 uv;

void main()
{
    gl_Position = vec4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    color = iColor * coolColor.rgb;
    uv = iUv.xy;
}
)";
constexpr const char shader_pixel[] = SHADER_HEADER R"(
precision mediump float;

// gl_FragColor is deprecated in GLSL 4.4+
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D brickTex;

in vec3 color;
in vec2 uv;

void main()
{
    fragColor = vec4(mix(color, texture(brickTex, uv).rgb, 0.5f), 1.0f);
} 
)";

PositionColorVertex vertices[] = {
    { .position = { -1, -1, 0 }, .color = { 1, 0, 0 }, .uv = { 0, 0 } },
    { .position = {  1, -1, 0 }, .color = { 0, 1, 0 }, .uv = { 1, 0 } },
    { .position = {  1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 1, 1 } },
    { .position = { -1,  1, 0 }, .color = { 0, 0, 1 }, .uv = { 0, 1 } },
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
            .depthState = gpu::CompareFunc::LessOrEqual, // inverse Z
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

    // Define tri-linear aniso 16 texture sampler
    {
        int texWidth, texHeight, nrChannels;
        unsigned char* texData = stbi_load("assets/textures/brick_wall.png", &texWidth, &texHeight, &nrChannels, 0);

        m_texture = getDevice()->makeTexture({
            .width = (uint32_t) texWidth,
            .height = (uint32_t) texHeight,
            .type = gpu::TextureType::Texture2D,
            }, texData);

        m_trillinearAniso16ClampSampler = getDevice()->makeTextureSampler({
            .minFilter = gpu::SamplingMode::Linear,
            .magFilter = gpu::SamplingMode::Linear,
            .mipFilter = gpu::SamplingMode::Linear, // Trilinear sampling, Nearest = Bilinear sampling

            .wrapX = gpu::TextureWrap::Repeat,
            .wrapY = gpu::TextureWrap::Repeat,
            .wrapZ = gpu::TextureWrap::Repeat,

            // do not bias mip-map sampling
            .lodBias = 0,

            // 16x anisotropic filtering
            .anisotropy = 16.0f,
        });
    }

    auto fbo = getDevice()->makeFramebuffer({
        .colorDesc = {
            .width = App::getInstance()->getWindow()->getWidth(),
            .height = App::getInstance()->getWindow()->getHeight(),
            .samples = 4,
            .format = gpu::TextureFormat::RGB10_A2,
        },
        .depthStencilDesc = {
            .width = App::getInstance()->getWindow()->getWidth(),
            .height = App::getInstance()->getWindow()->getHeight(),
            .samples = 1,
            .format = gpu::TextureFormat::Depth24_Stencil8,
        },
        .hasDepth = true
    });
}

GameLayer::~GameLayer() {
}

void GameLayer::update(double timeElapsed, double deltaTime) {
    // @TODO: Box2D state update
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

    // Bind texture with trillinearAniso16ClampSampler at slot 0
    getDevice()->bindTexture(m_texture, m_trillinearAniso16ClampSampler, 0);

    getDevice()->drawIndexed({
        .vertexBufer = m_vertexBuffer,
        .indexBuffer = m_indexBuffer,
        .shader = m_shader,
        }, (sizeof(indices) / sizeof(indices[0])));

    // Present
    getDevice()->present();
}

void GameLayer::backBufferResizing() {
    // @TODO: invalidate post processing textures
}

void GameLayer::backBufferResized(uint32_t width, uint32_t height, uint32_t samples) {

    // Set viewport dimensions
    getDevice()->setViewport({
        .left = 0,
        .right = width,
        .top = 0,
        .bottom = height,
        });
}