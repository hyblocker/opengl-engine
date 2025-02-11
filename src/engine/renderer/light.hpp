#pragma once

#include <inttypes.h>
#include <hlsl++.h>
#include "engine/gpu/idevice.hpp"
#include "engine/renderer/scene_graph.hpp"

namespace render {

    enum class LightType : uint32_t {
        Directional = 0,
        Point,
        Spot,
    };

    class Light : public IComponent {
    public:
        Light(Entity* parent) : IComponent(parent) {
            componentType = ::render::ComponentType::Light;
        }
        ~Light() = default;

		LightType type = LightType::Directional;

        // Ignored for dir lights
        inline hlslpp::float3 getPosition() const {
            return getEntity()->transform.getPosition();
        }
		inline hlslpp::float3 getDirection() const {
            return -hlslpp::normalize(mul(getEntity()->transform.getRotation(), hlslpp::float3(0.0f, 0.0f, 1.0f)));
        }

        // RGB colour
		hlslpp::float3 colour = { 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        float innerRadius = 0.6f; // For spot lights
        float outerRadius = 1.0f; // For spot lights
    private:
        // derived classes are forbidden from modifying componentType
        using IComponent::componentType;
	};
}