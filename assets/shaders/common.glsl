#ifndef COMMON_H
#define COMMON_H

// HLSL like macros, as I'm more familiar with HLSL syntax
#define lerp(x, t) (mix(x,t))
#define saturate(x) (clamp(x, 0.0f, 1.0f))

// HLSL type aliases for GLSL
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float3x3 mat3x3
#define float4x3 mat4x3
#define float4x4 mat4x4

#define DEG2RAD (3.14159265 / 180.0)
#define RAD2DEG (180.0 / 3.14159265)

layout(std140) uniform GeometryBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float3 cameraPos;
    float elapsedTime;
};

layout(std140) uniform MaterialBuffer
{
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float3 emissionColour;
	float glintFactor;
    float roughness;
    float metallic;
    float emissionIntensity;
};

// equiv for CPU enum values
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct LightData
{
    // These 4 floats would be aligned into a float4, meaning a single light occupies 16 bytes
    uint type;
    float intensity;
    float innerRadius; // For spot lights
    float outerRadius; // For spot lights

    // Ignored for dir lights
    float3 position;
    float3 direction;

    // RGB colour
    float3 colour;
};

layout(std140) uniform LightsBuffer
{
    // Support up to 4 real-time lights
    LightData light0;
    LightData light1;
    LightData light2;
    LightData light3;
};

#endif // COMMON_H