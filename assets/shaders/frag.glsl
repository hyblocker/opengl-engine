precision mediump float;

#include "common.glsl"
#include "lighting.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform sampler2D specularTex;
layout(binding = 2) uniform sampler2D emissionTex;

// normal and pos are in world-space
in vec3 worldPos;
in vec3 normal;
in vec2 uv;

vec3 computeLighting(LightData lightData, in vec3 albedo, vec3 normal) {
    // pre-compute vectors and dot products we're going to use a lot for PBR lighting
    vec3 n = normalize(normal);
    vec3 l = normalize(lightData.direction);
    vec3 v = normalize(cameraPos - worldPos);
    vec3 h = normalize(v + l);
    vec3 r = reflect(-v, n);

    // We check if the magnitude of the dir vector is > 0.5 (i.e. it's a valid normalised value)
    // if it's a 0 vector the magnitude would be ~0, in which case we assume the light is disabled.
    float lightFactor = (length(lightData.direction) > 0.5f) ? 1.0 : 0.0;

    float NdotL = saturate(dot(n, l));
    float NdotV = saturate(dot(n, v));
    float LdotH = saturate(dot(l, h));
    float HdotV = saturate(dot(h, v));
    float NdotH = saturate(dot(n, h));

    float roughness = 0.99f;

    // compute diffuse lobe
    float Fd = Fd_Burley(NdotV, NdotL, LdotH, roughness);
    
    // compute specular lobe
    vec3 F = F_Schlick3(HdotV, albedo);
    float NDF = D_GGX(NdotH, roughness);
    float G = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
    vec3  Fr = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.001f /* avoid divide by 0 */);

    float lightAtten = lightFactor * NdotL;

    return albedo * Fd * lightData.colour * lightData.intensity * lightAtten +
        Fr * lightData.intensity * lightData.colour * lightAtten;
}

void main()
{
    vec4 albedo = vec4(texture(diffuseTex, uv).rgb, 1.0f);
    
    vec3 light0Contribution = computeLighting(light0, albedo.rgb, normal);
    vec3 light1Contribution = computeLighting(light1, albedo.rgb, normal);
    vec3 light2Contribution = computeLighting(light2, albedo.rgb, normal);
    vec3 light3Contribution = computeLighting(light3, albedo.rgb, normal);

    vec3 finalColor = ambient.rgb +
        light0Contribution +
        light1Contribution +
        light2Contribution +
        light3Contribution;

    fragColor = vec4(finalColor.rgb, albedo.a);
}
