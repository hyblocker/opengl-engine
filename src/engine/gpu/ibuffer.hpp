#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

#include "engine/refcounter.hpp"
#include "itypes.hpp"

namespace gpu {

	enum class Usage {
		// GPU has read write access to the resource
		Default,
		// Can only be written to on intialisation, and is read-only on the GPU
		Immutable,
		// Read-only on the GPU, write-only on the CPU
		Dynamic,
		// Resource may be copied from the GPU to the CPU
		Staging,
		Count,
	};

	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferData.xhtml
	enum class BufferType {
		VertexBuffer,
		BufferCopySource,
		BufferCopyDestination,
		IndexBuffer,
		PixelReadTarget,
		TextureDataSource,
		TransformFeedbackBuffer,
		ConstantBuffer,
		Count,
	};

	struct BufferDesc {
		gpu::BufferType type = gpu::BufferType::ConstantBuffer;
		gpu::Usage usage = gpu::Usage::Default;
		gpu::GpuFormat format = gpu::GpuFormat::Unknown;
	};

	class IBuffer {
	public:
		[[nodiscard]] virtual const BufferDesc& getDesc() const = 0;
		[[nodiscard]] virtual const uint32_t getNativeObject() const = 0;
	};

	typedef engine::RefCounter<IBuffer> BufferHandle;

	struct VertexAttributeDesc {
	public:
		std::string name;
		gpu::GpuFormat format = gpu::GpuFormat::Unknown;
		uint32_t arraySize = 1;
		uint32_t bufferIndex = 0;
		size_t offset = 0;
		// Must be the same between all elements
		uint32_t elementStride = 0;
	};

	class IInputLayout {
	public:
		IInputLayout() = default;
		virtual ~IInputLayout() = default;
		std::vector<VertexAttributeDesc> attributes;
		[[nodiscard]] virtual const uint32_t getNativeObject() const = 0;
	};

	typedef engine::RefCounter<IInputLayout> InputLayoutHandle;
}