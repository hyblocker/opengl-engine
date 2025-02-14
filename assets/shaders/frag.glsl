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

vec3 computeLighting(LightData lightData, in vec3 albedo, vec3 normal, float perceptualRoughness) {
    // pre-compute vectors and dot products we're going to use a lot for PBR lighting
    vec3 n = normalize(normal);
    vec3 l = normalize(lightData.direction);
    vec3 v = normalize(cameraPos - worldPos);
    vec3 h = normalize(v + l);
    vec3 r = reflect(-v, n);

    // We check if the magnitude of the dir vector is > 0.5 (i.e. it's a valid normalised value)
    // if it's a 0 vector the magnitude would be ~0, in which case we assume the light is disabled.
    float lightFactor = (abs(length(lightData.direction)) > 0.5f) ? 1.0 : 0.0;

    float NdotL = saturate(dot(n, l));
    float NdotV = saturate(dot(n, v));
    float LdotH = saturate(dot(l, h));
    float HdotV = saturate(dot(h, v));
    float NdotH = saturate(dot(n, h));

    vec3 F0 = max(albedo, vec3(0.04, 0.04, 0.04));

    // compute diffuse lobe
    float Fd = Fd_Burley(NdotV, NdotL, LdotH, perceptualRoughness);
    
    // compute specular lobe
    vec3 F = F_Schlick3(HdotV, F0);
    float NDF = D_GGX(NdotH, perceptualRoughness);
    float G = V_SmithGGXCorrelated(NdotV, NdotL, perceptualRoughness);
    vec3  Fr = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.001f /* avoid divide by 0 */);

    float lightAtten = lightFactor * NdotL;

    return albedo * Fd * lightData.colour * lightData.intensity * lightAtten +
        Fr * lightData.intensity * lightData.colour * lightAtten;
}

vec4 sampleMatcap(vec3 normal, float roughness) {

    // specular IBL
    const float k_MAX_REFLECTION_LOD = log2(256); // log2(matcapResolution)
    float specularLevel = clamp(roughness * k_MAX_REFLECTION_LOD, 0.0, k_MAX_REFLECTION_LOD);

    const float k_MATCAP_BORDER = 0.43;
    highp vec2 matcapUv = normal.xy * k_MATCAP_BORDER + vec2(0.5, 0.5);
    return textureLod(matcapTex, vec2(matcapUv.x, matcapUv.y), specularLevel);
}

// ibl contribution
vec3 computeIBL(vec3 albedo, vec3 normal, float roughness, inout vec3 specular) {

    vec3 n = normalize(normal);
    vec3 v = normalize(cameraPos - worldPos);
    vec3 r = reflect(-v, n);
    
    float NdotV = saturate(dot(n, v));
    
    vec3 F0 = max(albedo, vec3(0.04, 0.04, 0.04));
    vec3 kS = F_SchlickRoughness(NdotV, F0, roughness);
    vec3 kD = 1.0 - kS;
    vec4 irradiance = sampleMatcap(n, 0.9);
    vec3 ambient    = (kD * irradiance.rgb);

    // Read brdf texture from disk
    vec4 brdf = textureLod(brdfLutTex, vec2(NdotV, roughness), 0);
    // undo sRGB to read linear tex
    brdf.x = pow(brdf.x, 1 / 2.2);
    brdf.y = pow(brdf.y, 1 / 2.2);

    vec4 iblSpecular = sampleMatcap(r, roughness);
    specular = iblSpecular.rgb * (kS * brdf.x + brdf.y);

    return ambient.rgb;
}

float genGlint(vec2 uv) {
    float angle = -45.0 * DEG2RAD;
    float s = sin(angle);
    float c = cos(angle);

    mat2 rotMat = mat2(
        c, -s,
        s, c
    );
    uv.y += elapsedTime * 2.0;
    float samplePt = (uv * rotMat).y;
    float glint = pow(clamp(sin(samplePt * 3.0) - 0.75 + sin(samplePt * 4.0) - 0.75, 0.0, 1.0) * 2.2179342161, 1.4);
    
    return glint;
}

void main()
{
    vec4 albedo = vec4(texture(diffuseTex, uv).rgb, 1.0f) * vec4(diffuse, 1.0);
    vec4 meta = pow(vec4(texture(metaTex, uv).rgb, 1.0f), vec4(1.0/2.2)); // read metaTex and convert to linear
    float metal = meta.r * metallic;
    float roughness = meta.g * roughness;
    float perceptualRoughness = clamp(roughness, 0.01f, 0.99f);

    vec3 iblSpecular;
    vec3 iblDiffuse = computeIBL(albedo.rgb, normal, perceptualRoughness, iblSpecular);

    vec3 light0Contribution = computeLighting(light0, albedo.rgb, normal, perceptualRoughness);
    vec3 light1Contribution = computeLighting(light1, albedo.rgb, normal, perceptualRoughness);
    vec3 light2Contribution = computeLighting(light2, albedo.rgb, normal, perceptualRoughness);
    vec3 light3Contribution = computeLighting(light3, albedo.rgb, normal, perceptualRoughness);

    float glintFac = genGlint(uv) * glintFactor;
    vec3 glint = vec3(glintFac, glintFac, glintFac);

    vec3 finalColor = iblDiffuse.rgb * albedo.rgb +
        ambient.rgb * iblSpecular +
        light0Contribution +
        light1Contribution +
        light2Contribution +
        light3Contribution + glint;

    fragColor = vec4(finalColor.rgb, albedo.a);
}
