#pragma once

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

        // Ignored for dir lights
		hlslpp::float3 position = { 0, 0, 0 };
		hlslpp::float3 direction = { 0, -1, 0 };

        // RGB colour
		hlslpp::float3 colour = { 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        float radius = 1.0f; // For spot and point lights
	};
}