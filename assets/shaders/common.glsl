
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

layout(std140) uniform DataBuffer
{
    vec4 coolColor;
    float colorBlendFac;
};