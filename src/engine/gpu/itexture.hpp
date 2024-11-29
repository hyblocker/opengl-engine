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
	public:
		ITexture() = default;
		virtual ~ITexture() = default;
		[[nodiscard]] virtual const TextureDesc getDesc() const = 0;
		[[nodiscard]] virtual const uint32_t getNativeObject() const = 0;
	};

	typedef engine::RefCounter<ITexture> TextureHandle;

	class IFramebuffer {

	};

	typedef engine::RefCounter<IFramebuffer> FramebufferHandle;
}