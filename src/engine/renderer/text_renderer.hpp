#pragma once

#include "engine/core.hpp"
#include "engine/gpu/idevice.hpp"
#include "scene_graph.hpp"

namespace render {

    class FontRenderer;

    struct GlyphInfo {
        uint32_t unicodeCodePoint;
        double horizAdvanceEm;
        double quadBoundLeft;
        double quadBoundBottom;
        double quadBoundRight;
        double quadBoundTop;
        double pixelBoundLeft;
        double pixelBoundBottom;
        double pixelBoundRight;
        double pixelBoundTop;
    };
    struct FontData {
        friend class FontRenderer;

        std::vector<GlyphInfo> glyphs;
    private:
        bool isValid = false;
        uint32_t firstCodePoint = 0;
        gpu::ITexture* texture = nullptr;
    };

    class FontRenderer {
    public:
        FontRenderer();
        void init(gpu::IDevice* device);

        struct TextDrawParams {
            uint32_t posX = 0;
            uint32_t posY = 0;
            float outlineWidth;
            hlslpp::float4 colourForeground;
            hlslpp::float4 colourOutline;
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
        gpu::IShader* m_textShader;
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