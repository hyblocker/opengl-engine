#include "glmappings.hpp"
#include "engine/core.hpp"

namespace gpu::gl {

    // Format mapping table. The rows must be in the exactly same order as Format enum members are defined.
    static const GlGpuFormatMapping c_formatMappings[] = {
        { GpuFormat::Unknown, 0, 0, false },

        { GpuFormat::Uint8_TYPELESS, 1, GL_UNSIGNED_BYTE, false },
        { GpuFormat::Uint16_TYPELESS, 1, GL_UNSIGNED_SHORT, false },
        { GpuFormat::Uint32_TYPELESS, 1, GL_UNSIGNED_INT, false },

        { GpuFormat::Int8_TYPELESS, 1, GL_BYTE, false },
        { GpuFormat::Int16_TYPELESS, 1, GL_SHORT, false },
        { GpuFormat::Int32_TYPELESS, 1, GL_INT, false },

        { GpuFormat::R8_UNORM, 1, GL_FLOAT, true },
        { GpuFormat::RG8_UNORM, 2, GL_FLOAT, true },
        { GpuFormat::RGB8_UNORM, 3, GL_FLOAT, true },
        { GpuFormat::RGBA8_UNORM, 4, GL_FLOAT, true },

        { GpuFormat::R8_TYPELESS, 1, GL_FLOAT, false },
        { GpuFormat::RG8_TYPELESS, 2, GL_FLOAT, false },
        { GpuFormat::RGB8_TYPELESS, 3, GL_FLOAT, false },
        { GpuFormat::RGBA8_TYPELESS, 4, GL_FLOAT, false },

    };

    GlGpuFormatMapping getGlFormat(gpu::GpuFormat format) {
        static_assert(sizeof(c_formatMappings) / sizeof(GlGpuFormatMapping) == size_t(GpuFormat::Count),
            "The format mapping table doesn't have the correct number of elements");

        const GlGpuFormatMapping mapping = c_formatMappings[uint32_t(format)];
        ASSERT(mapping.format == format);
        return mapping;
    }

    static const GlUsageMapping c_usageMappings[] = {
        { Usage::Default,       GL_STATIC_DRAW },
        { Usage::Immutable,     GL_STATIC_DRAW },
        { Usage::Dynamic,       GL_DYNAMIC_DRAW },
        { Usage::Staging,       GL_STREAM_READ },
    };

    GlUsageMapping getGlUsage(gpu::Usage usage) {
        static_assert(sizeof(c_usageMappings) / sizeof(GlUsageMapping) == size_t(Usage::Count),
            "The usage mapping table doesn't have the correct number of elements");

        const GlUsageMapping mapping = c_usageMappings[uint32_t(usage)];
        ASSERT(mapping.usage == usage);
        return mapping;
    }

    static const GlBufferTypeMapping c_bufferTypeMappings[] = {

        { BufferType::VertexBuffer,             GL_ARRAY_BUFFER },
        { BufferType::IndexBuffer,              GL_ELEMENT_ARRAY_BUFFER },
        { BufferType::ConstantBuffer,           GL_UNIFORM_BUFFER },
        { BufferType::BufferCopySource,         GL_COPY_READ_BUFFER },
        { BufferType::BufferCopyDestination,    GL_COPY_WRITE_BUFFER },
        { BufferType::PixelReadTarget,          GL_PIXEL_PACK_BUFFER },
        { BufferType::TextureDataSource,        GL_PIXEL_UNPACK_BUFFER },
        { BufferType::TransformFeedbackBuffer,  GL_TRANSFORM_FEEDBACK_BUFFER },

    };

    GlBufferTypeMapping getGlBufferType(gpu::BufferType type) {
        static_assert(sizeof(c_bufferTypeMappings) / sizeof(GlBufferTypeMapping) == size_t(BufferType::Count),
            "The usage buffer type table doesn't have the correct number of elements");

        const GlBufferTypeMapping mapping = c_bufferTypeMappings[uint32_t(type)];
        ASSERT(mapping.type == type);
        return mapping;
    }

    static const GlPrimitiveTypeMapping c_primitiveTypeMappings[] = {

        { PrimitiveType::Triangles,     GL_TRIANGLES },
        { PrimitiveType::Points,        GL_POINTS },
        { PrimitiveType::Lines,         GL_LINES },
    };

    GlPrimitiveTypeMapping getGlPrimitiveType(gpu::PrimitiveType type) {
        static_assert(sizeof(c_primitiveTypeMappings) / sizeof(GlPrimitiveTypeMapping) == size_t(PrimitiveType::Count),
            "The primitive type table doesn't have the correct number of elements");

        const GlPrimitiveTypeMapping mapping = c_primitiveTypeMappings[uint32_t(type)];
        ASSERT(mapping.type == type);
        return mapping;
    }

    static const GlAccessFlagsMapping c_accessFlagsMappings[] = {

        { MapAccessFlags::Write,        GL_MAP_WRITE_BIT },
        { MapAccessFlags::Read,         GL_MAP_READ_BIT },
        // { MapAccessFlags::Persistent,   GL_MAP_PERSISTENT_BIT },
        // { MapAccessFlags::Coherent,     GL_MAP_COHERENT_BIT },
    };

    GlAccessFlagsMapping getGlAccessFlags(gpu::MapAccessFlags flags) {

        // can't do asserts due to how this enum is defined in the spec
        uint8_t flagsUnique = uint32_t(flags) & 0x01;
        uint8_t flagsBitwise = uint32_t(flags) & 0x10;
        GlAccessFlagsMapping mapping = c_accessFlagsMappings[flagsUnique];

        switch (flagsBitwise) {
        case 0x10:
            mapping.glFlag |= GL_MAP_INVALIDATE_RANGE_BIT;
            break;
        case 0x20:
            mapping.glFlag |= GL_MAP_INVALIDATE_BUFFER_BIT;
            break;
        case 0x40:
            mapping.glFlag |= GL_MAP_FLUSH_EXPLICIT_BIT;
            break;
        case 0x80:
            mapping.glFlag |= GL_MAP_UNSYNCHRONIZED_BIT;
            break;
        }

        return mapping;
    }

    static const GlTextureWrapMapping c_textureWrapMappings[] = {

        { TextureWrap::Repeat,              GL_REPEAT },
        { TextureWrap::MirrorRepeat,        GL_MIRRORED_REPEAT },
        // { TextureWrap::MirrorClampToEdge,   GL_MIRROR_CLAMP_TO_EDGE },
        { TextureWrap::ClampToEdge,         GL_CLAMP_TO_EDGE },
        // { TextureWrap::ClampToBorder,       GL_CLAMP_TO_BORDER },
    };

    GlTextureWrapMapping getGlWrapMode(gpu::TextureWrap wrapMode) {
        static_assert(sizeof(c_textureWrapMappings) / sizeof(GlTextureWrapMapping) == size_t(TextureWrap::Count),
            "The wrap mode table doesn't have the correct number of elements");

        const GlTextureWrapMapping mapping = c_textureWrapMappings[uint32_t(wrapMode)];
        ASSERT(mapping.textureWrap == wrapMode);
        return mapping;
    }

    static const GlTextureTypeMapping c_textureTypeMappings[] = {

        // { TextureType::Texture1D,                   GL_TEXTURE_1D },
        { TextureType::Texture2D,                   GL_TEXTURE_2D },
        { TextureType::Texture3D,                   GL_TEXTURE_3D },
        // { TextureType::TextureArray1D,              GL_TEXTURE_1D_ARRAY },
        { TextureType::TextureArray2D,              GL_TEXTURE_2D_ARRAY },
        // { TextureType::TextureRectangle,            GL_TEXTURE_RECTANGLE },
        { TextureType::TextureCubeMap,              GL_TEXTURE_CUBE_MAP },
        // { TextureType::TextureArrayCubeMap,         GL_TEXTURE_CUBE_MAP_ARRAY },
        // { TextureType::TextureBuffer,               GL_TEXTURE_BUFFER },
        // { TextureType::TextureMultisample2D,        GL_TEXTURE_2D_MULTISAMPLE },
        // { TextureType::TextureArrayMultisample2D,   GL_TEXTURE_2D_MULTISAMPLE_ARRAY },
    };

    GlTextureTypeMapping getGlTextureType(gpu::TextureType type) {
        static_assert(sizeof(c_textureTypeMappings) / sizeof(GlTextureTypeMapping) == size_t(TextureType::Count),
            "The texture type table doesn't have the correct number of elements");

        const GlTextureTypeMapping mapping = c_textureTypeMappings[uint32_t(type)];
        ASSERT(mapping.textureType == type);
        return mapping;
    }

    static const GlDepthFuncMapping c_depthCompareFuncMappings[] = {

        { CompareFunc::Never,           GL_NEVER },
        { CompareFunc::Less,            GL_LESS },
        { CompareFunc::Equal,           GL_EQUAL },
        { CompareFunc::LessOrEqual,     GL_LEQUAL },
        { CompareFunc::Greater,         GL_GREATER },
        { CompareFunc::NotEqual,        GL_NOTEQUAL },
        { CompareFunc::GreaterOrEqual,  GL_GEQUAL },
        { CompareFunc::Always,          GL_ALWAYS },
    };

    GlDepthFuncMapping getGlDepthFunc(gpu::CompareFunc func) {
        static_assert(sizeof(c_depthCompareFuncMappings) / sizeof(GlDepthFuncMapping) == size_t(CompareFunc::Count),
            "The depth compare function table doesn't have the correct number of elements");

        const GlDepthFuncMapping mapping = c_depthCompareFuncMappings[uint32_t(func)];
        ASSERT(mapping.compareFunc == func);
        return mapping;
    }

    static const GlTextureFormatMapping c_textureFormatMappings[] = {

        { TextureFormat::RGBA4,             GL_RGBA4 },
        { TextureFormat::RGB5_A1,           GL_RGB5_A1 },
        { TextureFormat::RGBA8,             GL_RGBA8 },
        { TextureFormat::RGB10_A2,          GL_RGB10_A2 },
        { TextureFormat::R11G11B10,         GL_R11F_G11F_B10F },
        { TextureFormat::SRGB8,             GL_SRGB8 },
        { TextureFormat::SRGB8_A8,          GL_SRGB8_ALPHA8 },

        { TextureFormat::Depth16,           GL_DEPTH_COMPONENT16 },
        { TextureFormat::Depth24,           GL_DEPTH_COMPONENT24 },
        { TextureFormat::Depth32,           GL_DEPTH_COMPONENT32F },
        { TextureFormat::Depth24_Stencil8,  GL_DEPTH24_STENCIL8 },
        { TextureFormat::Depth32_Stencil8,  GL_DEPTH32F_STENCIL8 },
    };

    GlTextureFormatMapping getGlTextureFormat(gpu::TextureFormat format) {
        static_assert(sizeof(c_textureFormatMappings) / sizeof(GlTextureFormatMapping) == size_t(TextureFormat::Count),
            "The texture format table doesn't have the correct number of elements");

        const GlTextureFormatMapping mapping = c_textureFormatMappings[uint32_t(format)];
        ASSERT(mapping.format == format);
        return mapping;
    }

    static const GlBlendFactorMapping c_blendFactorMappings[] = {

        { BlendFactor::Zero,                GL_ZERO },
        { BlendFactor::One,                 GL_ONE },

        { BlendFactor::SrcColour,           GL_SRC_COLOR },
        { BlendFactor::OneMinusSrcColour,   GL_ONE_MINUS_SRC_COLOR },
        { BlendFactor::SrcAlpha,            GL_SRC_ALPHA },
        { BlendFactor::OneMinusSrcAlpha,    GL_ONE_MINUS_SRC_ALPHA },

        { BlendFactor::DstColour,           GL_DST_COLOR },
        { BlendFactor::OneMinusDstColour,   GL_ONE_MINUS_DST_COLOR },
        { BlendFactor::DstAlpha,            GL_DST_ALPHA },
        { BlendFactor::OneMinusDstAlpha,    GL_ONE_MINUS_DST_ALPHA },

        { BlendFactor::Src1Colour,          GL_SRC1_COLOR },
        { BlendFactor::OneMinusSrc1Colour,  GL_ONE_MINUS_SRC1_COLOR },
        { BlendFactor::Src1Alpha,           GL_SRC1_ALPHA },
        { BlendFactor::OneMinusSrc1Alpha,   GL_ONE_MINUS_SRC1_ALPHA },

    };

    GlBlendFactorMapping getGlBlendFactor(gpu::BlendFactor factor) {
        static_assert(sizeof(c_blendFactorMappings) / sizeof(GlBlendFactorMapping) == size_t(BlendFactor::Count),
            "The blend factor table doesn't have the correct number of elements");

        const GlBlendFactorMapping mapping = c_blendFactorMappings[uint32_t(factor)];
        ASSERT(mapping.factor == factor);
        return mapping;
    }

    static const GlBlendOpMapping c_blendOpMappings[] = {

        { BlendOp::Add,             GL_FUNC_ADD },
        { BlendOp::Subtract,        GL_FUNC_SUBTRACT },
        { BlendOp::InvSubtract,     GL_FUNC_REVERSE_SUBTRACT },
        { BlendOp::Min,             GL_MIN },
        { BlendOp::Max,             GL_MAX },

    };

    GlBlendOpMapping getGlBlendOp(gpu::BlendOp operation) {
        static_assert(sizeof(c_blendOpMappings) / sizeof(GlBlendOpMapping) == size_t(BlendOp::Count),
            "The blend operation table doesn't have the correct number of elements");

        const GlBlendOpMapping mapping = c_blendOpMappings[uint32_t(operation)];
        ASSERT(mapping.operation == operation);
        return mapping;
    }
}