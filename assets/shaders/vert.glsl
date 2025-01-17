layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;
layout(location = 2) in vec2 iUv;

#include "common.glsl"

out vec3 color;
out vec2 uv;

void main()
{
    gl_Position = vec4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    color = iColor * coolColor.rgb;
    uv = iUv.xy;
}