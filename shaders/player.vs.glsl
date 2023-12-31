#version 330

// Input attributes
in vec3 in_position;
in vec3 in_color;
uniform float time;
out vec3 vcolor;
out vec2 vpos;
uniform int onPlatForm;
// Application data
uniform mat3 transform;
uniform mat3 projection;
const float PI = 3.1415926;

void main()
{
	vpos = in_position.xy; // local coordinated before transform
	vcolor = in_color;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0); // why not simply *in_position.xyz ?
    
        gl_Position = vec4(pos.xy, in_position.z, 1.0);
    
}
