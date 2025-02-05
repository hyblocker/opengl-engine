layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iUv;

#include "common.glsl"

out vec3 eyeDir;

void main() {
    // i use a unit sphere and only use the vertex positions, which when we apply a special case of the view matrix doesn't have translations
    vec4 pos = projection * view * vec4(iPosition, 1.0);
    gl_Position = vec4(pos.xy, 1.0, pos.w);
    eyeDir = iPosition.xyz;
}