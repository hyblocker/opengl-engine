#pragma once

#include <inttypes.h>
#include <string>
#include <memory>

#include "engine/refcounter.hpp"
#include "itypes.hpp"
#include "itexture.hpp"
#include "ibuffer.hpp"

namespace gpu {

	struct GraphicsState {
		CompareFunc depthState = CompareFunc::GreaterOrEqual;
		bool depthWrite = true;
		bool depthTest = true;
		FaceCullMode faceCullingMode = FaceCullMode::Back;
		WindingOrder faceWindingOrder = WindingOrder::CounterClockwise;
	};

	struct ShaderProgram {
		// In the case of OpenGL, this points to a glsl file on disk
		uint8_t* byteCode = nullptr;
		std::string entryFunc = "main";
	};

	struct ShaderDesc {
		ShaderProgram VS;
		ShaderProgram PS;
		GraphicsState graphicsState = {
			.depthState = CompareFunc::GreaterOrEqual,
			.depthWrite = true,
			.faceCullingMode = FaceCullMode::Back,
			.faceWindingOrder = WindingOrder::CounterClockwise,
		};
		std::string debugName = "";
	};

	class IShader {
	public:
		IShader() = default;
		virtual ~IShader() = default;
		[[nodiscard]] virtual const ShaderDesc& getDesc() const = 0;
		[[nodiscard]] virtual const GpuPtr getNativeObject() const = 0;
	};

	typedef engine::RefCounter<IShader> ShaderHandle;

	struct BlendStateDesc {
		bool blendEnable = false;
		BlendFactor srcFactor = BlendFactor::One;
		BlendFactor dstFactor = BlendFactor::Zero;
		BlendOp blendOp = BlendOp::Add;
		BlendFactor srcFactorAlpha = BlendFactor::One;
		BlendFactor dstFactorAlpha = BlendFactor::Zero;
		BlendOp blendOpAlpha = BlendOp::Add;
	};

	class IBlendState {
	public:
		IBlendState() = default;
		virtual ~IBlendState() = default;
		[[nodiscard]] virtual const BlendStateDesc& getDesc() const = 0;
		[[nodiscard]] virtual const GpuPtr getNativeObject() const = 0;
	};
	typedef engine::RefCounter<IBlendState> BlendStateHandle;

	struct DrawCallState {
		// Buffers to draw
		IBuffer* vertexBufer = nullptr;
		IBuffer* indexBuffer = nullptr;

		// Shader to draw and it's associated vertex layout
		IShader* shader = nullptr;

		// Because OpenGL is horrible and vertex layout is tied to the vertex buffer (whats the point of even binding a vertex buffer then??)
		IInputLayout* vertexLayout = nullptr;

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
		// size is in bytes
		virtual void writeBuffer(IBuffer* handle, size_t size, const  void* data) = 0;
		virtual void mapBuffer(IBuffer* buffer, uint32_t offset, size_t length, MapAccessFlags accessFlags, void** mappedDataPtr) = 0;
		virtual void unmapBuffer(IBuffer* buffer) = 0;
		virtual void bindBuffer(IBuffer* buffer) = 0;
		// Buffer must be a constant buffer. Will ASSERT otherwise.
		virtual void setConstantBuffer(IBuffer* buffer, uint32_t bindIndex) = 0;
		virtual void unbindConstantBuffer(IBuffer* buffer, uint32_t bindIndex) = 0;
		virtual void unbindBuffer(IBuffer* buffer) = 0;
		virtual void setBufferBinding(IShader* shader, const std::string& name, uint32_t bindIndex) = 0;

		virtual void draw(DrawCallState drawState, size_t triangleCount, size_t offset = 0, size_t instances = 1) = 0;
		virtual void drawIndexed(DrawCallState drawState, size_t triangleCount, size_t offset = 0, size_t instances = 1) = 0;

		virtual void clearColor(Color color, float depth = 0.0f) = 0;
		virtual void present() = 0;

		// Blend state
		virtual BlendStateHandle makeBlendState(const BlendStateDesc blendStateDesc) = 0;
		virtual void bindBlendState(IBlendState* blendState) = 0;

		// Textures
		// Assumes RGBA data in textureData
		virtual TextureHandle makeTexture(TextureDesc desc, void* textureData) = 0;
		virtual TextureSamplerHandle makeTextureSampler(TextureSamplerDesc desc) = 0;
		virtual void bindTexture(ITexture* texture, ITextureSampler* sampler, uint32_t index = 0) = 0;
		virtual void bindTexture(IFramebuffer* texture, ITextureSampler* sampler, uint32_t index = 0) = 0;

		// Framebuffers
		virtual FramebufferHandle makeFramebuffer(FramebufferDesc desc) = 0;
		virtual void bindFramebuffer(IFramebuffer* texture) = 0;
		virtual void blitFramebuffer(IFramebuffer* textureSrc, IFramebuffer* textureDst) = 0;

		virtual void debugMarkerPush(const std::string& title) = 0;
		virtual void debugMarkerPop() = 0;
	};

	typedef engine::RefCounter<IDevice> DeviceHandle;
}