#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 vnor; // Input vertex normal from data
layout (location = 2) in vec2 vtext;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = vtext;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
