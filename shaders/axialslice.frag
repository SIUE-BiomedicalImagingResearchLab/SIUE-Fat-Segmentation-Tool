#version 330

uniform sampler2D tex;
varying vec2 texCoord;

uniform float brightness;
uniform float contrast;

vec4 colormap(float x);

void main(void)
{
    vec4 texColor = texture2D(tex, texCoord.st);

    gl_FragColor = colormap((texColor.r * contrast) + brightness);
}
