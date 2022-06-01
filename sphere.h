#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "globals.h"
#include "hittable.h"
#include "hittablevector.h"

class sphere : public hittable {
  public:
    sphere() {}
    sphere(vec3f c0, vec3f c1,
            float time0, float time1,
            float r, shared_ptr<material> m) : center0(c0), center1(c1),
            t0(time0), t1(time1),
            radius(r), matPtr(m) {};

    virtual bool  hit(const ray &ray, float tMin, float tMax, hitRecord &record) const override;
    virtual bool  boundingBox(float time0, float time1, aabb& outputBox) const override;
    virtual void  calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& bitangent) const override;

    virtual int   populateVector(shared_ptr<class hittableVector> hittableVector) const override {
        int index = hittableVector->objects.size();
        hittableVector->objects.emplace_back();
        hittableVector->objects[index].positions[0] = vec4f(999.0f, 999.0f, 999.0f, 1.0f);
        return index;
      }

    vec3f         center(float time) const;

  private:
    static void getSphereUV(const vec3f& p, vec2f& uv) {
      auto  theta = acosf(-p(1));
      auto  phi = atan2f(-p(2), p(0)) + pi;

      uv(0) = phi / (2.0f * pi);
      uv(1) = theta / pi;
    }
  
  public:
    vec3f                 center0, center1;
    float                 t0, t1;
    float                 radius;
    shared_ptr<material>  matPtr;
};

vec3f sphere::center(float time) const {
  if (center0 != center1)
    return center0 + ((time - t0) / (t1 - t0)) * (center1 - center0);
  else
    return center0;
}

bool sphere::hit(const ray &ray, float tMin, float tMax, hitRecord &record) const {
  vec3f oc = ray.o - center(ray.time);
  auto  a = lengthSquared(ray.dir);
  auto  halfB = oc.dot(ray.dir);
  auto  c = lengthSquared(oc) - radius * radius;
  auto  discriminant = halfB * halfB - a * c;

  if (discriminant < 0.0f)
    return false;
  
  auto  sqrtd = sqrtf(discriminant);

  // find nearest root in range
  auto    root = (-halfB - sqrtd) / a;
  if (root < tMin || root > tMax) {
    root = (-halfB + sqrtd) / a;
    if (root < tMin || root > tMax)
      return false;
  }

  record.t = root;
  record.p = ray.at(record.t);
  vec3f outwardNormal = unitVector(record.p - center(ray.time));// / radius;
  record.setFaceNormal(ray, outwardNormal);
  getSphereUV(outwardNormal, record.uv);
  record.matPtr = matPtr;
  calcTangentBasis(outwardNormal, record.tangent, record.bitangent);

  return true;
}

bool sphere::boundingBox(float time0, float time1, aabb& outputBox) const {
  aabb  box0(center(time0) - vec3f(radius, radius, radius),
              center(time0) + vec3f(radius, radius, radius));
  aabb  box1(center(time1) - vec3f(radius, radius, radius),
              center(time1) + vec3f(radius, radius, radius));

  outputBox = surroundingBox(box0, box1);
  
  return true;
}

void sphere::calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& bitangent) const {
  vec3f b;

  if (1.0f - (fabsf(normal.dot(vec3f::UnitY()))) < epsilon)
    b = -vec3f::UnitZ();
  else
    b = vec3f::UnitY();
  
  tangent = unitVector(b.cross(normal));
  bitangent = unitVector(normal.cross(tangent));
}

#endif