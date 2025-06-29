#version 430 core

layout(binding = 0) uniform sampler2D u_diffuseTexture;

in vec4 position;
in vec2 texCoords;
in vec3 normal;

out vec4 out_color;
out vec3 out_normal;
out vec3 out_position;


void main() {
	out_color = texture(u_diffuseTexture, texCoords);
	out_normal = normalize(normal);
	out_position = position.xyz;
}

