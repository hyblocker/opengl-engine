#pragma once

#include <inttypes.h>
#include <string>

namespace gpu {

	// Primitive types and enums

	enum class GpuFormat : uint8_t {
		Unknown,
		Uint8_TYPELESS,
		Uint16_TYPELESS,
		Uint32_TYPELESS,
		Int8_TYPELESS,
		Int16_TYPELESS,
		Int32_TYPELESS,
		R8_UNORM,
		RG8_UNORM,
		RGB8_UNORM,
		RGBA8_UNORM,
		R8_TYPELESS,
		RG8_TYPELESS,
		RGB8_TYPELESS,
		RGBA8_TYPELESS,
		Count,
	};

	enum class PrimitiveType {
		Triangles,
	};

	struct Rect {
		uint32_t left = 0;
		uint32_t right = 0;
		uint32_t top = 0;
		uint32_t bottom = 0;

	public:
		inline uint32_t getWidth() const { return right - left; }
		inline uint32_t getHeight() const { return bottom - top; }
	};
}