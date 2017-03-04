#version 330

uniform sampler2D tex;

in vec2 texCoord;

out vec4 colorOut;

void main(void)
{
    vec4 texColor = texture(tex, texCoord.st);

    colorOut = vec4(texColor.rrr, 1.0);
}
