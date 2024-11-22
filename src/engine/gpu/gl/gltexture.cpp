#include "engine/gpu/itexture.hpp"
#include "engine/gpu/gl/gldevice.hpp"

namespace gpu::gl {

	TextureHandle GlDevice::makeTexture(TextureDesc desc) {
		return TextureHandle::Create(nullptr);
	}
	TextureSamplerHandle GlDevice::makeTextureSampler(TextureSamplerDesc desc) {
		return TextureSamplerHandle::Create(nullptr);
	}
}