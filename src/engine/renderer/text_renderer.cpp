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
            },
            .vertShader = "text_shader_vert.glsl",
            .fragShader = "text_shader_frag.glsl",
            .debugName = "TextShader",
            });
        m_pDevice->setBufferBinding(m_textShader, "TextBuffer", 0);

        // prepare text buffer for rendering
        m_vertices.resize(3);
        m_textVertexBuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::VertexBuffer, .usage = gpu::Usage::Dynamic, .debugName = "FontRenderer_vertexBuffer" });
        m_pDevice->writeBuffer(m_textVertexBuffer, m_vertices.size() * sizeof(TextVertex), m_vertices.data());

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

    bool tryParseDouble(const char* str, int line_number, double* result) {
        char* endptr;
        errno = 0;
        *result = strtod(str, &endptr);
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

        while (fgets(line, sizeof(line), file) != nullptr) {
            line_number++;

            GlyphInfo glyphInfo = { 0 };
            char* token = strtok(line, ",");
            int index = 0;

            while (token != nullptr) {
                switch (index) {
                    case 0: if (!tryParseInt(token, line_number, &glyphInfo.unicodeCodePoint)) break;
                    case 1: if (!tryParseDouble(token, line_number, &glyphInfo.horizAdvanceEm)) break;
                    case 2: if (!tryParseDouble(token, line_number, &glyphInfo.quadBoundLeft)) break;
                    case 3: if (!tryParseDouble(token, line_number, &glyphInfo.quadBoundBottom)) break;
                    case 4: if (!tryParseDouble(token, line_number, &glyphInfo.quadBoundRight)) break;
                    case 5: if (!tryParseDouble(token, line_number, &glyphInfo.quadBoundTop)) break;
                    case 6: if (!tryParseDouble(token, line_number, &glyphInfo.pixelBoundLeft)) break;
                    case 7: if (!tryParseDouble(token, line_number, &glyphInfo.pixelBoundBottom)) break;
                    case 8: if (!tryParseDouble(token, line_number, &glyphInfo.pixelBoundRight)) break;
                    case 9: if (!tryParseDouble(token, line_number, &glyphInfo.pixelBoundTop)) break;
                }

                token = strtok(nullptr, ",");
                index++;
            }

            // only accept the parsed data if there were at least 10 entries in this line
            if (index == 10) {
                fontData.glyphs.push_back(glyphInfo);
            } else {
                LOG_WARN("[Font]: Invalid line {}: {}", line_number, line);
            }
        }

        // sort by unicode code point, so that we can do indexing nicely
        std::sort(fontData.glyphs.begin(), fontData.glyphs.end(), [](const GlyphInfo& a, const GlyphInfo& b) {
            return a.unicodeCodePoint < b.unicodeCodePoint;
        });

        fontData.firstCodePoint = fontData.glyphs.front().unicodeCodePoint;
        fontData.texture = texture;
        fontData.isValid = true;

        fclose(file);

        return fontData;
    }

    void FontRenderer::drawText(const FontData& fontData, const TextDrawParams params, render::Entity* pEntity, render::Camera* pCameraComponent) {

        ASSERT(pEntity != nullptr);
        ASSERT(pCameraComponent != nullptr);

        // bind texture to gpu
        m_pDevice->bindTexture(fontData.texture, m_trillinearAniso16ClampSampler, 0);

        // Set text cbuffer on bind slot 0
        m_pDevice->setConstantBuffer(m_textCBuffer, 0);
        TextCBuffer* textBufferView = nullptr;
        m_pDevice->mapBuffer(m_textCBuffer, 0, sizeof(TextCBuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&textBufferView));
        if (textBufferView != nullptr) {
            textBufferView->model = pEntity->transform.getModel();
            textBufferView->view = pCameraComponent->getViewMatrix();
            textBufferView->projection = pCameraComponent->getProjectionMatrix();
            textBufferView->colourForeground = params.colourForeground;
            textBufferView->colourOutline = params.colourOutline;
            textBufferView->outlineWidth = params.outlineWidth;

            m_pDevice->unmapBuffer(m_textCBuffer);
        }

        // construct vertex buffer of text to issue draw call with
        auto textureDesc = fontData.texture->getDesc();
        float texWidth = (float)textureDesc.width;
        float texHeight = (float)textureDesc.height;

        // since this is only for text we'll just store raw floats
        uint32_t currentX = params.posX;

        for (const char& c : params.text) {
            bool glyphFound = false;
            for (const auto& glyph : fontData.glyphs) {
                if (glyph.unicodeCodePoint == (uint32_t)c) {
                    glyphFound = true;

                    float xpos = currentX + glyph.pixelBoundLeft; // Use currentX
                    float ypos = params.posY - (glyph.pixelBoundTop - glyph.pixelBoundBottom);
                    float w = glyph.pixelBoundRight - glyph.pixelBoundLeft;
                    float h = glyph.pixelBoundTop - glyph.pixelBoundBottom;

                    // Calculate UV coordinates
                    float u0 = glyph.pixelBoundLeft / texWidth;
                    float v0 = glyph.pixelBoundBottom / texHeight;
                    float u1 = glyph.pixelBoundRight / texWidth;
                    float v1 = glyph.pixelBoundTop / texHeight;

                    // Add vertices for the character quad (two triangles - 6 vertices)
                    m_vertices.insert(m_vertices.end(), {
                        {xpos,     ypos + h,   u0, v1},  // Top-left
                        {xpos,     ypos,       u0, v0},  // Bottom-left
                        {xpos + w, ypos,       u1, v0},  // Bottom-right

                        {xpos,     ypos + h,   u0, v1},  // Top-left (again)
                        {xpos + w, ypos,       u1, v0},  // Bottom-right (again)
                        {xpos + w, ypos + h,   u1, v1}   // Top-right (again)
                        });

                    currentX += (uint32_t)(glyph.horizAdvanceEm); // Update currentX
                    break;
                }
            }

            if (!glyphFound) {
                LOG_WARN("[Font]: Glyph for char {} (U+{:04X}) not found.", c, (uint32_t)c);
            }
        }

        // upload to gpu
        m_pDevice->writeBuffer(m_textVertexBuffer, m_vertices.size() * sizeof(TextVertex), m_vertices.data());

        uint32_t triCount = m_vertices.size() / 3;

        m_pDevice->draw({
            .vertexBufer = m_textVertexBuffer,
            .shader = m_textShader,
            .vertexLayout = m_textVertexLayout
            }, triCount);
    }
}