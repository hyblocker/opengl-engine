#include "engine/gpu/idevice.hpp"
#include "engine/log.hpp"

namespace gpu::gl {

	// Macro to get errors from OpenGL at call sites
	// Similar in design to the VK_CHECK macro, from https://vkguide.dev/docs/chapter-1/vulkan_init_code/
#if _DEBUG
	#define GL_CHECK(x) \
		do { \
			x; \
			GLenum err; \
			while ((err = glGetError()) != GL_NO_ERROR) { \
				LOG_ERROR("GL: {}", #x); \
				abort(); \
			} \
		} while (0)
#else
	#define GL_CHECK(x) x;
#endif


	class GlDevice;

	class GlBuffer : public gpu::IBuffer {
	public:
		GlBuffer(gpu::BufferDesc bufferDesc);
		~GlBuffer();

		[[nodiscard]] inline const BufferDesc& getDesc() const override { return m_bufferDesc; }
		[[nodiscard]] inline const GpuPtr getNativeObject() const override { return m_pointer; }
	private:
		gpu::BufferDesc m_bufferDesc;
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};
	
	// 
	// Buffers
	//
	class GlInputLayout : public gpu::IInputLayout {
	public:
		GlInputLayout();
		~GlInputLayout() override;

		[[nodiscard]] inline const GpuPtr getNativeObject() const override { return m_pointer; }
		// Inherited:
		//	   std::vector<VertexAttributeDesc> attributes;
	private:
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};

	//
	// Textures
	//
	class GlTexture : public gpu::ITexture {
	public:
		~GlTexture() override;
		[[nodiscard]] const TextureDesc getDesc() const override;
		[[nodiscard]] const GpuPtr getNativeObject() const override;
	private:
		TextureDesc m_desc;
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};

	class GlTextureSampler : public gpu::ITextureSampler {
	public:
		~GlTextureSampler() override;
		[[nodiscard]] const TextureSamplerDesc& getDesc() const override;
		[[nodiscard]] const GpuPtr getNativeObject() const override;
	private:
		TextureSamplerDesc m_desc;
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};

	class GlFramebuffer : public gpu::IFramebuffer {
	public:
		~GlFramebuffer() override;
		[[nodiscard]] const FramebufferDesc& getDesc() const override;
		[[nodiscard]] const GpuPtr getNativeObject() const override;
		[[nodiscard]] const GpuPtr getTextureNativeObject() const override;
	private:
		FramebufferDesc m_desc;
		uint32_t m_textureAttachment = 0;
		uint32_t m_depthStencilAttachment = 0;
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};

	//
	// Shaders
	//
	class GlShader : public gpu::IShader {
	public:
		GlShader(ShaderDesc shaderDesc);
		~GlShader();
		[[nodiscard]] inline const ShaderDesc& getDesc() const override { return m_shaderDesc; }
		[[nodiscard]] inline const GpuPtr getNativeObject() const override { return m_pointer; }
		// Inherited:
		//	   uint32_t pointer;
		//     uint32_t vertexShaderPtr;
		//     uint32_t pixelShaderPtr;
	private:
		ShaderDesc m_shaderDesc;
		uint32_t m_pointer = 0;
		uint32_t m_vertexShaderPtr = 0;
		uint32_t m_pixelShaderPtr = 0;

		friend class gpu::gl::GlDevice;
	};

	//
	// BlendState
	//
	class GlBlendState : public gpu::IBlendState {
	public:
		GlBlendState(BlendStateDesc blendStateDesc) : m_blendStateDesc(blendStateDesc) {}
		~GlBlendState() = default;
		[[nodiscard]] inline const BlendStateDesc& getDesc() const override { return m_blendStateDesc; }
		[[nodiscard]] inline const GpuPtr getNativeObject() const override { return -1; }

	private:
		BlendStateDesc m_blendStateDesc;
	};

	//
	// Device
	//
	class GlDevice : public ::gpu::IDevice {
	public:
		GlDevice();
		~GlDevice() override;

		void setViewport(const Rect viewportRect) override;
		InputLayoutHandle createInputLayout(const VertexAttributeDesc* desc, uint32_t attributeCount) override;
		ShaderHandle makeShader(const ShaderDesc shaderDesc) override;

		BufferHandle makeBuffer(const BufferDesc bufferDesc) override;
		void writeBuffer(IBuffer* handle, size_t size, const void* data) override;
		void mapBuffer(IBuffer* buffer, uint32_t offset, size_t length, MapAccessFlags accessFlags, void** mappedDataPtr) override;
		void unmapBuffer(IBuffer* buffer) override;
		void bindBuffer(IBuffer* buffer) override;
		void unbindBuffer(IBuffer* buffer) override;
		void setConstantBuffer(IBuffer* buffer, uint32_t bindIndex) override;
		void unbindConstantBuffer(IBuffer* buffer, uint32_t bindIndex) override;
		void setBufferBinding(IShader* shader, const std::string& name, uint32_t bindIndex) override;

		void draw(DrawCallState drawCallState, size_t triangleCount, size_t offset = 0, size_t instances = 1) override;
		void drawIndexed(DrawCallState drawCallState, size_t triangleCount, size_t offset = 0, size_t instances = 1) override;

		void clearColor(Color color, float depth) override;
		void present() override;

		// Blend state
		BlendStateHandle makeBlendState(const BlendStateDesc blendStateDesc) override;
		void bindBlendState(IBlendState* blendState) override;

		// Textures
		TextureHandle makeTexture(TextureDesc desc, void* textureData) override;
		TextureSamplerHandle makeTextureSampler(TextureSamplerDesc desc) override;
		void bindTexture(ITexture* texture, ITextureSampler* sampler, uint32_t index = 0) override;
		void bindTexture(IFramebuffer* texture, ITextureSampler* sampler, uint32_t index = 0) override;

		FramebufferHandle makeFramebuffer(FramebufferDesc desc) override;
		void bindFramebuffer(IFramebuffer* texture) override; 
		void blitFramebuffer(IFramebuffer* textureSrc, IFramebuffer* textureDst) override;

		void debugMarkerPush(const std::string& title) override;
		void debugMarkerPop() override;

	private:
		void bindShader(IShader* shader);
		const bool isExtensionAvailable(const std::string& extensionName) const;

		uint32_t m_boundShader = -1;
		uint32_t m_currentBuffers[(uint32_t)gpu::BufferType::Count] = {};
		Color m_clearColor = {};
		float m_depth = 0xFFFFFFFF;

		std::vector<std::string> m_openGlExtensions;
		int32_t m_maxUniformBufferBindings = 0;
		int32_t m_maxCombinedTextureImageUnits = 0;
		int32_t m_maxUniformBufferBlockSize = 0;
		float m_maxTextureMaxAnisotropyExt = 0;
	};
}