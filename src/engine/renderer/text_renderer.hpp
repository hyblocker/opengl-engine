#pragma once

#include "engine/core.hpp"
#include "engine/gpu/idevice.hpp"
#include "scene_graph.hpp"

#include <unordered_map>

namespace render {

    class FontRenderer;

    struct GlyphRect {
        double left;
        double right;
        double top;
        double bottom;
    };

    struct GlyphInfo {
        uint32_t unicodeCodePoint;
        float horizAdvanceEm;
        hlslpp::float4 quadBounds; // quadBound
        hlslpp::float4 pixelBounds; // pixelBound
    };
    struct FontData {
        friend class FontRenderer;

        std::unordered_map<uint32_t, GlyphInfo> glyphs;
    private:
        bool isValid = false;
        uint32_t firstCodePoint = 0;
        float fontSize = 37.84375f; // Hardcoded, ik its bad but imagine adding a json parser
        gpu::ITexture* texture = nullptr;
    };

    class FontRenderer {
    public:
        FontRenderer();
        void init(gpu::IDevice* device);

        struct TextDrawParams {
            float posX = 0;
            float posY = 0;
            float outlineWidth;
            hlslpp::float4 colourForeground;
            hlslpp::float4 colourOutline;
            float size = 1.0f;
            float lineHeight = 1.0f;
            std::string text;
        };

        FontData loadFont(const std::string& filePath, gpu::ITexture* texture);
        void drawText(const FontData& fontData, const TextDrawParams params, render::Entity* pEntity, render::Camera* pCameraComponent);

    private:

        struct TextVertex {
            float pos[2];
            float uv[2];
        };

        std::vector<TextVertex> m_vertices;

        gpu::IDevice* m_pDevice = nullptr;
        gpu::IShader* m_textShader = nullptr;
        gpu::TextureSamplerHandle m_trillinearAniso16ClampSampler;
        gpu::InputLayoutHandle m_textVertexLayout;
        gpu::BufferHandle m_textVertexBuffer;

        struct TextCBuffer {
            hlslpp::float4x4 model;
            hlslpp::float4x4 view;
            hlslpp::float4x4 projection;
            hlslpp::float4 colourForeground;
            hlslpp::float4 colourOutline;
            float outlineWidth;
        };

        gpu::BufferHandle m_textCBuffer;

        std::string m_executableDir;
    };

}