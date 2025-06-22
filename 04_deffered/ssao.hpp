#include "scene_object.hpp"
#include <array>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <random>

class ssao {
public:
    static constexpr int SAMPLES_COUNT = 64;
    using kernel_t = std::array<glm::vec3, SAMPLES_COUNT>;

    static const kernel_t& kernel() { return _baseKernel; }

private:
    static size_t _seed;
    static std::mt19937 _rngGenerator;
    static std::normal_distribution<float> _pointsDistribution;
    static std::uniform_real_distribution<float> _rotationDistribution;

    static kernel_t _baseKernel;

    static void init() {
        _seed = std::random_device()();
        _rngGenerator = std::mt19937(_seed);
        _pointsDistribution = std::normal_distribution<float>(0, 0.3);
        _rotationDistribution = std::uniform_real_distribution<float>(0, 2 * glm::pi<float>());

        _baseKernel = createSamplePoints();
    }

    static kernel_t createSamplePoints() {
        kernel_t points;

        for (int i = 0; i < SAMPLES_COUNT; ++i) {
            points[i] = {
                _pointsDistribution(_rngGenerator),
                _pointsDistribution(_rngGenerator),
                std::abs(_pointsDistribution(_rngGenerator))
            };
        }

        return points;
    }
};