#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texPos;

uniform mat4 MVP;

out vec2 texCoord;

void main(void)
{
    gl_Position = MVP * vec4(pos, 1.0);
    texCoord = texPos;
}
