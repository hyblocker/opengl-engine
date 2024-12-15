#include <algorithm>

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
		glGenTextures(1, &glTexture);

		glBindTexture(getGlTextureType(desc.type).glEnum, glTexture);

		// Upload data
		glTexImage2D(getGlTextureType(desc.type).glEnum, 0, GL_RGB, desc.width, desc.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
		
		if (desc.generateMipmaps) {
			glGenerateMipmap(getGlTextureType(desc.type).glEnum);
		}

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

		// Ask the GPU driver what the maximum anisotropy the hardware supports is, so that we may clamp with it
		static GLfloat maximumAniso = -1.0;
		static bool maximumAnisoQueried = false;
		if (maximumAnisoQueried == false) {
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAniso);
			maximumAnisoQueried = true;
		}

		// clamp anisotropy to known range, 1 is defined as minimum by the spec, max is the max as defined by the hardware
		float newAniso = std::clamp(desc.anisotropy, 1.0f, maximumAniso);
		// update descriptor before using it
		desc.anisotropy = newAniso;

		GLuint glSampler = 0;
		glGenSamplers(1, &glSampler);

		GLuint minFilter = desc.minFilter == gpu::SamplingMode::Linear ? GL_LINEAR : GL_NEAREST;
		if (desc.mipFilter == gpu::SamplingMode::Linear) {
			minFilter = desc.minFilter == gpu::SamplingMode::Linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
		}

		glSamplerParameteri(glSampler, GL_TEXTURE_MIN_FILTER, minFilter);
		glSamplerParameteri(glSampler, GL_TEXTURE_MAG_FILTER, desc.magFilter == gpu::SamplingMode::Linear ? GL_LINEAR : GL_NEAREST);
		glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_S, getGlWrapMode(desc.wrapX).glEnum);
		glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_T, getGlWrapMode(desc.wrapY).glEnum);
		glSamplerParameteri(glSampler, GL_TEXTURE_WRAP_R, getGlWrapMode(desc.wrapZ).glEnum);
		glSamplerParameteri(glSampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<GLint>(desc.anisotropy));

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
		glGenFramebuffers(1, &glFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, glFramebuffer);

		// make colour attachment
		GLuint textureColorbuffer = 0;
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, desc.colorDesc.samples, getGlTextureFormat(desc.colorDesc.format).glEnum, desc.colorDesc.width, desc.colorDesc.height, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

		// make depth stencil attachment
		GLuint rbo = 0;
		if (desc.hasDepth) {
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, getGlTextureFormat(desc.depthStencilDesc.format).glEnum, desc.depthStencilDesc.width, desc.depthStencilDesc.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
		}

		// reset state (unbind fbo)
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GlFramebuffer* framebuffer = new GlFramebuffer();
		framebuffer->m_desc = desc;
		framebuffer->m_pointer = glFramebuffer;
		framebuffer->m_textureAttachment = textureColorbuffer;
		framebuffer->m_depthStencilAttachment = rbo;

		return FramebufferHandle::Create(framebuffer);
	}
}