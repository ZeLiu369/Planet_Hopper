#version 130

uniform sampler2D texture;

void main()
{
    vec2 texCoords = gl_TexCoord[0].st;
    vec4 color = texture2D(texture, texCoords);

    // Blur the color by taking the average of the neighboring pixels
    vec2 size = vec2(textureSize(texture, 0));
    vec2 dx = vec2(1.0 / size.x, 0.0);
    vec2 dy = vec2(0.0, 1.0 / size.y);
    color += texture2D(texture, texCoords + dx);
    color += texture2D(texture, texCoords - dx);
    color += texture2D(texture, texCoords + dy);
    color += texture2D(texture, texCoords - dy);
    color += texture2D(texture, texCoords + dx + dy);
    color += texture2D(texture, texCoords + dx - dy);
    color += texture2D(texture, texCoords - dx + dy);
    color += texture2D(texture, texCoords - dx - dy);
    color /= 11.0;

    gl_FragColor = color;
}











