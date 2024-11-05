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
            "The usage buffer type table doesn't have the correct number of elements");

        const GlPrimitiveTypeMapping mapping = c_primitiveTypeMappings[uint32_t(type)];
        ASSERT(mapping.type == type);
        return mapping;
    }
}