#pragma once

#include <inttypes.h>
#include <string>

namespace gpu {

	// Alias for native pointer to gpu memory, used to bind textures etc in cbuffers
	typedef uint32_t GpuPtr;

	// Macro taken from nvrhi
	// https://github.com/NVIDIAGameWorks/nvrhi/blob/656bb06d35f6fe92bafaeab368a2d65da9bc4b58/include/nvrhi/nvrhi.h#L34C1-L40C74
#define GPU_ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

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

	enum class PrimitiveType : uint8_t {
		Triangles,
		Points,
		Lines,
		Count,
	};

	enum class MapAccessFlags : uint8_t {
		Write = 0x00,
		Read = 0x01,
		// Unsupported in GLES3
		// Persistent = 0x02,
		// Coherent = 0x03,
	
		// These are bitwise flags
		InvalidateRange = 0x10,
		InvalidateBuffer = 0x20,
		FlushEexplicit = 0x40,
		Unsychronised = 0x80,
	};
	GPU_ENUM_CLASS_FLAG_OPERATORS(MapAccessFlags);

	struct Rect {
		uint32_t left = 0;
		uint32_t right = 0;
		uint32_t top = 0;
		uint32_t bottom = 0;

	public:
		inline uint32_t getWidth() const { return right - left; }
		inline uint32_t getHeight() const { return bottom - top; }
	};

	struct Color {
		float r;
		float g;
		float b;
		float a;
	};

	enum class CompareFunc : uint8_t {
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always,
		Count,
	};

	enum class FaceCullMode : uint8_t {
		Back,
		Front,
		// Culls both front and back faces
		Both,
		// Disables culling
		Never,
	};

	enum class WindingOrder : uint8_t {
		Clockwise = 0,
		CounterClockwise = 1,
	};

	enum class TextureType : uint8_t {
		// Texture1D,
		Texture2D,
		Texture3D,
		// TextureArray1D,
		TextureArray2D,
		// TextureRectangle,
		TextureCubeMap,
		// TextureArrayCubeMap,
		// TextureBuffer,
		// TextureMultisample2D,
		// TextureArrayMultisample2D,
		Count,
	};

	enum class SamplingMode : uint8_t {
		Nearest,
		Linear,
	};

	enum class TextureWrap : uint8_t {
		Repeat,
		MirrorRepeat,
		// MirrorClampToEdge,
		ClampToEdge,
		// ClampToBorder,
		Count
	};

	enum class TextureFormat : uint8_t {
		RGBA4,
		RGB5_A1,
		RGBA8,
		RGB10_A2,
		R11G11B10,
		SRGB8,
		SRGB8_A8,

		Depth16,
		Depth24,
		Depth32,

		Depth24_Stencil8,
		Depth32_Stencil8,
		
		Count
	};

	enum class BlendFactor : uint8_t {
		Zero,
		One,
		
		SrcColour,
		OneMinusSrcColour,
		SrcAlpha,
		OneMinusSrcAlpha,

		DstColour,
		OneMinusDstColour,
		DstAlpha,
		OneMinusDstAlpha,

		Src1Colour,
		OneMinusSrc1Colour,
		Src1Alpha,
		OneMinusSrc1Alpha,

		Count,
	};

	enum class BlendOp : uint8_t {
		Add,
		Subtract,
		// Subtracts source from dest
		InvSubtract,
		Min,
		Max,
		Count,
	};
}