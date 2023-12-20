#version 330

in vec4 vPosition;
in vec4 vColor;
out vec4 color;
uniform vec3 theta; //rotation angle at x,y,z-axes
uniform vec3 transform;
uniform vec3 scale;

void main()
{
    // Compute the sines and cosines of theta for each of
    //   the three axes in one computation.
    vec3 angles = radians( theta );
    vec3 c = cos( angles );
    vec3 s = sin( angles );

    // Remember that matrices are column major.
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

	// scale mat
	mat4 sc = mat4(scale.x, 0.0, 0.0, 0.0,
	               0.0, scale.y, 0.0, 0.0,
				   0.0, 0.0, scale.z, 0.0,
				   0.0, 0.0, 0.0, 1.0);


	gl_Position = tl * rx * ry * rz * sc * vPosition;
	color = vColor;
}