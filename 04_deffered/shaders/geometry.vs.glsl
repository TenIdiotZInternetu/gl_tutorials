#version 430 core

uniform mat4 u_viewMat;
uniform mat4 u_projMat;

// uniform mat4 u_lightMat;

in vec3 in_vert;
in vec3 in_normal;
in vec2 in_texCoords;

out vec4 position;
out vec2 texCoords;
out vec3 normal;

out vec4 shadowCoords;

void main(void)
{
	position = vec4(in_vert, 1);
	normal = normalize(in_normal);
	texCoords = in_texCoords;

	gl_Position = u_projMat * u_viewMat * position;

	// shadowCoords = u_lightMat * position;
}

