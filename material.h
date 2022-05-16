#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "globals.h"
#include "texture.h"

struct hitRecord;

class material {
  public:
    virtual bool    scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const = 0;
    virtual color3f emitted(float u, float v, const vec3f& p) const {
      return color3f(0, 0, 0);
    }
};

class lambertian : public material {
  public:
    lambertian(const color3f& a) : albedo(make_shared<solidColor>(a)), normalMap(0),
                                    color(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    lambertian(shared_ptr<texture> a) : albedo(a), normalMap(0),
                                        color(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    lambertian(shared_ptr<texture> a, vec4f c) : albedo(a), normalMap(0), color(c) {}
    lambertian(shared_ptr<texture> a, shared_ptr<texture> n) :
                                    albedo(a), normalMap(n),
                                    color(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    lambertian(shared_ptr<texture> a, shared_ptr<texture> n, vec4f c) :
                                    albedo(a), normalMap(n), color(c) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const override {
      // initial scatter direction based on face normal
      vec3f  scatterDir = record.normal + randomUnitVector();

      if (nearZero(scatterDir))
        scatterDir = record.normal;
      
      scatterDir = unitVector(scatterDir);
      scattered = ray(record.p, scatterDir, rIn.time);

      // sample reflected color for this point
      attenuation = albedo->value(record.uv(0), record.uv(1), record.p);

      if (0) {
        // tangent space to world space xfrom
        quatf tangentToWorld = quatf::FromTwoVectors(vec3f::UnitZ(), record.normal);

        // sample tangent space normal
        vec3f tangentNormal = normalMap->value(record.uv(0), record.uv(1), record.p);

        // convert to range -1 to 1
        tangentNormal(0) -= 128.0f; tangentNormal(1) -= 128.0f; tangentNormal(2) -= 128.0f;
        tangentNormal /= 128.0f;

        // xform tangent space normal to world space
        tangentNormal = tangentToWorld * tangentNormal;

        // face normal to pixel normal xform
        quatf faceToNormal = quatf::FromTwoVectors(record.normal, tangentNormal);

        // transform scatter direction by pixel normal
        scattered.dir = faceToNormal * scattered.dir;
      }
      attenuation /= 255.0f;
      attenuation = vec3f(attenuation(0) * color(0), attenuation(1) * color(1),
                          attenuation(2) * color(2));

      return true;
    }

  public:
    vec4f               color;
    shared_ptr<texture> albedo;
    shared_ptr<texture> normalMap;
};

class metal : public material {
  public:
    metal(const color3f& a, float f) : albedo(a), fuzz(f < 1.0f ? f : 1.0f) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const override {
      vec3f reflected = reflect(unitVector(rIn.dir), record.normal);
      scattered = ray(record.p, reflected + fuzz * randomInUnitSphere(), rIn.time);
      attenuation = albedo;

      return (scattered.dir.dot(record.normal) > 0);
    }

  public:
    color3f albedo;
    float   fuzz;
};

class dielectric : public material {
  public:
    dielectric(float indexRefraction) : ir(indexRefraction) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const override {
      attenuation = color3f(1.0f, 1.0f, 1.0f);
      float refractionRatio = record.frontFace ? (1.0f / ir) : ir;

      vec3f unitDir = unitVector(rIn.dir);
      float cosTheta = fmin(record.normal.dot(-unitDir), 1.0f);
      float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

      bool  cannotRefract = refractionRatio * sinTheta > 1.0f;
      vec3f dir;

      if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomFloat())
        dir = reflect(unitDir, record.normal);
      else
        dir = refract(unitDir, record.normal, refractionRatio);

      scattered = ray(record.p, dir, rIn.time);
      return true;
    }
  
  public:
    float ir;

  private:
    static float reflectance(float cosine, float refIDX) {
      auto r0 = (1.0f - refIDX) / (1.0f + refIDX);
      r0 = r0 * r0;
      return r0 + (1.0f - r0) * pow((1.0f - cosine), 5.0f);
    }
};

class diffuseLight : public material {
  public:
    diffuseLight(shared_ptr<texture> a) : emit(a) {}
    diffuseLight(color3f c) : emit(make_shared<solidColor>(c)) {}

    virtual bool    scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const override {
      return false;
    }

    virtual color3f emitted(float u, float v, const vec3f& p) const override {
      return emit->value(u, v, p);
    }

  private:
    shared_ptr<texture> emit;
};

#endif
