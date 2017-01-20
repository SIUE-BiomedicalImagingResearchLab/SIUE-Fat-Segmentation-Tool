#version 330

uniform sampler2D tex;
uniform sampler1D mappingTexture;
varying vec2 texCoord;

uniform float opacity;
uniform float brightness;
uniform float contrast;

void main(void)
{
    vec4 texColor = texture2D(tex, texCoord.st);

    gl_FragColor = vec4(texture1D(mappingTexture, clamp(((texColor.r * contrast) + brightness), 0.0, 1.0)).rgb, opacity);
}
