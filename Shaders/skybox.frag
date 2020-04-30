#version 330
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}
