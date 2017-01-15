#version 330

uniform sampler2D tex;
varying vec2 texCoord;

void main(void)
{
    vec4 texColor = texture(tex, texCoord.st);

    gl_FragColor = vec4(texColor.g, 1.0, 0.0, 1.0); //texColor.rrrg;
}
