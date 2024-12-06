#pragma once

#include "engine/gpu/idevice.hpp"
#include <glad/glad.h>

namespace gpu::gl {

	struct GlGpuFormatMapping {
		gpu::GpuFormat format;
		size_t size;
		GLenum glType;
		GLboolean glNormalised;
	};

	GlGpuFormatMapping getGlFormat(gpu::GpuFormat format);

	struct GlUsageMapping {
		gpu::Usage usage;
		GLenum glUsage;
	};

	GlUsageMapping getGlUsage(gpu::Usage usage);

	struct GlBufferTypeMapping {
		gpu::BufferType type;
		GLenum glType;
	};

	GlBufferTypeMapping getGlBufferType(gpu::BufferType type);

	struct GlPrimitiveTypeMapping {
		gpu::PrimitiveType type;
		GLenum glType;
	};

	GlPrimitiveTypeMapping getGlPrimitiveType(gpu::PrimitiveType type);

	struct GlAccessFlagsMapping {
		gpu::MapAccessFlags accessFlags;
		GLbitfield glFlag;
	};

	GlAccessFlagsMapping getGlAccessFlags(gpu::MapAccessFlags type);

	struct GlTextureWrapMapping {
		gpu::TextureWrap textureWrap;
		GLenum glEnum;
	};

	GlTextureWrapMapping getGlWrapMode(gpu::TextureWrap wrapMode);

	struct GlTextureTypeMapping {
		gpu::TextureType textureType;
		GLenum glEnum;
	};

	GlTextureTypeMapping getGlTextureType(gpu::TextureType type);
}