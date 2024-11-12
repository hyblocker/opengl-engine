#include <inttypes.h>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>

namespace render {

    enum class LightType : uint8_t {
        Directional,
        Point,
        Spot,
    };

    enum class AttenuationType : uint8_t {
        None,
        Linear,
        Quad,
    };

	struct Light {
		LightType type = LightType::Directional;
		AttenuationType attenuation = AttenuationType::None;

		float alpha = 0.965f;
		float beta = 0.82f;

		hlslpp::float3 position = {0, 0, 0};
		hlslpp::float3 direction = { 0, -1, 0 };

		hlslpp::float3 ambient = { 0.2, 0.2, 0.2 };
		hlslpp::float3 diffuse = { 1, 1, 1 };
		hlslpp::float3 specular = { 1, 1, 1 };
	};
}