#version 430 core

layout(location = 0) uniform sampler2D u_ssaoBuffer;

in vec2 texCoords;

out float out_ssaoFactor;

const int KERNEL_SIZE = 25;
const float gaussian[KERNEL_SIZE] = {
    1.0,  4.0,  6.0,  4.0, 1.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    6.0, 24.0, 36.0, 24.0, 6.0,
    4.0, 16.0, 24.0, 16.0, 4.0,
    1.0,  4.0,  6.0,  4.0, 1.0
};

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(u_ssaoBuffer, 0));
    float sum = 0;

    for (int i = 0; i < KERNEL_SIZE; ++i) {
        vec2 offset = vec2(i / 5, i % 5) - vec2(2);
        float ssaoSample = texture(u_ssaoBuffer, texCoords + offset * texelSize).r;
        sum += ssaoSample * gaussian[i];
    }

    out_ssaoFactor = sum / 256.0;
}