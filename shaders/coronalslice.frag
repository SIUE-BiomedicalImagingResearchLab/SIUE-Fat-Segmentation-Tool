#version 330

uniform sampler2D tex;
varying vec2 texCoord;

void main(void)
{
    vec4 texColor = texture(tex, texCoord.st);

    gl_FragColor = vec4(texColor.rrr, 1.0);
}
