layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iUv;

#include "common.glsl"

out vec3 normal;
out vec2 uv;

void main()
{
    vec4 pos = projection * view * model * vec4(iPosition, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);
    normal = iNormal.xyz;
    uv = iUv.xy;
}