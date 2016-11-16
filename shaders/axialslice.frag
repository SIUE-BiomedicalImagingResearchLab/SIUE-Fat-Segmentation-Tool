#version 330

uniform sampler2D tex;
varying vec2 texCoord;

void main(void)
{
    vec4 texColor = texture2D(tex, texCoord.st);
    gl_FragColor = vec4(texColor.r, 0.0, 0.0, 1.0);
    //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
