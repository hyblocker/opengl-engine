#pragma once

#include <inttypes.h>
#include <string>

#include "engine/refcounter.hpp"

namespace gpu {
	struct TextureDesc {
		uint32_t width;
		uint32_t height;
	};

	class ITexture {

	};

	typedef engine::RefCounter<ITexture> TextureHandle;

	class IFramebuffer {

	};

	typedef engine::RefCounter<IFramebuffer> FramebufferHandle;
}