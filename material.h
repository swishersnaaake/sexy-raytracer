#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <cmath>
#include <algorithm>

#include "globals.h"
#include "texture.h"
#include "pbr.h"

using namespace Eigen;

struct hitRecord;

class material {
  public:
    virtual bool    scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scattered) const = 0;
    virtual color3f emitted(float u, float v, const vec3f& p) const {
      return color3f(0, 0, 0);
    }
};

class pbrMetallicRoughness : public material {
  public:
    pbrMetallicRoughness(const color3f& a) :
                          albedoMap(make_shared<solidColor>(a)),
                          normalMap(0),
                          albedo(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap) :
                          albedoMap(aMap),
                          normalMap(0),
                          albedo(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap,
                          vec4f a) :
                          albedoMap(aMap),
                          normalMap(0),
                          albedo(a) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap) :
                          albedoMap(aMap), normalMap(nMap),
                          albedo(vec4f(1.0f, 1.0f, 1.0f, 1.0f)) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mMap, shared_ptr<texture> rMap) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicMap(mMap), roughnessMap(rMap),
                          albedo(vec4f(1.0f, 1.0f, 1.0f, 1.0f)),
                          metalness(0), roughness(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mMap, shared_ptr<texture> rMap,
                          vec4f a) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicMap(mMap), roughnessMap(rMap),
                          albedo(a), metalness(0), roughness(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mrMap,
                          vec4f a) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicRoughnessMap(mrMap),
                          metallicMap(0), roughnessMap(0),
                          albedo(a), metalness(0), roughness(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mrMap,
                          vec4f a, float m, float r) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicRoughnessMap(mrMap),
                          metallicMap(0), roughnessMap(0),
                          albedo(a), metalness(m), roughness(r) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap,
                          vec4f a, float m, float r) :
                          albedoMap(aMap), albedo(a),
                          metalness(m), roughness(r) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation,
                          ray& scattered) const override {
      vec3f halfVec;
      vec3f normal;
      float m;
      float r;

      Matrix3f tangentToWorld;

      if (albedoMap) {
        // sample reflected color for this point
        attenuation = albedoMap->value(record.uv(0), record.uv(1), record.p);
        attenuation /= 255.0f;
      }
      else
        attenuation = vec3f(albedo(0), albedo(1), albedo(2));
      
      if (normalMap && record.blah) {
        // tangent space to world space xfrom
        vec3f tangent, bitangent, tangentNormal;

        calcTangentBasis(record.sourceV[0], record.sourceV[1], record.sourceV[2],
                          record.sourceUV[0], record.sourceUV[1], record.sourceUV[2],
                          tangent, bitangent);
        tangentNormal = tangent.cross(bitangent);
        tangent = unitVector(tangent);
        bitangent = unitVector(bitangent);
        tangentNormal = unitVector(tangentNormal);
        
        tangentToWorld.row(0) = tangent;
        tangentToWorld.row(1) = bitangent;
        tangentToWorld.row(2) = tangentNormal;

        tangentToWorld.transposeInPlace();

        // sample tangent space normal
        normal = normalMap->value(record.uv(0), record.uv(1), record.p);

        // convert to range -1 to 1
        normal(0) -= 128.0f; normal(1) -= 128.0f; normal(2) -= 128.0f;
        normal /= 128.0f;

        // xform tangent space normal to world space
        normal = tangentToWorld * normal;
      }
      else
        normal = record.normal;

      if (metallicMap) {
        float value = metallicMap->value(record.uv(0), record.uv(1), record.p)(0) / 255.0f;
        m = clamp(std::max(metalness + epsilon, value), 0, 1.0f);
      }
      else
        m = metalness;

      if (roughnessMap) {
        float value = roughnessMap->value(record.uv(0), record.uv(1), record.p)(0) / 255.0f;
        r = clamp(std::max(roughness + epsilon, value), 0, 1.0f);
      }
      else
        r = roughness;
      
      // initial scatter direction based on face normal
      vec3f  scatterDir = normal + randomUnitVector();

      if (nearZero(scatterDir))
        scatterDir = normal;
      
      scatterDir = unitVector(scatterDir);
      scattered = ray(record.p, scatterDir, rIn.time);

      vec3f viewVec = unitVector(rIn.dir);
      halfVec = unitVector(scattered.dir - viewVec);
      
      float NdotL = fmaxf(normal.dot(scattered.dir), 0);
      float NdotH = fmaxf(normal.dot(halfVec), 0);
      float HdotV = fmaxf(halfVec.dot(-viewVec), 0);
      float NdotV = fmaxf(normal.dot(-viewVec), 0);
      float HdotT;
      float HdotB;

      vec3f fresnelReflectance = vec3f(1.0f, 1.0f, 1.0f);

      vec3f F0 = lerp(vec3f(0.4f, 0.4f, 0.4f), fresnelReflectance, m);
      float D = trowbridgeReitzNDF(NdotH, r);
      vec3f F = fresnelEpic(F0, HdotV);
      float G = schlickGAF(NdotL, r) * schlickGAF(NdotV, r);

      vec3f finalDiffuse = (attenuation / pi);
      finalDiffuse(0) *= (1.0f - F(0));  finalDiffuse(1) *= (1.0f - F(1));  finalDiffuse(2) *= (1.0f - F(2));
      finalDiffuse *= (1.0f - m);

      finalDiffuse(0) *= albedo(0); finalDiffuse(1) *= albedo(1); finalDiffuse(2) *= albedo(2);

      vec3f finalSpecular = D * F * G / (4.0f * NdotV * NdotL + epsilon);

      attenuation = vec3f((finalDiffuse + finalSpecular) * NdotL);

      return true;
    }

  public:
    shared_ptr<texture> albedoMap;
    shared_ptr<texture> normalMap;
    shared_ptr<texture> metallicRoughnessMap;
    shared_ptr<texture> metallicMap;
    shared_ptr<texture> roughnessMap;
    vec4f               albedo;
    float               metalness;
    float               roughness;
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
