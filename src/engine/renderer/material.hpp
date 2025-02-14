#pragma once

#include <inttypes.h>
#include <string>
#include <hlsl++.h>
#include <engine/gpu/idevice.hpp>

namespace render {

	constexpr uint32_t k_drawOrder_Opaque		= 2000;
	constexpr uint32_t k_drawOrder_Skybox		= 2050;
	constexpr uint32_t k_drawOrder_Transparent	= 3000;
	constexpr uint32_t k_drawOrder_Ui			= 4000;

	struct Material {
		// shader must not be null, or we will hit an assert
		gpu::IShader* shader = nullptr;

		std::string name = ""; // debug name

		// colour tints
		hlslpp::float3 ambient = { 0.2, 0.2, 0.2 };
		hlslpp::float3 diffuse = { 1, 1, 1 };
		hlslpp::float3 specular = { 1, 1, 1 };
		hlslpp::float3 emissionColour = { 0, 0, 0 };
		float glintFactor = 0.0f;
		float metallic = 0.0f;
		float roughness = 1.0f;
		float emissionIntensity = 1.0f;
	
		// textures
		gpu::ITexture* diffuseTex = nullptr;
		gpu::ITexture* metaTex = nullptr;
		gpu::ITexture* emissionTex = nullptr;
		gpu::ITexture* matcapTex = nullptr;
		gpu::ITexture* brdfLutTex = nullptr;

		uint32_t drawOrder = k_drawOrder_Opaque;
	};

	// bind material to the opengl state machine
	void bindMaterial(Material& mat);
}