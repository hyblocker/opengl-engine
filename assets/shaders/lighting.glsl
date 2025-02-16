#ifndef LIGHTING_H
#define LIGHTING_H

#include "common.glsl"

// PBR equations taken from https://google.github.io/filament/Filament.html

// diffuse brdf
#define k_PI (3.14159265359)
#define k_F0 (0.04f)

#define k_LIGHT_ATTEN_CUTOFF (0.01f)

float F_Schlick(float u, float f0, float f90) {
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = F_Schlick(NdotL, 1.0, f90);
    float viewScatter = F_Schlick(NdotV, 1.0, f90);
    return lightScatter * viewScatter * (1.0 / k_PI);
}

// specular brdf
float D_GGX(float NdotH, float a) {
    float a2 = a * a;
    float f = (NdotH * NdotH * (a2 - 1.0f)) + 1.0;
    return a2 / (k_PI * f * f);
}

vec3 F_Schlick3(float u, vec3 f0) {
    return f0 + (vec3(1.0) - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NdotV, float NdotL, float a) {
    float a2 = a * a;
    float GGXL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    float GGXV = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    return 0.5 / (GGXV + GGXL);
}

vec3 F_SchlickRoughness(float u, vec3 f0, float roughness) {
    return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - u, 5.0);
}

// lighting funcs
float getSquareFalloffAttenuation(vec3 posToLight, float lightInvRadius) {
    float distanceSquare = dot(posToLight, posToLight);
    float factor = distanceSquare * lightInvRadius * lightInvRadius;
    float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

float getSpotAngleAttenuation(vec3 l, vec3 lightDir, float innerAngle, float outerAngle) {
    // the scale and offset computations can be done CPU-side
    float cosOuter = cos(outerAngle);
    float spotScale = 1.0 / max(cos(innerAngle) - cosOuter, 1e-4);
    float spotOffset = -cosOuter * spotScale;

    float cd = dot(normalize(-lightDir), l);
    float attenuation = clamp(cd * spotScale + spotOffset, 0.0, 1.0);
    return attenuation * attenuation;
}

void ComputeLight(LightData lightData, const vec3 worldPos, out vec3 pLightDir, out vec3 pLightColour, out float attenuation) {
    // Assume directional light
    attenuation = 1.0;
    pLightColour = lightData.colour * lightData.intensity;
    pLightDir = vec3(0,1,0);

    // Handle directional lights
    if (lightData.type == LIGHT_TYPE_DIRECTIONAL) {
        attenuation = 1.0;
        pLightDir = lightData.direction;
    }

    // Handle point / spot lights
    if (lightData.type == LIGHT_TYPE_POINT) {
        // inverse square law
        attenuation = getSquareFalloffAttenuation(worldPos - lightData.position, lightData.outerRadius);
        pLightDir = lightData.direction;
    }

    if (lightData.type == LIGHT_TYPE_SPOT) {
        attenuation = getSquareFalloffAttenuation(worldPos - lightData.position, lightData.outerRadius) *
            getSpotAngleAttenuation(worldPos, lightData.direction, lightData.innerRadius, lightData.outerRadius);
        pLightDir = lightData.direction;
    }
}

#endif // LIGHTING_H