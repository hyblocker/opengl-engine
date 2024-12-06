#pragma once

#include <inttypes.h>
#include <string>

#include "engine/refcounter.hpp"

namespace gpu {

	enum class TextureType : uint8_t {
		Texture1D,
		Texture2D,
		Texture3D,
		TextureArray1D,
		TextureArray2D,
		TextureRectangle,
		TextureCubeMap,
		TextureArrayCubeMap,
		TextureBuffer,
		TextureMultisample2D,
		TextureArrayMultisample2D,
		Count,
	};

	struct TextureDesc {
		uint32_t width = 0;
		uint32_t height = 0;

		TextureType type = TextureType::Texture2D;
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

	enum class SamplingMode : uint8_t {
		Nearest,
		Linear,
	};

	enum class TextureWrap : uint8_t {
		Repeat,
		MirrorRepeat,
		MirrorClampToEdge,
		ClampToEdge,
		ClampToBorder,
		Count
	};

	struct TextureSamplerDesc {

		SamplingMode minFilter = SamplingMode::Linear;
		SamplingMode magFilter = SamplingMode::Linear;
		SamplingMode mipFilter = SamplingMode::Linear; // Trilinear sampling, Nearest = Bilinear sampling

		TextureWrap wrapX = TextureWrap::Repeat;
		TextureWrap wrapY = TextureWrap::Repeat;
		TextureWrap wrapZ = TextureWrap::Repeat;
	};

	class ITextureSampler {
	public:
		virtual ~ITextureSampler() = default;
		[[nodiscard]] virtual const TextureSamplerDesc& getDesc() const = 0;
		[[nodiscard]] virtual const uint32_t getNativeObject() const = 0;
	};
	typedef engine::RefCounter<ITextureSampler> TextureSamplerHandle;
}