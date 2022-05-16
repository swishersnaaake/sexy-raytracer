#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <cmath>
#include <limits>
#include <memory>
#include <random>

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

const float infinity = std::numeric_limits<float>::infinity();
const float epsilon = std::numeric_limits<float>::epsilon();
const float pi = 3.1415926535897932385f;

inline float clamp(float x, float min, float max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  
  return x;
}

inline float deg2rad(float degrees) {
  return degrees * pi / 180.0f;
}

inline float randomFloat() {
  static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
  static std::mt19937 generator;

  return distribution(generator);
}

inline float randomFloat(float min, float max) {
  return min + (max - min) * randomFloat();
}

inline int randomInt(int min, int max) {
  return static_cast<int>(randomFloat(min, max + 1));
}

#include "ray.h"
#include "vec3.h"

//using mat44f = Eigen::Matrix<float, 4, 4, Eigen::RowMajor>;

#endif