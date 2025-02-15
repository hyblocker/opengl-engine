layout(location = 0) in vec2 iPosition;
layout(location = 2) in vec2 iUv;

out vec2 uv;

layout(std140) uniform TextBuffer
{
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 colourForeground;
    vec4 colourOutline;
    float outlineWidth;
};

void main()
{
    vec4 pos = projection * view * model * vec4(iPosition, 0.0, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);
    uv = iUv.xy;
}