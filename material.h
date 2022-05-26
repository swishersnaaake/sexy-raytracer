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
    virtual bool    scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scatterRay) const = 0;
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
                          metalness(0), roughness(0), anisotropy(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mMap, shared_ptr<texture> rMap,
                          vec4f a) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicMap(mMap), roughnessMap(rMap),
                          albedo(a), metalness(0), roughness(0), anisotropy(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mrMap,
                          vec4f a) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicRoughnessMap(mrMap),
                          metallicMap(0), roughnessMap(0),
                          albedo(a), metalness(0), roughness(0), anisotropy(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap, shared_ptr<texture> nMap,
                          shared_ptr<texture> mrMap,
                          vec4f a, float m, float r) :
                          albedoMap(aMap), normalMap(nMap),
                          metallicRoughnessMap(mrMap),
                          metallicMap(0), roughnessMap(0),
                          albedo(a), metalness(m), roughness(r), anisotropy(0) {}
    pbrMetallicRoughness(shared_ptr<texture> aMap,
                          vec4f a, float m, float r) :
                          albedoMap(aMap), albedo(a),
                          metalness(m), roughness(r), anisotropy(0) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation,
                          ray& scatterRay) const override;

  public:
    shared_ptr<texture> albedoMap;
    shared_ptr<texture> normalMap;
    shared_ptr<texture> metallicRoughnessMap;
    shared_ptr<texture> metallicMap;
    shared_ptr<texture> roughnessMap;
    vec4f               albedo;
    float               metalness;
    float               roughness;
    float               anisotropy;
};

class metal : public material {
  public:
    metal(const color3f& a, float f) : albedo(a), fuzz(f < 1.0f ? f : 1.0f) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scatterRay) const override {
      vec3f reflected = reflect(unitVector(rIn.dir), record.normal);
      scatterRay = ray(record.p, reflected + fuzz * randomInUnitSphere(), rIn.time);
      attenuation = albedo;

      return (scatterRay.dir.dot(record.normal) > 0);
    }

  public:
    color3f albedo;
    float   fuzz;
};

class dielectric : public material {
  public:
    dielectric(float indexRefraction) : ir(indexRefraction) {}

    virtual bool scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scatterRay) const override {
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

      scatterRay = ray(record.p, dir, rIn.time);
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

    virtual bool    scatter(const ray& rIn, const hitRecord& record, color3f& attenuation, ray& scatterRay) const override {
      return false;
    }

    virtual color3f emitted(float u, float v, const vec3f& p) const override {
      return emit->value(u, v, p);
    }

  private:
    shared_ptr<texture> emit;
};

bool pbrMetallicRoughness::scatter(const ray& rIn, const hitRecord& record,
              color3f& attenuation, ray& scatterRay) const {
  vec3f halfVec;
  vec3f normal;
  float m;
  float r;

  if (albedoMap) {
    // sample reflected color for this point
    attenuation = albedoMap->value(record.uv(0), record.uv(1), record.p);
    attenuation /= 255.0f;
  }
  else
    attenuation = vec3f(albedo(0), albedo(1), albedo(2));
  
  if (normalMap) {
    // sample tangent space normal
    normal = normalMap->value(record.uv(0), record.uv(1), record.p);

    // convert to range -1 to 1
    normal = normalIntToFloat(normal);

    // construct tangent to world space xfrom
    Matrix3f tangentToWorld;
    tangentToWorld.col(0) = record.tangent;
    tangentToWorld.col(1) = record.bitangent;
    tangentToWorld.col(2) = record.normal;

    // xform tangent space normal to world space
    normal = unitVector(tangentToWorld * normal);
  }
  else
    normal = record.normal;

  if (metallicMap) {
    m = clamp(metallicMap->value(record.uv(0), record.uv(1), record.p)(0) / 255.0f, 0, 1.0f);
  }
  else
    m = metalness;

  if (roughnessMap) {
    r = clamp(roughnessMap->value(record.uv(0), record.uv(1), record.p)(1) / 255.0f, 0, 1.0f);
  }
  else
    r = roughness;
  
  // generate random scatter direction from surface normal
  vec3f  scatterDir = normal + randomUnitVector();

  if (nearZero(scatterDir))
    scatterDir = normal;
  
  scatterDir = unitVector(scatterDir);
  scatterRay = ray(record.p, scatterDir, rIn.time);

  // BRDF assumes view vector points towards camera
  vec3f viewVec = -unitVector(rIn.dir);

  // using scatter ray as effective light ray
  halfVec = unitVector(scatterRay.dir + viewVec);
  
  // begin PBR BRDF
  float NdotL = fmaxf(normal.dot(scatterRay.dir), 0);
  float NdotH = fmaxf(normal.dot(halfVec), 0);
  float HdotV = fmaxf(halfVec.dot(viewVec), 0);
  float NdotV = fmaxf(normal.dot(viewVec), 0);
  float HdotT; // used for anisotropic BRDFs
  float HdotB; // used for anisotropic BRDFs

  //vec3f fresnelReflectance = vec3f(1.0f, 1.0f, 1.0f);
  vec3f fresnelReflectance = vec3f(albedo(0), albedo(1), albedo(2));

  vec3f F0 = lerp(vec3f(0.4f, 0.4f, 0.4f), fresnelReflectance, m);
  float D = trowbridgeReitzNDF(NdotH, r);
  vec3f F = fresnelEpic(F0, HdotV);
  float G = schlickGAF(NdotL, r) * schlickGAF(NdotV, r);

  // final diffuse is a ratio vs specular
  vec3f finalDiffuse = (attenuation / pi);
  finalDiffuse(0) *= (1.0f - F(0));  finalDiffuse(1) *= (1.0f - F(1));  finalDiffuse(2) *= (1.0f - F(2));
  finalDiffuse *= (1.0f - m);
  finalDiffuse(0) *= albedo(0); finalDiffuse(1) *= albedo(1); finalDiffuse(2) *= albedo(2);

  vec3f finalSpecular = D * F * G / (4.0f * NdotV * NdotL + epsilon);

  // final attenuation *= cosine factor
  attenuation = vec3f((finalDiffuse + finalSpecular) * NdotL);

  return true;
}

#endif
