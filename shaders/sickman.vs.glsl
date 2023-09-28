#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
uniform float time;
uniform int move;
// Passed to fragment shader
out vec2 texcoord;
const float PI = 3.1415926;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main()
{
   
	texcoord = in_texcoord;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
        float duration = 3;
        float maxAmplitude = 0.1;
        float t2 = mod(time, duration);
        float amplitude = 1.0 + maxAmplitude * abs(sin(t2 * (PI / duration)));
    float t3 = mod(time, 40);
    if ((move == 1) && t3 > 10) {
        float a = t3 / 10;
        pos.x /= a;
        pos.y /= a;
        
    }
    
        gl_Position = vec4(pos.x * amplitude, pos.y / amplitude, in_position.z, 1.0);
   
}
