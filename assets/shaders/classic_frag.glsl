precision mediump float;

#include "common.glsl"
#include "lighting.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D metaTex;
layout(binding = 2) uniform sampler2D emissionTex;
layout(binding = 3) uniform sampler2D matcapTex;
layout(binding = 4) uniform sampler2D brdfLutTex;

// normal and pos are in world-space
in vec3 worldPos;
in vec3 normal;
in vec2 uv;

void main()
{
    vec4 albedo = vec4(texture(diffuseTex, uv).rgb, 1.0f) * vec4(diffuse, 1.0);
    fragColor = vec4(albedo.rgb, albedo.a);
}
