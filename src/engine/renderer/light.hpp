#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include "engine/gpu/idevice.hpp"
#include "engine/renderer/scene_graph.hpp"

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

    class Light : public IComponent {
    public:
        Light(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::Light;
        }
        ~Light() = default;

		LightType type = LightType::Directional;
		AttenuationType attenuation = AttenuationType::None;

        // Ignored for dir lights
		hlslpp::float3 position = { 0, 0, 0 };
		hlslpp::float3 direction = { 0, -1, 0 };

        // RGB colour
		hlslpp::float3 colour = { 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        float radius = 1.0f; // For spot and point lights
    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
	};
}