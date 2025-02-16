precision mediump float;

layout(location = 0) in vec3 iPosition;
layout(location = 2) in vec2 iUv;

out vec2 uv;

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
    // in pos is 2x2 px, centered around 0,0
    // take in pos and scale it such that we are sizeX x sizeY pixels in size at the end
    // projection takes 
    vec2 newPos = iPosition.xy;
    newPos.xy = newPos.xy * size.xy * 0.5f + vec2(offset.x, -offset.y); // flip offset y because OpenGL
    vec4 pos = projection * model * vec4(newPos, 0.0, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);
    uv = vec2(1.0f - iUv.x, iUv.y);
}