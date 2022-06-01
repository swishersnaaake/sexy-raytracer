#ifndef __VEC3_H__
#define __VEC3_H__

#include <cmath>
#include <iostream>

#include "Eigen/Core"

using std::int32_t;

using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;
//using vec4i = Eigen::Vector4i;
using vec4i = Eigen::Matrix<int32_t, 4, 1>; // wtf is the size of an int in Eigen?
using quatf = Eigen::Quaternionf;
using color3f = Eigen::Vector3f;
using color4f = Eigen::Vector4f;

using std::sqrt;
using std::fabs;

// vec3 utility functions

//inline std::ostream& operator<<(std::ostream out, const vec3 &v) {
//  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
//}

float lengthSquared(const vec3f& v) {
  return v(0) * v(0) + v(1) * v(1) + v(2) * v(2);
}

float length(const vec3f& v) {
  return sqrtf(lengthSquared(v));
}

float distance(const vec3f& u, const vec3f& v) {
  return sqrtf(lengthSquared(vec3f(u - v)));
}

inline vec3f randomVec3f() {
  return vec3f(randomFloat(), randomFloat(), randomFloat());
}

inline vec3f randomVec3f(float min, float max) {
  return vec3f(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
}

bool nearZero(const vec3f& v) {
  const auto s = 1e-8;
  return (fabs(v(0)) < s) && (fabs(v(1)) < s) && (fabs(v(2)) < s);
}

inline vec3f unitVector(const vec3f &v) {
  float len = length(v);
  if (len != 0)
    return vec3f(v(0) / len, v(1) / len, v(2) / len);
  else
    return v;
}

inline vec3f randomInUnitSphere() {
  while (true) {
    auto  p = vec3f(randomVec3f(-1.0f, 1.0f));
    if (lengthSquared(p) >= 1.0f)
      continue;
    
    return p;
  }
}

inline vec3f randomUnitVector() {
  return unitVector(randomInUnitSphere());
}

inline vec3f reflect(const vec3f& v, const vec3f& n) {
  return v - 2.0f * v.dot(n) * n;
}

inline vec3f refract(const vec3f& uv, const vec3f& n, float etaIOverEtaiT) {
  auto  cosTheta = fmin(n.dot(-uv), 1.0f);
  vec3f rOutPerp = etaIOverEtaiT * (uv + cosTheta * n);
  vec3f rOutParallel = -sqrtf(fabs(1.0f - lengthSquared(rOutPerp))) * n;

  return rOutPerp + rOutParallel;
}

inline vec3f randomInUnitDisk() {
  while(true) {
    auto  p = vec3f(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f), 0);
    if (lengthSquared(p) >= 1.0f)
      continue;
    return p;
  }
}

inline vec3f lerp(const vec3f& a, const vec3f& b, float t) {
  return vec3f((1.0f - t) * a(0) + t * b(0),
                (1.0f - t) * a(1) + t * b(1),
                (1.0f - t) * a(2) + t * b(2));
}

inline vec3f normalIntToFloat(const vec3f& n) {
  vec3f normal;

  normal(0) = n(0) - 128.0f;  normal(1) = n(1) - 128.0f;  normal(2) = n(2) - 128.0f;
  normal /= 128.0f;

  return normal;
}

bool isNan(const vec2f& v) {
  if (!(v(0) == v(0)) ||
        !(v(1) == v(1)))
    return true;
  
  return false;
}

bool isNan(const vec3f& v) {
  if (!(v(0) == v(0)) ||
        !(v(1) == v(1)) ||
        !(v(2) == v(2)))
    return true;
  
  return false;
}

#endif