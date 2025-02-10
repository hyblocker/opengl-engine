#include <algorithm>

#include <fmt/format.h>
#include "engine/gpu/itexture.hpp"
#include "engine/gpu/gl/gldevice.hpp"
#include "engine/gpu/gl/glmappings.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"

namespace gpu::gl {
	GlTexture::~GlTexture() {
		glDeleteTextures(1, &m_pointer);
	}
	[[nodiscard]] const TextureDesc GlTexture::getDesc() const {
		return m_desc;
	}
	[[nodiscard]] const GpuPtr GlTexture::getNativeObject() const {
		return m_pointer;
	}

	TextureHandle GlDevice::makeTexture(TextureDesc desc, void* textureData) {
		ASSERT(desc.width > 0);
		ASSERT(desc.height > 0);
		ASSERT(desc.type != gpu::TextureType::Count);

		GLuint glTexture = 0;
		GL_CHECK(glGenTextures(1, &glTexture));

		GL_CHECK(glBindTexture(getGlTextureType(desc.type).glEnum, glTexture));

		// Upload data
		GL_CHECK(glTexImage2D(getGlTextureType(desc.type).glEnum, 0, GL_RGBA, desc.width, desc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));
		
		if (desc.generateMipmaps) {
			GL_CHECK(glGenerateMipmap(getGlTextureType(desc.type).glEnum));
		}

#if _DEBUG
		if (!desc.debugName.empty()) {
			GL_CHECK(glObjectLabel(GL_TEXTURE, glTexture, -1, desc.debugName.c_str()));
		}
#endif

		GlTexture* texture = new GlTexture();
		texture->m_desc = desc;
		texture->m_pointer = glTexture;

		return TextureHandle::Create(texture);
	}

	GlTextureSampler::~GlTextureSampler() {
		glDeleteSamplers(1, &m_pointer);
	}
	[[nodiscard]] const TextureSamplerDesc& GlTextureSampler::getDesc() const {
		return m_desc;
	}
	[[nodiscard]] const GpuPtr GlTextureSampler::getNativeObject() const {
		return m_pointer;
	}

	TextureSamplerHandle GlDevice::makeTextureSampler(TextureSamplerDesc desc) {
		ASSERT(desc.wrapX != gpu::TextureWrap::Count);
		ASSERT(desc.wrapY != gpu::TextureWrap::Count);
		ASSERT(desc.wrapZ != gpu::TextureWrap::Count);


		// clamp anisotropy to known range, 1 is defined as minimum by the spec, max is the max as defined by the hardware
		float newAniso = std::clamp(desc.anisotropy, 1.0f, m_maxTextureMaxAnisotropyExt);
		// update descriptor before using it
		desc.anisotropy = newAniso;

		GLuint glSampler = 0;
		GL_CHECK(glGenSamplers(1, &glSampler));

		GLuint minFilter = desc.minFilter == gpu::SamplingMode::Linear ? GL_LINEAR : GL_NEAREST;
		if (desc.mipFilter == gpu::SamplingMode::Linear) {
			minFilter = desc.minFilter == gpu::SamplingMode::Linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
		}

		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_MIN_FILTER, minFilter));
		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_MAG_FILTER, desc.magFilter == gpu::SamplingMode::Linear ? GL_LINEAR : GL_NEAREST));
		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_S, getGlWrapMode(desc.wrapX).glEnum));
		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_T, getGlWrapMode(desc.wrapY).glEnum));
		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_R, getGlWrapMode(desc.wrapZ).glEnum));
		GL_CHECK(glSamplerParameteri(glSampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<GLint>(desc.anisotropy)));

#if _DEBUG
		if (!desc.debugName.empty()) {
			GL_CHECK(glObjectLabel(GL_SAMPLER, glSampler, -1, desc.debugName.c_str()));
		}
#endif

		GlTextureSampler* sampler = new GlTextureSampler();
		sampler->m_desc = desc;
		sampler->m_pointer = glSampler;

		return TextureSamplerHandle::Create(sampler);
	}

	GlFramebuffer::~GlFramebuffer() {
		glDeleteTextures(1, &m_textureAttachment);
		if (m_desc.hasDepth) {
			glDeleteRenderbuffers(1, &m_depthStencilAttachment);
		}
		glDeleteFramebuffers(1, &m_pointer);
	}
	[[nodiscard]] const FramebufferDesc& GlFramebuffer::getDesc() const {
		return m_desc;
	}
	[[nodiscard]] const GpuPtr GlFramebuffer::getNativeObject() const {
		return m_pointer;
	}
	[[nodiscard]] const GpuPtr GlFramebuffer::getTextureNativeObject() const {
		return m_textureAttachment;
	}

	FramebufferHandle GlDevice::makeFramebuffer(FramebufferDesc desc) {
		
		// Assert MSAA sample count is valid and format isn't depth stencil
		ASSERT(desc.colorDesc.width > 0);
		ASSERT(desc.colorDesc.height > 0);
		ASSERT(desc.colorDesc.samples > 0);
		ASSERT(
			desc.colorDesc.format != gpu::TextureFormat::Depth16 &&
			desc.colorDesc.format != gpu::TextureFormat::Depth24 &&
			desc.colorDesc.format != gpu::TextureFormat::Depth32 &&
			desc.colorDesc.format != gpu::TextureFormat::Depth24_Stencil8 &&
			desc.colorDesc.format != gpu::TextureFormat::Depth32_Stencil8
		);

		if (desc.hasDepth) {
			// Ensure dimensions are the same as that of the colour attachment
			ASSERT(desc.depthStencilDesc.width == desc.colorDesc.width);
			ASSERT(desc.depthStencilDesc.height == desc.colorDesc.height);
			// Ensure depth attachment is valid
			ASSERT(desc.depthStencilDesc.samples == 1);
			ASSERT(
				desc.depthStencilDesc.format == gpu::TextureFormat::Depth16 ||
				desc.depthStencilDesc.format == gpu::TextureFormat::Depth24 ||
				desc.depthStencilDesc.format == gpu::TextureFormat::Depth32 ||
				desc.depthStencilDesc.format == gpu::TextureFormat::Depth24_Stencil8 ||
				desc.depthStencilDesc.format == gpu::TextureFormat::Depth32_Stencil8
			);
		}

		// make fbo
		GLuint glFramebuffer = 0;
		GL_CHECK(glGenFramebuffers(1, &glFramebuffer));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer));

		// make colour attachment
		GLuint textureColorbuffer = 0;
		GL_CHECK(glGenTextures(1, &textureColorbuffer));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureColorbuffer));
		GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, desc.colorDesc.samples, getGlTextureFormat(desc.colorDesc.format).glEnum, desc.colorDesc.width, desc.colorDesc.height, GL_TRUE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0));

		// make depth stencil attachment
		GLuint rbo = 0;
		if (desc.hasDepth) {
			GL_CHECK(glGenRenderbuffers(1, &rbo));
			GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
			GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, getGlTextureFormat(desc.depthStencilDesc.format).glEnum, desc.depthStencilDesc.width, desc.depthStencilDesc.height));
			GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo));
		}

		// reset state (unbind fbo)
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

#if _DEBUG
		if (!desc.debugName.empty()) {
			GL_CHECK(glObjectLabel(GL_FRAMEBUFFER, glFramebuffer, -1, desc.debugName.c_str()));
			GL_CHECK(glObjectLabel(GL_TEXTURE, textureColorbuffer, -1, fmt::format("{}_colorAttachment", desc.debugName).c_str()));
			if (desc.hasDepth) {
				GL_CHECK(glObjectLabel(GL_RENDERBUFFER, rbo, -1, fmt::format("{}_depthStencilAttachment", desc.debugName).c_str()));
			}
		}
#endif

		GlFramebuffer* framebuffer = new GlFramebuffer();
		framebuffer->m_desc = desc;
		framebuffer->m_pointer = glFramebuffer;
		framebuffer->m_textureAttachment = textureColorbuffer;
		framebuffer->m_depthStencilAttachment = rbo;

		return FramebufferHandle::Create(framebuffer);
	}
}