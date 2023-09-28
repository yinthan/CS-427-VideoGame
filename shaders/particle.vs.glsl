#version 330

// !!! Simple shader for colouring basic meshes

// Input attributes
in vec3 in_position;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float lifetime;
uniform float pos_x[100];
uniform float pos_y[100];
uniform float scale[100];

void main()
{
	mat3 transform2;
	transform2[0][0] = transform[0][0];
	transform2[1][0] = transform[1][0];
	transform2[2][0] = transform[2][0];
	transform2[0][1] = transform[0][1];
	transform2[1][1] = transform[1][1];
	transform2[2][1] = transform[2][1];
	transform2[0][2] = scale[gl_InstanceID] * pos_x[gl_InstanceID];
	transform2[1][2] = scale[gl_InstanceID] * pos_y[gl_InstanceID];
	transform2[2][2] = transform[2][2];
	vec3 pos = projection * transform2 * vec3(in_position.xy, 1.0);

	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}