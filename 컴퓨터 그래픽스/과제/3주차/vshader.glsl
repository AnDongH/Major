#version 330

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 model_view;
uniform mat4 projection;

uniform vec3 transform;
uniform vec3 theta;

void main()
{
	vec3 angles = radians( theta );
    vec3 c = cos( angles );
    vec3 s = sin( angles );

	mat4 rx = mat4( 1.0,  0.0,  0.0, 0.0,
					0.0,  c.x,  s.x, 0.0,
					0.0, -s.x,  c.x, 0.0,
					0.0,  0.0,  0.0, 1.0 );

    mat4 ry = mat4( c.y, 0.0, -s.y, 0.0,
					0.0, 1.0,  0.0, 0.0,
					s.y, 0.0,  c.y, 0.0,
					0.0, 0.0,  0.0, 1.0 );

	mat4 rz = mat4( c.z, s.z, 0.0, 0.0,
		           -s.z,  c.z, 0.0, 0.0,
		            0.0,  0.0, 1.0, 0.0,
		            0.0,  0.0, 0.0, 1.0 );

	// transform mat
	mat4 tl = mat4(1.0, 0.0, 0.0, 0.0,
	               0.0, 1.0, 0.0, 0.0,
				   0.0, 0.0, 1.0, 0.0,
				   transform.x, transform.y, transform.z, 1.0);


	gl_Position = projection * model_view * tl * rx * ry * rz * vPosition;
	color = vColor;
}