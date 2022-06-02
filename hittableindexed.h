#ifndef __HITTABLEINDEXED_H__
#define __HITTABLEINDEXED_H__

#include "globals.h"
#include "aabb.h"

struct hitRecordIndexed {
  vec4f p;          // 3 used
  vec4f normal;     // 3 used
  vec4f tangent;    // 3 used
  vec4f bitangent;  // 3 used
  vec4f uv;         // 2 used
  vec4f t;          // 1 used

  vec4i frontFace;  // 1 used
  vec4i matIndex;   // 1 used

  inline void setFaceNormal(const ray &r, const vec3f &outwardNormal) {
    frontFace[0] = r.dir.dot(outwardNormal) < 0 ? 1 : 0;
    //normal = frontFace ? outwardNormal : -outwardNormal;
  }
};

struct hittableIndexed {
  vec4f   positions[3];   // for triangle nodes
  vec4f   UVs[3];         // for triangle nodes
  vec4i   matIndex;       // for triangle nodes

  vec4f   boxMin, boxMax; // for bvh nodes
  vec4i   leftAndRight;   // for bvh nodes

/*  float   positions[3][4];   // for triangle nodes
  float   UVs[3][4];         // for triangle nodes
  int32_t   matIndex[4];       // for triangle nodes

  float   boxMin[4], boxMax[4]; // for bvh nodes
  int32_t   leftAndRight[4];   // for bvh nodes*/
};

#endif