#pragma once

#include <inttypes.h>
#include <string>

#include "itypes.hpp"
#include "engine/refcounter.hpp"

namespace gpu {

	struct TextureDesc {
		uint32_t width = 0;
		uint32_t height = 0;

		bool generateMipmaps = true;

		TextureType type = TextureType::Texture2D;

		std::string debugName = "";
	};

	class ITexture {
	public:
		ITexture() = default;
		virtual ~ITexture() = default;
		[[nodiscard]] virtual const TextureDesc getDesc() const = 0;
		[[nodiscard]] virtual const GpuPtr getNativeObject() const = 0;
	};

	typedef engine::RefCounter<ITexture> TextureHandle;

	// default texture constant, used to unbind texture
	constexpr ITexture* k_unbindTexture = static_cast<ITexture*>(nullptr);

	struct TextureSamplerDesc {

		SamplingMode minFilter = SamplingMode::Linear;
		SamplingMode magFilter = SamplingMode::Linear;
		SamplingMode mipFilter = SamplingMode::Linear; // Trilinear sampling, Nearest = Bilinear sampling

		TextureWrap wrapX = TextureWrap::Repeat;
		TextureWrap wrapY = TextureWrap::Repeat;
		TextureWrap wrapZ = TextureWrap::Repeat;

		float lodBias = 0;

		// Target anisotropy. Clamped to the maximum supported by the hardware
		float anisotropy = 16.0f;

		std::string debugName = "";
	};

	class ITextureSampler {
	public:
		virtual ~ITextureSampler() = default;
		[[nodiscard]] virtual const TextureSamplerDesc& getDesc() const = 0;
		[[nodiscard]] virtual const GpuPtr getNativeObject() const = 0;
	};
	typedef engine::RefCounter<ITextureSampler> TextureSamplerHandle;

	struct FramebufferAttachmentDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		uint8_t samples = 1; // MSAA , clamped between 1 and max supported in hardware. Call device->getMaxMSAA() to get the max value from the driver. MSAA 1 disables MSAA
		gpu::TextureFormat format;
	};

	struct FramebufferDesc {
		FramebufferAttachmentDesc colorDesc = {
			.width = 1,
			.height = 1,
			.samples = 1,
			.format = gpu::TextureFormat::RGBA8,
		};
		FramebufferAttachmentDesc depthStencilDesc = {
			.width = 1,
			.height = 1,
			.samples = 1,
			.format = gpu::TextureFormat::Depth24_Stencil8,
		};
		// must be set to true for depth stencil attachment to be valid
		bool hasDepth = true;

		std::string debugName = "";
	};

	class IFramebuffer {
	public:
		virtual ~IFramebuffer() = default;
		[[nodiscard]] virtual const FramebufferDesc& getDesc() const = 0;
		[[nodiscard]] virtual const GpuPtr getNativeObject() const = 0;
		[[nodiscard]] virtual const GpuPtr getTextureNativeObject() const = 0;
	};

	typedef engine::RefCounter<IFramebuffer> FramebufferHandle;
	
	// default framebuffer constant, maps to backbuffer
	constexpr IFramebuffer* k_defaultFramebuffer = static_cast<IFramebuffer*>(nullptr);
}