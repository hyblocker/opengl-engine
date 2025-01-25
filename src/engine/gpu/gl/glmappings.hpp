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

	struct GlDepthFuncMapping {
		gpu::CompareFunc compareFunc;
		GLenum glEnum;
	};

	GlDepthFuncMapping getGlDepthFunc(gpu::CompareFunc func);

	struct GlTextureFormatMapping {
		gpu::TextureFormat format;
		GLenum glEnum;
	};

	GlTextureFormatMapping getGlTextureFormat(gpu::TextureFormat format);

	struct GlBlendFactorMapping {
		gpu::BlendFactor factor;
		GLenum glEnum;
	};

	GlBlendFactorMapping getGlBlendFactor(gpu::BlendFactor factor);

	struct GlBlendOpMapping {
		gpu::BlendOp operation;
		GLenum glEnum;
	};

	GlBlendOpMapping getGlBlendOp(gpu::BlendOp operation);
}