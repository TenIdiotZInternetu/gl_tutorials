#version 430 core

const vec3 AMBIENT_COLOR = vec3(0.1, 0.1, 0.2);

layout(location = 0) uniform sampler2D u_albedo;
layout(location = 1) uniform sampler2D u_normal;
layout(location = 2) uniform sampler2D u_position;
layout(location = 3) uniform sampler2D u_ssao;

layout(location = 10) uniform bool u_enableAlbedo;

layout(location = 15) uniform vec3 u_lightPos;
layout(location = 20) uniform mat4 u_lightViewMat;
layout(location = 40) uniform mat4 u_lightProjMat;

in vec2 texCoords;

// out vec4 fb1;
// out vec4 fb2;
// out vec4 fb3;
// out vec4 fb4;
out vec4 fragColor;

void main() {
	vec3 position = texture(u_position, texCoords).xyz;
	vec3 normal = texture(u_normal, texCoords).xyz;
	vec3 albedo = texture(u_albedo, texCoords).xyz;
	float ssaoFactor = texture(u_ssao, texCoords).r;

	if (!u_enableAlbedo) {
		fragColor = vec4(ssaoFactor);
		return;
	}

	vec3 ambientColor = AMBIENT_COLOR * ssaoFactor;
	vec3 lightDir = normalize(u_lightPos - position);
	float lamb = max(dot(lightDir, normal), 0.0);
	
	fragColor = vec4(lamb * albedo + ambientColor, 1.0);
}


