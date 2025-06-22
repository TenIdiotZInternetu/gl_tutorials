#version 430 core

const int SSAO_COUNT = 64;
const vec3 AMBIENT_COLOR = vec3(0.1, 0.1, 0.1);

layout(binding = 0) uniform sampler2D u_diffuse;
layout(binding = 1) uniform sampler2D u_normal;
layout(binding = 2) uniform sampler2D u_position;
layout(binding = 3) uniform sampler2D u_shadowMap;

layout(location = 15) uniform vec3 u_lightPos;
layout(location = 20) uniform mat4 u_lightViewMat;
layout(location = 40) uniform mat4 u_lightProjMat;

layout(location = 60) uniform mat4 u_viewMat;
layout(location = 80) uniform mat4 u_projMat;
layout(location = 128) uniform vec3 u_ssaoSamples[SSAO_COUNT];

in vec2 texCoords;

out vec4 fragColor;

// randomVec determines rotation of the base around Z axis => rotation of the ssao kernel
mat3 tbnMatrix(vec3 normal, vec3 randomVec) {
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	return mat3(tangent, bitangent, normal);
}

float ssaoFactor(vec3 position, mat3 tbn) {
	int samplesOccluded = 0;

	for (int i = 0; i < SSAO_COUNT; ++i) {
		vec3 sampl = u_ssaoSamples[i];
		vec4 samplePos = vec4(position + tbn * sampl, 1);
		float sampleDepth = (u_viewMat * u_projMat * samplePos).z;

		float fragDepth = texture(u_position, samplePos.xy).z;
		if (sampleDepth >= fragDepth) {
			samplesOccluded++;
		}
	}

	return samplesOccluded / SSAO_COUNT;
}

void main() {
	vec3 position = texture(u_position, texCoords).xyz;
	vec3 normal = texture(u_normal, texCoords).xyz;
	vec3 diffuseColor = texture(u_diffuse, texCoords).xyz;

	mat3 tbn = tbnMatrix(normal, diffuseColor);
	vec3 ambientColor = AMBIENT_COLOR * ssaoFactor(position, tbn);

	vec3 lightDir = normalize(u_lightPos - position);
	float lamb = max(dot(lightDir, normal), 0.0);
	fragColor = vec4(lamb * diffuseColor + AMBIENT_COLOR, 1.0);

	vec4 shadowCoords = (u_lightProjMat * u_lightViewMat * vec4(position, 1.0));
	// shadowCoords are in light clipspace, but we get fragment relative 
	// coordinates in the shadowmap, so we need to remap to [0,1] interval in all dimensions
	vec3 mappedShadowCoords = (shadowCoords.xyz/shadowCoords.w) * 0.5 + 0.5;
	if (mappedShadowCoords.x > 0 && mappedShadowCoords.x < 1
		&& mappedShadowCoords.y > 0 && mappedShadowCoords.y < 1) {
		float shadow = texture(u_shadowMap, mappedShadowCoords.xy).x;
		if (shadow < (mappedShadowCoords.z - 0.000001)) {
			fragColor = vec4(0.5 * diffuseColor, 1.0);
		}
	}
}

