#include "error_handling.hpp"
#include <glad/glad.h>
#include <algorithm>
#include <array>
#include <glm/ext/scalar_constants.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <random>
#include <vector>

class ssao {
public:
    static constexpr int SAMPLES_COUNT = 64;
    static constexpr int NOISE_TEX_WIDHT = 4;
    static constexpr int NOISE_TEX_HEIGHT = 4;
    static constexpr int NOISE_SIZE = NOISE_TEX_HEIGHT * NOISE_TEX_WIDHT;

    using kernel_t = std::array<glm::vec3, SAMPLES_COUNT>;

    const kernel_t& kernel() const { return _baseKernel; }
    GLuint noiseTex() const { return _noiseTexture; }

    ssao() : 
        _seed(std::random_device()()), 
        _rngGenerator(_seed),
        _pointsDistribution(0, 0.4)
    {
        _baseKernel = createSamplePoints();
        createNoiseTexture();
    }

private:
    size_t _seed;
    std::mt19937 _rngGenerator;
    std::normal_distribution<float> _pointsDistribution;

    kernel_t _baseKernel;
    GLuint _noiseTexture;


    kernel_t createSamplePoints() {
        kernel_t points;

        for (int i = 0; i < SAMPLES_COUNT; ++i) {
            points[i] = {
                _pointsDistribution(_rngGenerator),
                _pointsDistribution(_rngGenerator),
                std::clamp(std::abs(_pointsDistribution(_rngGenerator)), 0.2f, 1.0f)
            };
        }

        return points;
    }

    void createNoiseTexture() {
        std::array<glm::vec3, NOISE_SIZE> noise;
        
        for (int i = 0; i < NOISE_SIZE; ++i) {
            noise[i] = {
                _pointsDistribution(_rngGenerator),
                _pointsDistribution(_rngGenerator),
                _pointsDistribution(_rngGenerator)
            };

            noise[i] = glm::normalize(noise[i]);
        }

        GL_CHECK(glGenTextures(1, &_noiseTexture));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, _noiseTexture));

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, NOISE_TEX_WIDHT, NOISE_TEX_HEIGHT, 0, GL_RGB, GL_FLOAT, &noise[0]));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    }
};