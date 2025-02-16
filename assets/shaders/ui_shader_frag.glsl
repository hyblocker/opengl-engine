precision mediump float;

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D uiTex;

in vec2 uv;

layout(std140) uniform UiBuffer
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 textureTint;
    vec2 size;
    vec2 offset;
    vec4 screenSize; // xy => screenSize width height, zw = 1/ width 1/height
};

void main()
{
    vec4 textureColour = texture(uiTex, uv);
    // premultipled colour out
    fragColor = vec4(textureTint.rgb * textureTint.a * textureColour.rgb * textureColour.a, textureColour.a * textureTint.a);
}
