#ifndef __HITTABLE_H__
#define __HITTABLE_H__

#include "globals.h"
#include "aabb.h"

class material;

struct hitRecord {
  vec3f   p;
  vec3f   normal;
  float   t;
  vec2f   uv;
  bool    frontFace;
  bool    blah;
  
  vec3f   sourceV[3];
  vec2f   sourceUV[3];

  shared_ptr<material> matPtr;

  inline void setFaceNormal(const ray &r, const vec3f &outwardNormal) {
    frontFace = r.dir.dot(outwardNormal) < 0;
    normal = frontFace ? outwardNormal : -outwardNormal;
  }
};

class hittable {
  public:
    virtual bool  hit(const ray &r, float tMin, float tMax, hitRecord &record) const = 0;
    virtual bool  boundingBox(float time0, float time1, aabb& outputBox) const = 0;
    virtual int   selectBvhAxis() const { return randomInt(0, 2); };
};

#endif