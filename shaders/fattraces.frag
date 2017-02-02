#version 330

uniform sampler2D tex;
uniform vec4 traceColor;

in vec2 texCoord;

out vec4 colorOut;

void main(void)
{
    vec4 texColor = texture(tex, texCoord.st);

    colorOut = vec4(traceColor.rgb, texColor.r);
}
