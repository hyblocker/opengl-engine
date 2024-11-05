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

	void GlDevice::draw(DrawCallState drawCallState) {

	}
	void GlDevice::drawIndexed(DrawCallState drawCallState) {
		// Bind vertex buffer
		bindBuffer(drawCallState.vertexBufer);
		// Bind index buffer
		bindBuffer(drawCallState.indexBuffer);

		// Show how to interpret vertex format attributes
		glUseProgram(drawCallState.shader->getNativeObject());
		glBindVertexArray(drawCallState.shader->getDesc().graphicsState.vertexLayout->getNativeObject());

		// Draw
		// DrawIndexed
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, 0);
	}
} 