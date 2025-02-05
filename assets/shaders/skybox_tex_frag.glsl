precision mediump float;

#include "common.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D brickTex;

in vec3 normal;
in vec2 uv;

void main()
{
    vec4 albedo = vec4(texture(brickTex, uv).rgb, 1.0f);

    float atten = saturate(dot(normal, normalize(vec3(0, 1, 2))));

    fragColor = albedo * atten + vec4(ambient.rgb, 0.0f);
}
