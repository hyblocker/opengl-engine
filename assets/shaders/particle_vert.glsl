layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iUv;

#include "common.glsl"

struct ParticleData
{
    vec3 position;
    vec3 velocity;
    vec4 colourBegin;
    vec4 colourEnd;

    // packed as float4
    float sizeBegin;
    float sizeEnd;
    float life;
};

layout(std140) uniform ParticleBuffer
{
    ParticleData particles[800];
};

out vec3 worldPos;
out vec3 normal;
out vec3 uvLife;
out vec4 colourBegin;
out vec4 colourEnd;

vec4 billboard(vec4 vertex)
{
    const vec2 scale = vec2(1, 1);
    mat4 matrixModelView = view * model;

    vec4 billboardPosition =
        (matrixModelView * vec4(0.0, 0.0, 0.0, 1.0))
        + vec4(vertex.x * scale.x, vertex.y * scale.y, 0.0, 0.0);

    return projection * billboardPosition;
}

void main()
{
    float size = mix(particles[gl_InstanceID].sizeEnd, particles[gl_InstanceID].sizeBegin, particles[gl_InstanceID].life);

    vec3 particlePos = iPosition.xyz * vec3(size, size, size) + particles[gl_InstanceID].position;
    particlePos = particlePos;

    // primitive z sort
    // push particles a hint away based on age
    particlePos.z -= (1.0f - particles[gl_InstanceID].life) * 0.1f;

    gl_Position = projection * view * model * vec4(particlePos, 1.0);
    gl_Position = billboard(vec4(particlePos, 1.0));
    worldPos = (model * vec4(particlePos, 1.0)).xyz;
    normal = (view * model * vec4(iNormal, 0.0)).xyz;
    uvLife.xy = iUv.xy;
    uvLife.z = particles[gl_InstanceID].life;
    colourBegin = particles[gl_InstanceID].colourBegin;
    colourEnd = particles[gl_InstanceID].colourEnd;
}