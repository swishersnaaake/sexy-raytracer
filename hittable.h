#ifndef __HITTABLE_H__
#define __HITTABLE_H__

#include "globals.h"
#include "aabb.h"

class material;

struct hitRecord {
  vec3f   p;
  vec3f   normal, tangent, bitangent;
  float   t;
  vec2f   uv;
  bool    frontFace;

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
    virtual void  calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& biTangent) const = 0;
    virtual int   selectBvhAxis() const { return randomInt(0, 2); }
};

#endif