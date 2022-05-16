#ifndef __BVH_H__
#define __BVH_H__

#include <algorithm>

#include "globals.h"
#include "hittable.h"
#include "hittablelist.h"

class bvhNode : public hittable {
  public:
    bvhNode() {}
    bvhNode(const hittableList& list, float time0, float time1) :
      bvhNode(list.objects, 0, list.objects.size(), time0, time1) {}
    
    bvhNode(const std::vector<shared_ptr<hittable>>& srcObjects,
            size_t start, size_t end, float time0, float time1);
    
    virtual bool  hit(const ray& r, float tMin, float tMax, hitRecord& record) const override;
    virtual bool  boundingBox(float time0, float time1, aabb& outputBox) const override;

  public:
    shared_ptr<hittable>  left;
    shared_ptr<hittable>  right;
    aabb                  box;
};

inline bool boxCompare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
  aabb  boxA, boxB;

  if (!a->boundingBox(0, 0, boxA) || !b->boundingBox(0, 0, boxB))
    std::cerr << "No bounding box in bvhNode constructor.\n";
  
  return boxA.minimum(axis) < boxB.minimum(axis);
}

bool boxXCompare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return boxCompare(a, b, 0);
}

bool boxYCompare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return boxCompare(a, b, 1);
}

bool boxZCompare(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
  return boxCompare(a, b, 2);
}

bvhNode::bvhNode(const std::vector<shared_ptr<hittable>>& srcObjects,
                  size_t start, size_t end, float time0, float time1) {
  auto  objects = srcObjects;

  //int     axis = objects[0]->selectBvhAxis();
  int     axis = randomInt(0, 2);
  auto    comparator = (axis == 0) ? boxXCompare
                      : (axis == 1) ? boxYCompare
                      : boxZCompare;
  
  size_t  objectSpan = end - start;

  if (objectSpan == 1) {
    left = right = objects[start];
  }
  else if (objectSpan == 2) {
    if (comparator(objects[start], objects[start + 1])) {
      left = objects[start];
      right = objects[start + 1];
    }
    else {
      left = objects[start + 1];
      right = objects[start];
    }
  }
  else {
    std::sort(objects.begin() + start, objects.begin() + end, comparator);

    auto  mid = start + objectSpan / 2;
    left = make_shared<bvhNode>(objects, start, mid, time0, time1);
    right = make_shared<bvhNode>(objects, mid, end, time0, time1);
  }

  aabb  boxLeft, boxRight;

  if (!left->boundingBox(time0, time1, boxLeft) ||
      !right->boundingBox(time0, time1, boxRight))
      std::cerr << "No bounding box in bvh node constructor.\n";

  //shared_ptr<triangle> triLeft = static_cast<shared_ptr<triangle>>(left);
  //shared_ptr<triangle> triRight = static_cast<shared_ptr<triangle>>(right);
  box = surroundingBox(boxLeft, boxRight);
}

bool bvhNode::hit(const ray& r, float tMin, float tMax, hitRecord& record) const {
  if (!box.hit(r, tMin, tMax))
    return false;

  bool  bHitLeft = left->hit(r, tMin, tMax, record);
  bool  bHitRight = right->hit(r, tMin, bHitLeft ? record.t : tMax, record);

  return bHitLeft || bHitRight;
}

bool bvhNode::boundingBox(float time0, float time1, aabb &outputBox) const {
  outputBox = box;
  return true;
}

#endif