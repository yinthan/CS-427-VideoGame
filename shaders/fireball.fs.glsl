#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;

// Output color
layout(location = 0) out  vec4 color;

vec4 color_shift(vec4 color)
{
    float i = sin(time*3)/5;
    color -= vec4(i, i, i, 0.0);
    return color;
}


void main()
{
    color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
    color = color_shift(color);
}
