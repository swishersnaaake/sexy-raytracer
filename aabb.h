#ifndef __AABB_H__
#define __AABB_H__

#include "globals.h"

class aabb {
  public:
    aabb() {}
    aabb(const vec3f& a, const vec3f& b) : minimum(a), maximum(b) {}

    bool hit(const ray& r, float tMin, float tMax) const {
      vec3f nDir = unitVector(r.dir);
      for (int axis = 0; axis < 3; axis++) {
        auto  t0 = fminf((minimum(axis) - r.o(axis)) / r.dir(axis),
                          (maximum(axis) - r.o(axis)) / r.dir(axis));
        auto  t1 = fmaxf((minimum(axis) - r.o(axis)) / r.dir(axis),
                          (maximum(axis) - r.o(axis)) / r.dir(axis));
        
        tMin = fmaxf(t0, tMin);
        tMax = fminf(t1, tMax);

        if (tMax <= tMin)
          return false;
      }

      return true;
    }

  public:
    vec3f minimum, maximum;
};

aabb surroundingBox(aabb box0, aabb box1) {
  vec3f small(fminf(box0.minimum(0), box1.minimum(0)),
              fminf(box0.minimum(1), box1.minimum(1)),
              fminf(box0.minimum(2), box1.minimum(2)));
  
  vec3f large(fmaxf(box0.maximum(0), box1.maximum(0)),
              fmaxf(box0.maximum(1), box1.maximum(1)),
              fmaxf(box0.maximum(2), box1.maximum(2)));
  
  return aabb(small, large);
}

#endif