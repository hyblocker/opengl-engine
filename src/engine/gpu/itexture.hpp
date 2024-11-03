#pragma once

#include <inttypes.h>
#include <string>

#include "engine/refcounter.hpp"

namespace gpu {
	class ITexture {

	};

	typedef engine::RefCounter<ITexture> TextureHandle;

	class IFramebuffer {

	};

	typedef engine::RefCounter<IFramebuffer> FramebufferHandle;
}