#version 430 core

const int SSAO_COUNT = 64;
const float SSAO_RADIUS = 0.4;
const float SSAO_BIAS = 0.02;
const vec3 AMBIENT_COLOR = vec3(0.1, 0.1, 0.2);

layout(location = 0) uniform sampler2D u_albedo;
layout(location = 1) uniform sampler2D u_normal;
layout(location = 2) uniform sampler2D u_position;
layout(location = 3) uniform sampler2D u_noise;

layout(location = 15) uniform vec3 u_lightPos;
layout(location = 20) uniform mat4 u_lightViewMat;
layout(location = 40) uniform mat4 u_lightProjMat;

layout(location = 60) uniform mat4 u_viewMat;
layout(location = 80) uniform mat4 u_projMat;

layout(location = 100) uniform vec2 u_noiseScale;
layout(location = 128) uniform vec3 u_ssaoSamples[SSAO_COUNT];

layout(location = 10) uniform bool u_enableAlbedo;
layout(location = 11) uniform bool u_enableSSAO;

in vec2 texCoords;

// out vec4 fb1;
// out vec4 fb2;
// out vec4 fb3;
// out vec4 fb4;
out vec4 fragColor;

// randomVec determines rotation of the base around Z axis => rotation of the ssao kernel
mat3 tbnMatrix(vec3 normal) {
	vec3 randomVec = texture(u_noise, texCoords * u_noiseScale).xyz;
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	return mat3(tangent, bitangent, normal);
}

float ssaoFactor(vec3 position, mat3 tbn) {
	float samplesOccluded = 0;

	for (int i = 0; i < SSAO_COUNT; ++i) {
		bool pointInInfinity = position == vec3(0);
		if (pointInInfinity) {
			samplesOccluded++;
			continue;
		}

		vec3 samplePos = position + tbn * u_ssaoSamples[i] * SSAO_RADIUS;
		vec4 samplePointPos = vec4(samplePos, 1);

		vec4 projectedSample = u_projMat * samplePointPos;
		projectedSample /= projectedSample.w;
		vec2 offset = projectedSample.xy * 0.5 + 0.5;

		vec4 sampleFragPos = texture(u_position, offset);
		// fb1 = sampleFragPos;
		// fb2 = samplePointPos;
		// fb3 = vec4(tbn * u_ssaoSamples[i], 1);

		if (sampleFragPos.z > samplePos.z - SSAO_BIAS) {
			float rangeCheck = smoothstep(0, 1, SSAO_RADIUS / abs(sampleFragPos.z - samplePointPos.z));
			samplesOccluded += rangeCheck;
		}
	}

	return 1 - samplesOccluded / float(SSAO_COUNT);
}

void main() {
	vec3 position = texture(u_position, texCoords).xyz;
	vec3 normal = texture(u_normal, texCoords).xyz;
	vec3 albedo = texture(u_albedo, texCoords).xyz;

	mat3 tbn = tbnMatrix(normal);
	float ssaoFact = 1;

	if (u_enableSSAO) {
		ssaoFact = ssaoFactor(position, tbn);
	}

	if (!u_enableAlbedo) {
		fragColor = vec4(ssaoFact);
		return;
	}

	vec3 ambientColor = AMBIENT_COLOR * ssaoFact;
	vec3 lightDir = normalize(u_lightPos - position);
	float lamb = max(dot(lightDir, normal), 0.0);
	
	fragColor = vec4(lamb * albedo + ambientColor, 1.0);
	// fragColor = vec4(ssaoFactor(position, tbn));


	vec4 shadowCoords = (u_lightProjMat * u_lightViewMat * vec4(position, 1.0));
	// shadowCoords are in light clipspace, but we get fragment relative 
	// coordinates in the shadowmap, so we need to remap to [0,1] interval in all dimensions
	// vec3 mappedShadowCoords = (shadowCoords.xyz/shadowCoords.w) * 0.5 + 0.5;
	// if (mappedShadowCoords.x > 0 && mappedShadowCoords.x < 1
	// 	&& mappedShadowCoords.y > 0 && mappedShadowCoords.y < 1) {
	// 	float shadow = texture(u_shadowMap, mappedShadowCoords.xy).x;
	// 	if (shadow < (mappedShadowCoords.z - 0.000001)) {
	// 		fragColor = vec4(0.5 * diffuseColor, 1.0);
	// 	}
	// }
}


