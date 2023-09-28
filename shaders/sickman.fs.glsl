#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform int move;
uniform float time;
uniform vec2 widtheight;

// Output color
layout(location = 0) out  vec4 color;

float _Blur = abs(sin(time))/2;

void main()
{
    color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
    if (move == 3) {
        float distance = _Blur * 0.0625f;
        vec4 leftColor = texture(sampler0,vec2(texcoord.x - distance,texcoord.y));
        vec4 rightColor = texture(sampler0,vec2(texcoord.x + distance,texcoord.y));
        vec4 topColor = texture(sampler0,vec2(texcoord.x,texcoord.y + distance));
        vec4 bottomColor = texture(sampler0,vec2(texcoord.x,texcoord.y - distance));

        color = color * 4 + leftColor + rightColor + topColor + bottomColor;

        color = color * 0.125;
    }
    vec2 coord = texcoord;
    if (move == 1) {
        coord.x = -coord.x;
        color = vec4(fcolor, 1.0) * texture(sampler0, coord);
    }
    if (move == 2) {
        float i = cos(time)/13;
        float j = sin(time)/11;
        float k = cos(time)/7*sin(time)/5;
        color += vec4(i,j,k,0);
    }
    
}
