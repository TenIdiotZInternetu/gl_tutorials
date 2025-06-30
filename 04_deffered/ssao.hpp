#include <GL/gl.h>
#include <algorithm>
#include <array>
#include <glm/ext/scalar_constants.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <random>

class ssao {
public:
    static constexpr int SAMPLES_COUNT = 64;
    using kernel_t = std::array<glm::vec3, SAMPLES_COUNT>;

    const kernel_t& kernel() const { return _baseKernel; }

    ssao() : 
        _seed(std::random_device()()), 
        _rngGenerator(_seed),
        _pointsDistribution(0, 0.4) 
    {
        _baseKernel = createSamplePoints();
    }

private:
    size_t _seed;
    std::mt19937 _rngGenerator;
    std::normal_distribution<float> _pointsDistribution;
    kernel_t _baseKernel;


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
};