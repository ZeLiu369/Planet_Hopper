#version 130

precision mediump float;

uniform sampler2D sprite;

void main()
{
    // Sample the sprite texture with a blur filter
    vec2 texCoord = gl_TexCoord[0].st;
    vec4 texColor = texture(sprite, texCoord);
    texColor += texture(sprite, texCoord + vec2(-0.01, 0.01));
    texColor += texture(sprite, texCoord + vec2(-0.01, 0.00));
    texColor += texture(sprite, texCoord + vec2(-0.01,-0.01));
    texColor += texture(sprite, texCoord + vec2( 0.01, 0.01));
    texColor += texture(sprite, texCoord + vec2( 0.01, 0.00));
    texColor += texture(sprite, texCoord + vec2( 0.01,-0.01));
    texColor += texture(sprite, texCoord + vec2( 0.00, 0.01));
    texColor += texture(sprite, texCoord + vec2( 0.00,-0.01));
    texColor /= 9.0;

    // Apply the blur filter to the sprite
    gl_FragColor = vec4(texColor.rgb, texColor.a);
}









