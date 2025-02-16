precision mediump float;

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D msdfTex;

in vec2 uv;

layout(std140) uniform TextBuffer
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 colourForeground;
    vec4 colourOutline;
    float outlineWidth;
};

float screenPxRange() {
    // given a 32x32 distance field with a pixel range of 2 and a render resolution of 72x72
    // return 4.5
    return 72.0 / 32.0 * 2.0;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
    vec3 msdfRaw = vec4(texture(msdfTex, uv).rgb, 1.0f).rgb;
    float sd = median(msdfRaw.r, msdfRaw.g, msdfRaw.b);
    float screenPxDistance = screenPxRange() * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    
    float screenPxDistanceOutline = screenPxRange() * (sd - 0.5 - outlineWidth);
    float outlineOpacity = clamp(screenPxDistance + 0.5 + outlineWidth, 0.0, 1.0);
    
    vec4 textColor = vec4(colourForeground.rgb * opacity, opacity);
    vec4 outlineColor = vec4(colourOutline.rgb * outlineOpacity, outlineOpacity);
    
    fragColor = mix(outlineColor, textColor, opacity);
}
