#version 130

precision mediump float;

uniform sampler2D sprite;
uniform float time;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 texColor = texture(sprite, gl_TexCoord[0].st);

    // Calculate the current hue based on time
    float hue = time * 2.0;

    // Convert the hue value to RGB color
    vec3 rgb = hsv2rgb(vec3(hue, 1.0, 1.0));

    // Apply the rainbow color to the sprite
    gl_FragColor = vec4(rgb, texColor.a);
}