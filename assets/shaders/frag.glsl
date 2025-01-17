precision mediump float;

#include "common.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D brickTex;

in vec3 color;
in vec2 uv;

void main()
{
    fragColor = vec4(mix(color, texture(brickTex, uv).rgb, colorBlendFac), 1.0f);
}
