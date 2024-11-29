#include "engine/gpu/itexture.hpp"
#include "engine/gpu/gl/gldevice.hpp"

namespace gpu::gl {
	GlTexture::~GlTexture() {

	}
	[[nodiscard]] const TextureDesc GlTexture::getDesc() const
	{
	}
	[[nodiscard]] const uint32_t GlTexture::getNativeObject() const {

	}

	TextureHandle GlDevice::makeTexture(TextureDesc desc) {
		return TextureHandle::Create(nullptr);
	}
	TextureSamplerHandle GlDevice::makeTextureSampler(TextureSamplerDesc desc) {
		return TextureSamplerHandle::Create(nullptr);
	}
}