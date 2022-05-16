#ifndef __RAY_H__
#define __RAY_H__

#include "vec3.h"

class ray {
  public:
    ray() {}
    ray(const vec3f &origin, const vec3f &direction, float t = 0) : o(origin), dir(direction), time(t) {}

//    vec3f origin() const { return o; }
//    vec3f   direction() const { return dir; }
//    float   time() const { return tm; }

    vec3f at(float t) const {
      return o + t * dir;
    }
    
  public:
    vec3f o;
    vec3f dir;
    float time;
};

#endif