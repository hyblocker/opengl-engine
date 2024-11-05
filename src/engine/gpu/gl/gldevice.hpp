#include "engine/gpu/idevice.hpp"

namespace gpu::gl {

	class GlDevice;

	class GlBuffer : public gpu::IBuffer {
	public:
		GlBuffer(gpu::BufferDesc bufferDesc);
		~GlBuffer();

		[[nodiscard]] inline const BufferDesc& getDesc() const override { return m_bufferDesc; }
		[[nodiscard]] inline const uint32_t getNativeObject() const override { return m_pointer; }
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

		[[nodiscard]] inline const uint32_t getNativeObject() const override { return m_pointer; }
		// Inherited:
		//	   std::vector<VertexAttributeDesc> attributes;
	private:
		uint32_t m_pointer = 0;

		friend class gpu::gl::GlDevice;
	};

	//
	// Textures
	//
	

	//
	// Shaders
	//
	class GlShader : public gpu::IShader {
	public:
		GlShader(ShaderDesc shaderDesc);
		~GlShader();
		[[nodiscard]] inline const ShaderDesc& getDesc() const override { return m_shaderDesc; }
		[[nodiscard]] inline const uint32_t getNativeObject() const override { return m_pointer; }
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
	// Device
	//
	class GlDevice : public ::gpu::IDevice {
	public:
		~GlDevice() override;

		void setViewport(const Rect viewportRect) override;
		InputLayoutHandle createInputLayout(const VertexAttributeDesc* desc, uint32_t attributeCount) override;
		ShaderHandle makeShader(const ShaderDesc shaderDesc) override;
		BufferHandle makeBuffer(const BufferDesc bufferDesc) override;
		void writeBuffer(IBuffer* handle, size_t size, const void* data) override;

		void draw(DrawCallState drawCallState, size_t elementCount, size_t offset = 0) override;
		void drawIndexed(DrawCallState drawCallState, size_t elementCount, size_t offset = 0) override;

		void clearColor(Color color) override;
		void present() override;

	private:
		// Binds a buffer if necessary
		void bindBuffer(IBuffer* handle);

	private:
		uint32_t m_currentBuffers[(uint32_t)gpu::BufferType::Count] = {};
		Color m_clearColor = {};
	};
}