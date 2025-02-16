#include "text_renderer.hpp"
#include "engine/log.hpp"
#include "engine/app.hpp"
#include "engine/renderer/camera.hpp"
#include <algorithm>

namespace render {

    FontRenderer::FontRenderer() {
        m_executableDir = engine::App::getInstance()->getAssetManager()->getExecutableDir();
    }

    void FontRenderer::init(gpu::IDevice* device) {
        ASSERT(device != nullptr);
        m_pDevice = device;

        m_trillinearAniso16ClampSampler = m_pDevice->makeTextureSampler({ /* default (linear, wrap, 16x-aniso) */ });

        m_textShader = engine::App::getInstance()->getAssetManager()->fetchShader({
            .graphicsState = {
                .depthWrite = false,
                .depthTest = false,
                .faceCullingMode = gpu::FaceCullMode::Never,
            },
            .vertShader = "text_shader_vert.glsl",
            .fragShader = "text_shader_frag.glsl",
            .debugName = "TextShader",
            });
        m_pDevice->setBufferBinding(m_textShader, "TextBuffer", 0);

        // prepare text buffer for rendering
        m_vertices.resize(3*5000);
        m_textVertexBuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Dynamic, .debugName = "FontRenderer_vertexBuffer" });
        m_pDevice->writeBuffer(m_textVertexBuffer, m_vertices.size() * sizeof(TextVertex), m_vertices.data());
        m_vertices.clear();

        gpu::VertexAttributeDesc vDesc[] = {
            {.name = "POSITION", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 0, .offset = offsetof(TextVertex, pos), .elementStride = sizeof(TextVertex)},
            {.name = "TEXCOORD0", .format = gpu::GpuFormat::RG8_TYPELESS, .bufferIndex = 1, .offset = offsetof(TextVertex, uv), .elementStride = sizeof(TextVertex)}
        };

        m_textVertexLayout = m_pDevice->createInputLayout(vDesc, sizeof(vDesc) / sizeof(vDesc[0]));

        m_textCBuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic, .debugName = "TextCBuffer" });
        m_pDevice->writeBuffer(m_textCBuffer, sizeof(TextCBuffer), nullptr);
    }


    // to handle malformed csv
    bool tryParseInt(const char* str, int line_number, uint32_t* result) {
        char* endptr;
        errno = 0;
        *result = strtol(str, &endptr, 10); // base 10 string input
        if (errno != 0 || *endptr != '\0') {
            LOG_WARN("[Font]: Expected <int> on line {}, but got {}! Ignoring...", line_number, str);
            return false;
        }
        return true;
    }

    bool tryParseFloat(const char* str, int line_number, float* result) {
        char* endptr;
        errno = 0;
        *result = (float) strtod(str, &endptr);
        if (errno != 0 || *endptr != '\0') {
            LOG_WARN("[Font]: Expected <double> on line {}, but got {}! Ignoring...", line_number, str);
            return false;
        }
        return true;
    }

    FontData FontRenderer::loadFont(const std::string& filePath, gpu::ITexture* texture) {
        ASSERT(texture != nullptr);

        FontData fontData = {};
        fontData.isValid = false;

        FILE* file = fopen(fmt::format("{}/assets/{}", m_executableDir, filePath).c_str(), "r");
        if (file == nullptr) {
            LOG_ERROR("[Font]: File {} could not be opened.", filePath);
            return {};
        }

        // data format is GlyphInfo in order

        char line[2048] = {};
        int line_number = 0;

        fontData.firstCodePoint = 0xFFFFFFFF;

        while (fgets(line, sizeof(line), file) != nullptr) {
            line_number++;

            GlyphInfo glyphInfo = { 0 };
            char* token = strtok(line, ",\n");
            int index = 0;

            while (token != nullptr) {
                switch (index) {
                case 0: if (!tryParseInt(token, line_number, &glyphInfo.unicodeCodePoint)) { token = nullptr; break; } break;
                    case 1: if (!tryParseFloat(token, line_number, &glyphInfo.horizAdvanceEm)) { token = nullptr; break; } break;
                    case 2: if (!tryParseFloat(token, line_number, &glyphInfo.quadBounds.x)) { token = nullptr; break; } break; // left
                    case 3: if (!tryParseFloat(token, line_number, &glyphInfo.quadBounds.y)) { token = nullptr; break; } break; // bottom
                    case 4: if (!tryParseFloat(token, line_number, &glyphInfo.quadBounds.z)) { token = nullptr; break; } break; // right
                    case 5: if (!tryParseFloat(token, line_number, &glyphInfo.quadBounds.w)) { token = nullptr; break; } break; // top
                    case 6: if (!tryParseFloat(token, line_number, &glyphInfo.pixelBounds.x)) { token = nullptr; break; } break; // left
                    case 7: if (!tryParseFloat(token, line_number, &glyphInfo.pixelBounds.y)) { token = nullptr; break; } break; // bottom
                    case 8: if (!tryParseFloat(token, line_number, &glyphInfo.pixelBounds.z)) { token = nullptr; break; } break; // right
                    case 9: if (!tryParseFloat(token, line_number, &glyphInfo.pixelBounds.w)) { token = nullptr; break; } break; // top
                }

                token = strtok(nullptr, ",\n");
                index++;
            }

            // only accept the parsed data if there were at least 10 entries in this line
            if (index == 10) {
                fontData.glyphs.emplace(glyphInfo.unicodeCodePoint, glyphInfo);
                fontData.firstCodePoint = std::min(fontData.firstCodePoint, glyphInfo.unicodeCodePoint);
            } else {
                LOG_WARN("[Font]: Invalid line {}: {}", line_number, line);
            }
        }

        fontData.texture = texture;
        fontData.isValid = true;

        fclose(file);

        return fontData;
    }

    void FontRenderer::drawText(const FontData& fontData, const TextDrawParams params, render::Entity* pEntity, render::Camera* pCameraComponent) {

        ASSERT(pEntity != nullptr);
        ASSERT(pCameraComponent != nullptr);

        // construct vertex buffer of text to issue draw call with
        auto textureDesc = fontData.texture->getDesc();
        float texWidth = (float)textureDesc.width;
        float texHeight = (float)textureDesc.height;

        float scale = fontData.fontSize * params.size;

        // current "caret" pos
        float advanceX = params.posX * scale;
        float advanceY = params.posY * scale;

        for (int i = 0; i < params.text.length(); i++) {
            const char c = params.text.at(i);

            // newline = move to initial pos + add line height
            if (c == '\n') {
                advanceX = params.posX * scale;
                advanceY += params.lineHeight * scale;
                continue;
            }

            const auto& currGlyph = fontData.glyphs.at(c);
            hlslpp::float4 pixelBounds = currGlyph.pixelBounds;
            hlslpp::float4 quadBounds = currGlyph.quadBounds * scale;

            quadBounds.x += advanceX;
            quadBounds.z += advanceX;
            quadBounds.y -= advanceY;
            quadBounds.w -= advanceY;

            // build quad into VBO
            m_vertices.insert(m_vertices.end(), {
                {quadBounds.x, quadBounds.y, pixelBounds.x / texWidth, pixelBounds.y / texHeight},
                {quadBounds.x, quadBounds.w, pixelBounds.x / texWidth, pixelBounds.w / texHeight},
                {quadBounds.z, quadBounds.y, pixelBounds.z / texWidth, pixelBounds.y / texHeight},

                {quadBounds.z, quadBounds.y, pixelBounds.z / texWidth, pixelBounds.y / texHeight},
                {quadBounds.x, quadBounds.w, pixelBounds.x / texWidth, pixelBounds.w / texHeight},
                {quadBounds.z, quadBounds.w, pixelBounds.z / texWidth, pixelBounds.w / texHeight},
            });

            advanceX += currGlyph.horizAdvanceEm * scale;
        }

        // upload to gpu
        m_pDevice->writeBuffer(m_textVertexBuffer, m_vertices.size() * sizeof(TextVertex), m_vertices.data());

        // bind texture to gpu
        m_pDevice->bindTexture(fontData.texture, m_trillinearAniso16ClampSampler, 0);

        // Set text cbuffer on bind slot 0
        m_pDevice->setConstantBuffer(m_textCBuffer, 0);
        TextCBuffer* textBufferView = nullptr;
        m_pDevice->mapBuffer(m_textCBuffer, 0, sizeof(TextCBuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&textBufferView));
        if (textBufferView != nullptr) {
            textBufferView->model = pEntity->transform.getModel();
            textBufferView->view = pCameraComponent->getViewMatrix();
            textBufferView->projection = hlslpp::float4x4::orthographic(hlslpp::projection(hlslpp::frustum(
                /* width */ engine::App::getInstance()->getWindow()->getWidth(),
                /* height */ engine::App::getInstance()->getWindow()->getHeight(),
                /* near_z */ 0.1f,
                /* far_z */ 100.0f),
                hlslpp::zclip::minus_one, hlslpp::zdirection::reverse, hlslpp::zplane::infinite));
            textBufferView->colourForeground = params.colourForeground;
            textBufferView->colourOutline = params.colourOutline;
            textBufferView->outlineWidth = params.outlineWidth;

            m_pDevice->unmapBuffer(m_textCBuffer);
        }

        uint32_t triCount = m_vertices.size() / 3;

        m_pDevice->draw({
            .vertexBufer = m_textVertexBuffer,
            .shader = m_textShader,
            .vertexLayout = m_textVertexLayout
            }, triCount);

        m_vertices.clear();
    }
}