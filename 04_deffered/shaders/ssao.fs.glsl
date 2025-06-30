#version 430 core

const int SSAO_COUNT = 64;
const float SSAO_RADIUS = 0.4;
const float SSAO_BIAS = 0.02;

layout(location = 1) uniform sampler2D u_normal;
layout(location = 2) uniform sampler2D u_position;
layout(location = 3) uniform sampler2D u_noise;

layout(location = 11) uniform bool u_enableSSAO;

layout(location = 60) uniform mat4 u_viewMat;
layout(location = 80) uniform mat4 u_projMat;

layout(location = 100) uniform vec2 u_noiseScale;
layout(location = 128) uniform vec3 u_ssaoSamples[SSAO_COUNT];

in vec2 texCoords;

out float out_ssaoBuffer;


mat3 tbnMatrix() {
	vec3 randomVec = texture(u_noise, texCoords * u_noiseScale).xyz;

    vec3 normal = texture(u_normal, texCoords).xyz;
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);

	return mat3(tangent, bitangent, normal);
}

void main() {
    if (!u_enableSSAO) {
        out_ssaoBuffer = 1;
        return;
    }
    
    vec3 position = texture(u_position, texCoords).xyz;
    mat3 tbn = tbnMatrix();

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

		if (sampleFragPos.z > samplePos.z - SSAO_BIAS) {
			float rangeCheck = smoothstep(0, 1, SSAO_RADIUS / abs(sampleFragPos.z - samplePointPos.z));
			samplesOccluded += rangeCheck;
		}
	}

	out_ssaoBuffer = 1 - samplesOccluded / float(SSAO_COUNT);
}