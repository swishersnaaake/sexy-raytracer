#ifndef __HITTABLE_VECTOR_H__
#define __HITTABLE_VECTOR_H__

#include "globals.h"
#include "hittablelist.h"
#include "hittableindexed.h"

#include <vector>

class hittableVector : public std::enable_shared_from_this<hittableVector> {
  public:
    shared_ptr<hittableVector> getPtr() { return shared_from_this(); }
    [[nodiscard]] static shared_ptr<hittableVector> create() {
      return shared_ptr<hittableVector>(new hittableVector());
    }

    void clear() { objects.clear(); }
    void build(hittableList& list);

  private:
    hittableVector() {}

  public:
    std::vector<hittableIndexed> objects;
};

void hittableVector::build(hittableList& list) {
  for (const auto &object : list.objects) {
    object->populateVector(getPtr());
  }
}

#endif
