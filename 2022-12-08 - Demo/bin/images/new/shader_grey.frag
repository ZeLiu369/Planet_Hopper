#version 120
uniform sampler2D currentTexture;
uniform float time;

void main() {
    vec2 coord = gl_TexCoord[0].xy;   
    vec4 pixel_color = texture2D(currentTexture, coord);
    float gray = (pixel_color.r + pixel_color.g + pixel_color.b) / 3.0;
    gl_FragColor = vec4(vec3(gray), 1.0); 

    
}
