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
	[[nodiscard]] const uint32_t GlTexture::getNativeObject() const {
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
		glGenerateMipmap(getGlTextureType(desc.type).glEnum);

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
	[[nodiscard]] const uint32_t GlTextureSampler::getNativeObject() const {
		return m_pointer;
	}

	TextureSamplerHandle GlDevice::makeTextureSampler(TextureSamplerDesc desc) {
		ASSERT(desc.wrapX != gpu::TextureWrap::Count);
		ASSERT(desc.wrapY != gpu::TextureWrap::Count);
		ASSERT(desc.wrapZ != gpu::TextureWrap::Count);

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

		GlTextureSampler* sampler = new GlTextureSampler();
		sampler->m_desc = desc;
		sampler->m_pointer = glSampler;

		return TextureSamplerHandle::Create(sampler);
	}
}