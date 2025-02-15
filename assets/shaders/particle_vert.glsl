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
    float particleTextureCount; // technically worse as its per particle, but these 4 bytes would've been padding anyway
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

ivec2 calculateAtlasDimensions(float particleTextureCountRaw) {
    int particleTextureCount = int(particleTextureCountRaw);
    if (particleTextureCount <= 0) {
        return ivec2(0, 0);
    }

    int size = 1;
    particleTextureCount--;
    size |= particleTextureCount >> 1U;
    size |= particleTextureCount >> 2U;
    size |= particleTextureCount >> 4U;
    size |= particleTextureCount >> 8U;
    size |= particleTextureCount >> 16U;
    size++;

    int cols = size;
    int rows = size;

    if (particleTextureCount + 1 <= size * (size / 2)) {
        cols = size / 2;
    }
    if (particleTextureCount == 0) {
        cols = 1;
        rows = 1;
    }
    
    cols = max(cols, rows);

    return ivec2(float(cols), float(rows));
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
    
    // UVs
    int particleTextureIndex = int(mod(gl_InstanceID, int(particles[gl_InstanceID].particleTextureCount)));
    ivec2 atlasImageCount = calculateAtlasDimensions(particles[gl_InstanceID].particleTextureCount);
    
    float uvScaleX = 1.0 / float(atlasImageCount.x);
    float uvScaleY = 1.0 / float(atlasImageCount.y);

    int col = int(mod(int(particleTextureIndex), int(atlasImageCount.x)));
    int row = particleTextureIndex / atlasImageCount.x;

    float uvOffsetX = float(col) * uvScaleX;
    float uvOffsetY = float(row) * uvScaleY;

    vec2 finalUv = iUv.xy * vec2(uvScaleX, uvScaleY) + vec2(uvOffsetX, uvOffsetY);

    uvLife.xy = finalUv.xy;
    uvLife.z = particles[gl_InstanceID].life;

    colourBegin = particles[gl_InstanceID].colourBegin;
    colourEnd = particles[gl_InstanceID].colourEnd;
}