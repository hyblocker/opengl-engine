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
}