#pragma once

#include <inttypes.h>
#include <string>
#include <memory>

#include "engine/refcounter.hpp"
#include "itypes.hpp"
#include "itexture.hpp"
#include "ibuffer.hpp"

namespace gpu {

	enum class COMPARE : uint8_t {
		NEVER = 1,
		LESS = 2,
		EQUAL = 3,
		LESS_OR_EQUAL = 4,
		GREATER = 5,
		NOT_EQUAL = 6,
		GREATER_OR_EQUAL = 7,
		ALWAYS = 8,
	};

	struct GraphicsState {
		COMPARE depthState = COMPARE::GREATER_OR_EQUAL;
		IInputLayout* vertexLayout;
	};

	struct ShaderProgram {
		uint8_t* byteCode = nullptr;
		std::string entryFunc = "main";
	};

	struct ShaderDesc {
		std::string debugName;
		ShaderProgram VS;
		ShaderProgram PS;
		GraphicsState graphicsState = {
			.depthState = COMPARE::GREATER_OR_EQUAL
		};
	};

	class IShader {
	public:
		IShader() = default;
		virtual ~IShader() = default;
		[[nodiscard]] virtual const ShaderDesc& getDesc() const = 0;
		[[nodiscard]] virtual const uint32_t getNativeObject() const = 0;
	};

	typedef engine::RefCounter<IShader> ShaderHandle;

	class ITextureSampler {
		// @TODO: AAAAAAAAAAA
	};
	typedef engine::RefCounter<ITextureSampler*> TextureSamplerHandle;

	class IBlendState {
		// @TODO: AAAAAAAAAAAA
	};
	typedef engine::RefCounter<IBlendState*> BlendStateHandle;

	struct DrawCallState {
		// Buffers to draw
		IBuffer* vertexBufer = nullptr;
		IBuffer* indexBuffer = nullptr;

		// Shader to draw and it's associated vertex layout
		IShader* shader = nullptr;

		// Blending properties
		IBlendState* blendState = nullptr;

		PrimitiveType primitiveType = PrimitiveType::Triangles;
	};

	class IDevice {
	public:
		explicit IDevice() {}
		virtual ~IDevice() {};

		virtual void setViewport(const Rect viewportRect) = 0;
		virtual InputLayoutHandle createInputLayout(const VertexAttributeDesc* desc, uint32_t attributeCount) = 0;
		virtual ShaderHandle makeShader(const ShaderDesc shaderDesc) = 0;

		virtual BufferHandle makeBuffer(const BufferDesc bufferDesc) = 0;
		virtual void writeBuffer(IBuffer* handle, size_t size, const  void* data) = 0;
		virtual void mapBuffer(IBuffer* buffer, uint32_t offset, size_t length, MapAccessFlags accessFlags, void** mappedDataPtr) = 0;
		virtual void unmapBuffer(IBuffer* buffer) = 0;
		virtual void bindBuffer(IBuffer* buffer) = 0;
		virtual void bindBufferBinding(IBuffer* buffer, uint32_t bindIndex) = 0;
		virtual void unbindBuffer(IBuffer* buffer) = 0;
		virtual void setBufferBinding(IShader* shader, const std::string& name, uint32_t bindIndex) = 0;

		virtual void draw(DrawCallState drawState, size_t elementCount, size_t offset = 0) = 0;
		virtual void drawIndexed(DrawCallState drawState, size_t elementCount, size_t offset = 0) = 0;

		virtual void clearColor(Color color) = 0;
		virtual void present() = 0;
	};

	typedef engine::RefCounter<IDevice> DeviceHandle;
}