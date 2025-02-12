layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iUv;

#include "common.glsl"

out vec3 worldPos;
out vec3 normal;
out vec2 uv;

void main()
{
    gl_Position = projection * view * model * vec4(iPosition, 1.0);
    worldPos = (model * vec4(iPosition, 1.0)).xyz;
    normal = (view * model * vec4(iNormal, 0.0)).xyz;
    uv = iUv.xy;
}