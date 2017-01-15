#version 330

uniform sampler2D tex;
uniform sampler1D mappingTexture;
varying vec2 texCoord;

uniform float opacity;
uniform float brightness;
uniform float contrast;

void main(void)
{
    vec4 texColor = texture(tex, texCoord.st);

    if (texColor.g < 0.0001f)
        discard;

    //gl_FragColor = vec4(texColor.g, 0.0, 0.0, 1.0);
    gl_FragColor = vec4(texture(mappingTexture, clamp(((texColor.r * contrast) + brightness), 0.0, 1.0)).rgb, opacity);
}
