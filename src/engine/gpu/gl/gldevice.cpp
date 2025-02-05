#include <fmt/format.h>
#include <stb_image.h>
#include <algorithm>

#include "gldevice.hpp"
#include "glmappings.hpp"
#include "engine/core.hpp"
#include "engine/log.hpp"

namespace gpu::gl {

	GlShader::GlShader(ShaderDesc shaderDesc):
		m_shaderDesc(shaderDesc) {}

	GlShader::~GlShader() {
		ASSERT(m_pointer != 0);
		ASSERT(m_pixelShaderPtr != 0);
		ASSERT(m_vertexShaderPtr != 0);
		glDeleteProgram(m_pointer);
		glDeleteShader(m_pixelShaderPtr);
		glDeleteShader(m_vertexShaderPtr);
		m_pointer = 0;
		m_pixelShaderPtr = 0;
		m_vertexShaderPtr = 0;
	}

	GlDevice::GlDevice() {
		LOG_INFO("OpenGL Version: {}", std::string((char*)glGetString(GL_VERSION)));
		LOG_INFO("GLSL Version: {}", std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
		LOG_INFO("Available extensions:");

		// Query extensions
		GLint numExtensions = 0;
		GL_CHECK(glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions));
		m_openGlExtensions.reserve(numExtensions);

		for (int i = 0; i < numExtensions; i++) {
			const char* currentExtension = (const char*)glGetStringi(GL_EXTENSIONS, i);
			GL_CHECK(;);
			m_openGlExtensions.push_back(currentExtension);
			LOG_INFO("    {}", currentExtension);
		}

		// Tell stbi to flip textures to conform with OpenGL correctly
		stbi_set_flip_vertically_on_load(true);

		// Enable depth testing
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		// This engine uses reverse Z-buffer for improved accuracy. Flip depth buffer range.
		// Remap 0-1 to depth range
		GL_CHECK(glDepthRange(0.0f, 1.0f));

		// 0-1 depth range
		// only enable if GL_ARB_clip_control is available
		if (isExtensionAvailable("GL_ARB_clip_control")) {
			GL_CHECK(glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE));
		}

		// Enable MSAA
		GL_CHECK(glEnable(GL_MULTISAMPLE));
		GL_CHECK(glDisable(GL_BLEND));
	}

	GlDevice::~GlDevice() {

	}

	const bool GlDevice::isExtensionAvailable(const std::string& extensionName) const {
		return (std::find(m_openGlExtensions.begin(), m_openGlExtensions.end(), extensionName)) != m_openGlExtensions.end();
	}

	void GlDevice::setViewport(const Rect viewportRect) {
		GL_CHECK(glViewport(viewportRect.left, viewportRect.top, viewportRect.getWidth(), viewportRect.getHeight()));
	}

	InputLayoutHandle GlDevice::createInputLayout(const VertexAttributeDesc* desc, uint32_t attributeCount) {

		// Create native data
		GLuint vertexArray = 0;
		GL_CHECK(glGenVertexArrays(1, &vertexArray));
		GL_CHECK(glBindVertexArray(vertexArray));
		for (uint32_t i = 0; i < attributeCount; i++) {

			ASSERT(desc[i].format != GpuFormat::Unknown);

			auto formatData = gpu::gl::getGlFormat(desc[i].format);
			GL_CHECK(glVertexAttribPointer(
				desc[i].bufferIndex,
				static_cast<GLint>(formatData.size),
				formatData.glType,
				formatData.glNormalised,
				desc[i].elementStride,
				(void*) desc[i].offset));

			GL_CHECK(glEnableVertexAttribArray(desc[i].bufferIndex));
		}

		// Copy attributes
		GlInputLayout* inputLayout = new GlInputLayout();
		inputLayout->attributes.resize(attributeCount);
		for (uint32_t i = 0; i < attributeCount; i++) {
			inputLayout->attributes[0] = desc[0];
		}
		inputLayout->m_pointer = vertexArray;

		return InputLayoutHandle::Create(inputLayout);
	}

	ShaderHandle GlDevice::makeShader(const ShaderDesc shaderDesc) {

		// @TODO: Shader stages should be optional
		// @TODO: Re-generate glad to support geo, tess and compute shaders
		ASSERT(shaderDesc.VS.byteCode != nullptr);
		ASSERT(shaderDesc.VS.byteCode[0] != 0);
		ASSERT(shaderDesc.PS.byteCode != nullptr);
		ASSERT(shaderDesc.PS.byteCode[0] != 0);

		GLuint vertexShader = 0;
		GLuint pixelShader = 0;
		GLuint shaderProgram = 0;

		int  success = 0;
		char infoLog[4096] = {};

		// We prepend a common "global" header into every shader before passing them into the shader compiler.
		// Also we resolve #include pragma directives, so that we can better modularise the code.
		// This is done in the asset manager however, which isn't ideal

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GL_CHECK(;);
		GL_CHECK(glShaderSource(vertexShader, 1, (const GLchar**)&shaderDesc.VS.byteCode, NULL));
		GL_CHECK(glCompileShader(vertexShader));
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
			LOG_FATAL("[GL]: Failed to compile vertex shader. Got:\n{0}", infoLog);

			return ShaderHandle::Create(nullptr);
		}

		pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
		GL_CHECK(;);
		GL_CHECK(glShaderSource(pixelShader, 1, (const GLchar**)&shaderDesc.PS.byteCode, NULL));
		GL_CHECK(glCompileShader(pixelShader));
		glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(pixelShader, sizeof(infoLog), NULL, infoLog);
			LOG_FATAL("[GL]: Failed to compile pixel shader. Got:\n{0}", infoLog);

			return ShaderHandle::Create(nullptr);
		}

		shaderProgram = glCreateProgram();
		GL_CHECK(glAttachShader(shaderProgram, vertexShader));
		GL_CHECK(glAttachShader(shaderProgram, pixelShader));
		GL_CHECK(glLinkProgram(shaderProgram));
		GL_CHECK(glUseProgram(shaderProgram));

#if _DEBUG
		if (!shaderDesc.debugName.empty()) {
			GL_CHECK(glObjectLabel(GL_SHADER, vertexShader, -1, fmt::format("{}_vertexShader", shaderDesc.debugName).c_str()));
			GL_CHECK(glObjectLabel(GL_SHADER, pixelShader, -1, fmt::format("{}_pixelShader", shaderDesc.debugName).c_str()));
			GL_CHECK(glObjectLabel(GL_PROGRAM, shaderProgram, -1, shaderDesc.debugName.c_str()));
		}
#endif

		GlShader* shader = new GlShader(shaderDesc);
		shader->m_pointer = shaderProgram;
		shader->m_vertexShaderPtr = vertexShader;
		shader->m_pixelShaderPtr = pixelShader;

		return ShaderHandle::Create(shader);
	}

	BufferHandle GlDevice::makeBuffer(const BufferDesc bufferDesc) {

		GLuint glBuffer = 0;
		GL_CHECK(glGenBuffers(1, &glBuffer));
		GL_CHECK(glBindBuffer(getGlBufferType(bufferDesc.type).glType, glBuffer));

#if _DEBUG
		if (!bufferDesc.debugName.empty()) {
			GL_CHECK(glObjectLabel(GL_BUFFER, glBuffer, -1, bufferDesc.debugName.c_str()));
		}
#endif

		GlBuffer* buffer = new GlBuffer(bufferDesc);
		buffer->m_pointer = glBuffer;

		// Bind buffer internally
		m_currentBuffers[(uint32_t)bufferDesc.type] = buffer->m_pointer;

		return BufferHandle::Create(buffer);
	}

	void GlDevice::bindShader(IShader* shader) {
		ASSERT(shader != nullptr);

		// shader program
		GL_CHECK(glUseProgram(shader->getNativeObject()));

		// culling mode
		switch (shader->getDesc().graphicsState.faceCullingMode) {
			case gpu::FaceCullMode::Never:
			{
				GL_CHECK(glDisable(GL_CULL_FACE));
				break;
			}
			case gpu::FaceCullMode::Back:
			{
				GL_CHECK(glEnable(GL_CULL_FACE));
				GL_CHECK(glCullFace(GL_BACK));
				break;
			}
			case gpu::FaceCullMode::Front:
			{
				GL_CHECK(glEnable(GL_CULL_FACE));
				GL_CHECK(glCullFace(GL_FRONT));
				break;
			}
			case gpu::FaceCullMode::Both:
			{
				GL_CHECK(glEnable(GL_CULL_FACE));
				GL_CHECK(glCullFace(GL_FRONT_AND_BACK));
				break;
			}
		}

		// winding order
		GL_CHECK(glFrontFace(shader->getDesc().graphicsState.faceWindingOrder == gpu::WindingOrder::Clockwise ? GL_CW : GL_CCW));

		// depth state
		if (shader->getDesc().graphicsState.depthTest) {
			GL_CHECK(glEnable(GL_DEPTH_TEST));
		} else {
			GL_CHECK(glDisable(GL_DEPTH_TEST));
		}
		GL_CHECK(glDepthMask(shader->getDesc().graphicsState.depthWrite == true ? GL_TRUE : GL_FALSE));
		GL_CHECK(glDepthFunc(getGlDepthFunc(shader->getDesc().graphicsState.depthState).glEnum));
	}

	void GlDevice::draw(DrawCallState drawCallState, size_t triangleCount, size_t offset) {
		ASSERT(drawCallState.vertexBufer != nullptr);
		ASSERT(drawCallState.indexBuffer == nullptr);
		ASSERT(drawCallState.shader != nullptr);
		ASSERT(drawCallState.vertexLayout != nullptr);
		ASSERT(triangleCount > 0);
		ASSERT(drawCallState.primitiveType != PrimitiveType::Count);

		// associated vertex layout
		GL_CHECK(glBindVertexArray(drawCallState.vertexLayout->getNativeObject()));

		// Bind shader
		bindShader(drawCallState.shader);

		// Bind vertex buffer
		bindBuffer(drawCallState.vertexBufer);
		// Unbind index buffer
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		// Set BlendState
		if (drawCallState.blendState != nullptr) {
			bindBlendState(drawCallState.blendState);
		}

		auto primitiveType = getGlPrimitiveType(drawCallState.primitiveType);

		// Issue draw call
		GL_CHECK(glDrawArrays(primitiveType.glType, static_cast<GLint>(triangleCount) * 3U /* OpenGL expects number of vertices here, not tris */, static_cast<GLsizei>(offset)));
	}
	void GlDevice::drawIndexed(DrawCallState drawCallState, size_t triangleCount, size_t offset) {
		ASSERT(drawCallState.vertexBufer != nullptr);
		ASSERT(drawCallState.indexBuffer != nullptr);
		ASSERT(drawCallState.shader != nullptr);
		ASSERT(drawCallState.vertexLayout != nullptr);
		ASSERT(triangleCount > 0);
		ASSERT(drawCallState.primitiveType != PrimitiveType::Count);

		// associated vertex layout
		GL_CHECK(glBindVertexArray(drawCallState.vertexLayout->getNativeObject()));

		// Bind shader
		bindShader(drawCallState.shader);

		// Bind vertex buffer
		bindBuffer(drawCallState.vertexBufer);
		// Bind index buffer
		bindBuffer(drawCallState.indexBuffer);

		// Set BlendState
		if (drawCallState.blendState != nullptr) {
			bindBlendState(drawCallState.blendState);
		}

		auto primitiveType = getGlPrimitiveType(drawCallState.primitiveType);
		auto indexFormat = getGlFormat(drawCallState.indexBuffer->getDesc().format);

		// Issue draw call
		GL_CHECK(glDrawElements(primitiveType.glType, static_cast<GLsizei>(triangleCount) * 3U /* OpenGL expects number of indices here, not tris */, indexFormat.glType, reinterpret_cast<void*>(offset)));
	}

	void GlDevice::clearColor(Color color, float depth) {

		if (m_clearColor.r != color.r || m_clearColor.g != color.g || m_clearColor.b != color.b || m_clearColor.a != color.a) {
			GL_CHECK(glClearColor(color.r, color.g, color.b, color.a));
			m_clearColor = color;
		}
		if (m_depth != depth) {
			GL_CHECK(glClearDepth(depth));
			m_depth = depth;
		}
		// clear colour and depth buffer
		GL_CHECK(glDepthMask(GL_TRUE));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	void GlDevice::present() {
		GL_CHECK(glFlush());
	}

	BlendStateHandle GlDevice::makeBlendState(const BlendStateDesc blendStateDesc) {
		// This is just POD in OpenGL
		GlBlendState* blendState = new GlBlendState(blendStateDesc);
		return BlendStateHandle::Create(blendState);
	}

	void GlDevice::bindBlendState(IBlendState* blendState) {
		ASSERT(blendState != nullptr);
		gpu::BlendStateDesc desc = blendState->getDesc();

		if (desc.blendEnable) {
			GL_CHECK(glEnable(GL_BLEND));
			// Blend factors
			GLenum srcRGB = getGlBlendFactor(desc.srcFactor).glEnum;
			GLenum srcAlpha = getGlBlendFactor(desc.srcFactorAlpha).glEnum;
			GLenum dstRGB = getGlBlendFactor(desc.dstFactor).glEnum;
			GLenum dstAlpha = getGlBlendFactor(desc.dstFactorAlpha).glEnum;
			GL_CHECK(glBlendFuncSeparate(srcRGB, srcAlpha, dstRGB, dstAlpha));

			GLenum eqnRGB = getGlBlendOp(desc.blendOp).glEnum;
			GLenum eqnAlpha = getGlBlendOp(desc.blendOpAlpha).glEnum;
			GL_CHECK(glBlendEquationSeparate(eqnRGB, eqnAlpha));
		} else {
			GL_CHECK(glDisable(GL_BLEND));
		}
	}

	void GlDevice::bindTexture(ITexture* texture, ITextureSampler* sampler, uint32_t index) {
		ASSERT(texture != nullptr);
		ASSERT(sampler != nullptr);
		ASSERT(index < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

		GL_CHECK(glActiveTexture(GL_TEXTURE0 + index)); // we need to offset the binding offset for texture 0 with the user supplied index
		GL_CHECK(glBindTexture(getGlTextureType(texture->getDesc().type).glEnum, texture->getNativeObject()));
		GL_CHECK(glBindSampler(index, sampler->getNativeObject()));
	}

	void GlDevice::bindTexture(IFramebuffer* texture, ITextureSampler* sampler, uint32_t index) {
		ASSERT(texture != nullptr);
		ASSERT(sampler != nullptr);
		ASSERT(index < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

		GL_CHECK(glActiveTexture(GL_TEXTURE0 + index)); // we need to offset the binding offset for texture 0 with the user supplied index
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture->getTextureNativeObject()));
		GL_CHECK(glBindSampler(index, sampler->getNativeObject()));
	}

	void GlDevice::bindFramebuffer(IFramebuffer* texture) {
		GL_CHECK(glBindFramebuffer(GL_RENDERBUFFER, texture != nullptr ? texture->getNativeObject() : 0));
	}

	void GlDevice::blitFramebuffer(IFramebuffer* textureSrc, IFramebuffer* textureDst) {
		ASSERT(textureSrc != nullptr);

		GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, textureSrc->getNativeObject()));
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, textureDst != nullptr ? textureDst->getNativeObject() : 0));
		GL_CHECK(glBlitFramebuffer(
			// src rect (x, y, width, height)
			0,
			0,
			textureSrc->getDesc().colorDesc.width,
			textureSrc->getDesc().colorDesc.height,
			
			// dest rect (x, y, width, height)
			0,
			0,
			(textureDst != nullptr ? textureDst->getDesc().colorDesc.width : textureSrc->getDesc().colorDesc.width),
			(textureDst != nullptr ? textureDst->getDesc().colorDesc.height : textureSrc->getDesc().colorDesc.height),
			
			GL_COLOR_BUFFER_BIT, GL_NEAREST));
	}

	void GlDevice::debugMarkerPush(const std::string& title) {
#if _DEBUG
		GL_CHECK(glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION, 0, -1, title.c_str()));
#endif
	}
	void GlDevice::debugMarkerPop() {
#if _DEBUG
		GL_CHECK(glPopDebugGroupKHR());
#endif
	}
} 