#pragma once

#include <inttypes.h>
#include <string>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>

namespace engine {
	struct Material {
		// shader must not be null, or we will hit an assert
		gpu::IShader* shader = nullptr;

		std::string name = ""; // debug name

		// colour tints
		hlslpp::float3 ambient = { 0.2, 0.2, 0.2 };
		hlslpp::float3 diffuse = { 1, 1, 1 };
		hlslpp::float3 specular = { 1, 1, 1 };
		hlslpp::float3 emissionColour = { 0, 0, 0 };
		float emissionIntensity = 1.0f;
	
		// textures
		gpu::ITexture* diffuseTex = nullptr;
		gpu::ITexture* specularTex = nullptr;
		gpu::ITexture* emissionTex = nullptr;
	};

	// bind material to the opengl state machine
	void bindMaterial(Material& mat);
}