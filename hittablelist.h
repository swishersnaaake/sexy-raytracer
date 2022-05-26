#ifndef __HITTABLELIST_H__
#define __HITTABLELIST_H__

#include "globals.h"
#include "hittable.h"
#include "aabb.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittableList : public hittable {
  public:
    hittableList() {}
    hittableList(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }

    virtual bool hit(const ray &ray, float tMin, float tMax, hitRecord &record) const override;
    virtual bool boundingBox(float time0, float time1, aabb& outputBox) const override;
    virtual void calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& biTangent) const override {}
  
  public:
    std::vector<shared_ptr<hittable>> objects;
};

bool hittableList::hit(const ray &ray, float tMin, float tMax, hitRecord &record) const {
  hitRecord   tempRecord;
  bool        hit = false;
  auto        closest = tMax;

  for (const auto &object : objects) {
    if (object->hit(ray, tMin, closest, tempRecord)) {
      hit = true;
      closest = tempRecord.t;
      record = tempRecord;
    }
  }

  return hit;
}

bool hittableList::boundingBox(float time0, float time1, aabb& outputBox) const {
  if (objects.empty())
    return false;

  aabb  tempBox;
  bool  firstBox = true;

  for (const auto& object : objects) {
    if (!object->boundingBox(time0, time1, tempBox))
      return false;

    outputBox = firstBox ? tempBox : surroundingBox(outputBox, tempBox);
    firstBox = false;
  }

  return true;
}

#endif