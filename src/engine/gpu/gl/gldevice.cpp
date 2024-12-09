#include <fmt/format.h>
#include <stb_image.h>

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

		// Tell stbi to flip textures to conform with OpenGL correctly
		stbi_set_flip_vertically_on_load(true);

		// Enable depth testing
		glEnable(GL_DEPTH_TEST);
	}

	GlDevice::~GlDevice() {

	}

	void GlDevice::setViewport(const Rect viewportRect) {
		glViewport(viewportRect.left, viewportRect.top, viewportRect.getWidth(), viewportRect.getHeight());
	}

	InputLayoutHandle GlDevice::createInputLayout(const VertexAttributeDesc* desc, uint32_t attributeCount) {

		// Create native data
		GLuint vertexArray = 0;
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		for (uint32_t i = 0; i < attributeCount; i++) {

			ASSERT(desc[i].format != GpuFormat::Unknown);

			auto formatData = gpu::gl::getGlFormat(desc[i].format);
			glVertexAttribPointer(
				desc[i].bufferIndex,
				static_cast<GLint>(formatData.size),
				formatData.glType,
				formatData.glNormalised,
				desc[i].elementStride,
				reinterpret_cast<void*>(desc[i].offset));

			glEnableVertexAttribArray(desc[i].bufferIndex);
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
		ASSERT(shaderDesc.graphicsState.vertexLayout != nullptr);

		GLuint vertexShader = 0;
		GLuint pixelShader = 0;
		GLuint shaderProgram = 0;

		int  success = 0;
		char infoLog[4096] = {};

		// We prepend a common "global" header into every shader before passing them into the shader compiler.
		// Also we resolve #include pragma directives, so that we can better modularise the code

		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, (const GLchar**)&shaderDesc.VS.byteCode, NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, sizeof(infoLog), NULL, infoLog);
			LOG_FATAL("[GL]: Failed to compile vertex shader. Got:\n{0}", infoLog);

			return ShaderHandle::Create(nullptr);
		}

		pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(pixelShader, 1, (const GLchar**)&shaderDesc.PS.byteCode, NULL);
		glCompileShader(pixelShader);
		glGetShaderiv(pixelShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(pixelShader, sizeof(infoLog), NULL, infoLog);
			LOG_FATAL("[GL]: Failed to compile pixel shader. Got:\n{0}", infoLog);

			return ShaderHandle::Create(nullptr);
		}

		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, pixelShader);
		glLinkProgram(shaderProgram);
		glUseProgram(shaderProgram);

		GlShader* shader = new GlShader(shaderDesc);
		shader->m_pointer = shaderProgram;
		shader->m_vertexShaderPtr = vertexShader;
		shader->m_pixelShaderPtr = pixelShader;

		return ShaderHandle::Create(shader);
	}

	BufferHandle GlDevice::makeBuffer(const BufferDesc bufferDesc) {

		GLuint glBuffer = 0;
		glGenBuffers(1, &glBuffer);
		glBindBuffer(getGlBufferType(bufferDesc.type).glType, glBuffer);

		GlBuffer* buffer = new GlBuffer(bufferDesc);
		buffer->m_pointer = glBuffer;

		// Bind buffer internally
		m_currentBuffers[(uint32_t)bufferDesc.type] = buffer->m_pointer;

		return BufferHandle::Create(buffer);
	}

	void GlDevice::bindShader(IShader* shader) {
		ASSERT(shader != nullptr);

		// shader program
		glUseProgram(shader->getNativeObject());
		// associated vertex layout
		glBindVertexArray(shader->getDesc().graphicsState.vertexLayout->getNativeObject());

		// culling mode
		switch (shader->getDesc().graphicsState.faceCullingMode) {
			case gpu::FaceCullMode::Never:
			{
				glDisable(GL_CULL_FACE);
				break;
			}
			case gpu::FaceCullMode::Back:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			}
			case gpu::FaceCullMode::Front:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			}
			case gpu::FaceCullMode::Both:
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT_AND_BACK);
				break;
			}
		}

		// winding order
		glFrontFace(shader->getDesc().graphicsState.faceWindingOrder == gpu::WindingOrder::Clockwise ? GL_CW : GL_CCW);

		// depth state
		glDepthFunc(getGlDepthFunc(shader->getDesc().graphicsState.depthState).glEnum);
		glDepthMask(shader->getDesc().graphicsState.depthWrite ? GL_TRUE : GL_FALSE);
	}

	void GlDevice::draw(DrawCallState drawCallState, size_t elementCount, size_t offset) {
		ASSERT(drawCallState.vertexBufer != nullptr);
		ASSERT(drawCallState.indexBuffer == nullptr);
		ASSERT(drawCallState.shader != nullptr);
		ASSERT(drawCallState.shader->getDesc().graphicsState.vertexLayout != nullptr);
		ASSERT(elementCount > 0);
		ASSERT(drawCallState.primitiveType != PrimitiveType::Count);

		// Bind vertex buffer
		bindBuffer(drawCallState.vertexBufer);
		// Unbind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// Bind shader
		bindShader(drawCallState.shader);

		auto primitiveType = getGlPrimitiveType(drawCallState.primitiveType);

		// Issue draw call
		glDrawArrays(primitiveType.glType, static_cast<GLint>(elementCount), static_cast<GLsizei>(offset));
	}
	void GlDevice::drawIndexed(DrawCallState drawCallState, size_t elementCount, size_t offset) {
		ASSERT(drawCallState.vertexBufer != nullptr);
		ASSERT(drawCallState.indexBuffer != nullptr);
		ASSERT(drawCallState.shader != nullptr);
		ASSERT(drawCallState.shader->getDesc().graphicsState.vertexLayout != nullptr);
		ASSERT(elementCount > 0);
		ASSERT(drawCallState.primitiveType != PrimitiveType::Count);

		// Bind vertex buffer
		bindBuffer(drawCallState.vertexBufer);
		// Bind index buffer
		bindBuffer(drawCallState.indexBuffer);

		// Bind shader
		bindShader(drawCallState.shader);

		auto primitiveType = getGlPrimitiveType(drawCallState.primitiveType);
		auto indexFormat = getGlFormat(drawCallState.indexBuffer->getDesc().format);

		// Issue draw call
		glDrawElements(primitiveType.glType, static_cast<GLsizei>(elementCount), indexFormat.glType, reinterpret_cast<void*>(offset));
	}

	void GlDevice::clearColor(Color color, float depth) {

		if (m_clearColor.r != color.r || m_clearColor.g != color.g || m_clearColor.b != color.b || m_clearColor.a != color.a) {
			glClearColor(color.r, color.g, color.b, color.a);
			m_clearColor = color;
		}
		if (m_depth != depth) {
			glClearDepthf(depth);
			m_depth = depth;
		}
		// clear colour and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GlDevice::present() {
		glFlush();
	}

	void GlDevice::bindTexture(ITexture* texture, ITextureSampler* sampler, uint32_t index) {
		ASSERT(texture != nullptr);
		ASSERT(sampler != nullptr);
		ASSERT(index < GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);

		glActiveTexture(GL_TEXTURE0 + index); // we need to offset the binding offset for texture 0 with the user supplied index
		glBindTexture(getGlTextureType(texture->getDesc().type).glEnum, texture->getNativeObject());
		glBindSampler(index, sampler->getNativeObject());
	}
} 