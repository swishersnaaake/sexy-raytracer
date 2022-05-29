#ifndef __MODEL_H__
#define __MODEL_H__

#include "cgltf.h"

#include "globals.h"
#include "hittable.h"
#include "material.h"

using std::vector;
using std::uint16_t;
using std::uint32_t;

bool gltfLoad(std::string filename, shared_ptr<class model> model);

class triangle : public hittable, public std::enable_shared_from_this<triangle> {
  public:
    shared_ptr<triangle> getPtr() { return shared_from_this(); }
    [[nodiscard]] static shared_ptr<triangle> create(uint16_t index0, uint16_t index1,
                                                      uint16_t index2, shared_ptr<class mesh> srcMesh) {
      return shared_ptr<triangle>(new triangle(index0, index1, index2, srcMesh));
    }

    virtual bool  hit(const ray &r, float tMin, float tMax, hitRecord &record) const override;
    virtual bool  boundingBox(float time0, float time1, aabb& outputBox) const override;
    virtual void  calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& biTangent) const override;
    virtual int   selectBvhAxis() const override;

    inline vec3f getNormal() const;

  private:
    triangle() {}
    triangle(uint16_t index0, uint16_t index1, uint16_t index2,
            shared_ptr<class mesh> srcMesh) : parentMesh(srcMesh) {
      vertices[0] = index0;
      vertices[1] = index1;
      vertices[2] = index2;
    }

  private:
    uint16_t                vertices[3];
    shared_ptr<class mesh>  parentMesh;
};

// primitive in glTF
class mesh : public hittable, public std::enable_shared_from_this<mesh> {
  public:
    shared_ptr<mesh> getPtr() { return shared_from_this(); }
    [[nodiscard]] static shared_ptr<mesh> create() {
      return shared_ptr<mesh>(new mesh());
    }
    
    virtual bool  hit(const ray &r, float tMin, float tMax, hitRecord &record) const override;
    virtual bool  boundingBox(float time0, float time1, aabb& outputBox) const override;
    virtual void  calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& biTangent) const override {}
    
  private:
    mesh() {}

  public:
    std::vector<shared_ptr<triangle>>   triangles;
    std::vector<vec3f>                  positions;
    std::vector<vec2f>                  texcoords;
    shared_ptr<material>                matPtr;
    shared_ptr<class model>             parentModel;
};

class model : public hittable, public std::enable_shared_from_this<model> {
  public:
    shared_ptr<model> getPtr() { return shared_from_this(); }
    [[nodiscard]] static shared_ptr<model> create(std::string fn) {
      return shared_ptr<model>(new model(fn));
    }

    virtual bool hit(const ray &r, float tMin, float tMax, hitRecord &record) const override;
    virtual bool boundingBox(float time0, float time1, aabb& outputBox) const override;
    virtual void calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& biTangent) const override {}

    bool         init() {
      return gltfLoad(filename, getPtr());
    }

  private:
    model(std::string fn) : filename(fn) {}
  
  public:
    std::string                   filename;
    std::vector<shared_ptr<mesh>> meshes;
};

bool triangle::hit(const ray &ray, float tMin, float tMax, hitRecord &record) const {
  vec3f  sourceV[3];
  vec2f  sourceUV[3];
  
  for (int i = 0; i < 3; i++) {
    sourceV[i] = parentMesh->positions[vertices[i]];
    sourceUV[i] = parentMesh->texcoords[vertices[i]];
  }

  vec3f normal = getNormal();
  float area = length(normal);

  float NdotDir = normal.dot(ray.dir);

  // check if parallel
  if (fabsf(NdotDir) < epsilon)
    return false;
  
  if (ray.dir.dot(normal) > 0)
    return false;
  
  float d = -normal.dot(sourceV[0]);
  float t = -(normal.dot(ray.o) + d) / NdotDir;

  if (t < tMin)
    return false;

  vec3f p = ray.o + t * ray.dir;
  vec3f cross;
  float u, v;

  // edge 0
  vec3f edge0 = sourceV[1] - sourceV[0];
  vec3f vp0 = p - sourceV[0];
  cross = edge0.cross(vp0);
  if (normal.dot(cross) < 0)
    return false;
  
  // edge 1
  vec3f edge1 = sourceV[2] - sourceV[1];
  vec3f vp1 = p - sourceV[1];
  cross = edge1.cross(vp1);
  if (normal.dot(cross) < 0)
    return false;
  
  // edge 2
  vec3f edge2 = sourceV[0] - sourceV[2];
  vec3f vp2 = p - sourceV[2];
  cross = edge2.cross(vp2);
  if (normal.dot(cross) < 0)
    return false;

  // passed hit test
  // generate barycentric coordinates
  float d0, d1, d2, r0, r1, r2;
  d0 = distance(p, sourceV[0]);
  d1 = distance(p, sourceV[1]);
  d2 = distance(p, sourceV[2]);

  float denom = (1.0f / d0) + (1.0f / d1) + (1.0f / d2);
  r0 = (1.0f / d0) / denom;
  r1 = (1.0f / d1) / denom;
  r2 = (1.0f / d2) / denom;

  u = r0 * sourceUV[0](0) + r1 * sourceUV[1](0) + r2 * sourceUV[2](0);
  v = 1.0f - (r0 * sourceUV[0](1) + r1 * sourceUV[1](1) + r2 * sourceUV[2](1));

  // blah outwardNormal redundant here
  vec3f outwardNormal = unitVector(normal);
  record.t = t;
  record.p = ray.at(record.t);
  record.setFaceNormal(ray, outwardNormal);
  record.uv = vec2f(u, v);
  record.matPtr = parentMesh->matPtr;
  calcTangentBasis(outwardNormal, record.tangent, record.bitangent);

  return true;
}

bool triangle::boundingBox(float time0, float time1, aabb& outputBox) const {
  vec3f min(infinity, infinity, infinity);
  vec3f max(-infinity, -infinity, -infinity);

  vec3f v0 = parentMesh->positions[vertices[0]];
  vec3f v1 = parentMesh->positions[vertices[1]];
  vec3f v2 = parentMesh->positions[vertices[2]];

  for (const auto& vertex : vertices) {
    for (int axis = 0; axis < 3; axis++) {

      min(axis) = std::min(min(axis), parentMesh->positions[vertex](axis));
      max(axis) = std::max(max(axis), parentMesh->positions[vertex](axis));
    }
  }

  for (int axis = 0; axis < 3; axis++) {
    if (min(axis) == max(axis)) {
      min(axis) -= 0.0001f;
      max(axis) += 0.0001f;
    }
  }

  aabb  box0(min, max);
  aabb  box1(min, max);

  outputBox = surroundingBox(box0, box1);

  return true;
}

void triangle::calcTangentBasis(const vec3f& normal, vec3f& tangent, vec3f& bitangent) const {
  vec3f edge0 = parentMesh->positions[vertices[1]] - parentMesh->positions[vertices[0]];
  vec3f edge1 = parentMesh->positions[vertices[2]] - parentMesh->positions[vertices[0]];
  vec2f deltaUV0 = parentMesh->texcoords[vertices[1]] - parentMesh->texcoords[vertices[0]];;
  vec2f deltaUV1 = parentMesh->texcoords[vertices[2]] - parentMesh->texcoords[vertices[0]];;

  float f = (deltaUV0(0) * deltaUV1(1) - deltaUV1(0) * deltaUV0(1));
  if (f == 0)
    f += epsilon;
  
  f = 1.0f / f;

  tangent(0) = f * (deltaUV1(1) * edge0(0) - deltaUV0(1) * edge1(0));
  tangent(1) = f * (deltaUV1(1) * edge0(1) - deltaUV0(1) * edge1(1));
  tangent(2) = f * (deltaUV1(1) * edge0(2) - deltaUV0(1) * edge1(2));
  tangent = unitVector(tangent);

  bitangent(0) = f * (-deltaUV1(0) * edge0(0) + deltaUV0(0) * edge1(0));
  bitangent(1) = f * (-deltaUV1(0) * edge0(1) + deltaUV0(0) * edge1(1));
  bitangent(2) = f * (-deltaUV1(0) * edge0(2) + deltaUV0(0) * edge1(2));
  bitangent = unitVector(bitangent);
}

int triangle::selectBvhAxis() const {
  return randomInt(0, 2);

/*  vec3f n = getNormal();
  int   axis = (fabsf(n(0)) > fabsf(n(1)) && fabsf(n(0)) > fabsf(n(2))) ? 1 :
                (fabsf(n(1)) > fabsf(n(0)) && fabsf(n(1)) > fabsf(n(2))) ? 2 :
                (fabsf(n(2)) > fabsf(n(0)) && fabsf(n(2)) > fabsf(n(1))) ? 0 :
                4;
  
  return axis;*/
}

inline vec3f triangle::getNormal() const {
  vec3f a = parentMesh->positions[vertices[1]] -
            parentMesh->positions[vertices[0]];
  vec3f b = parentMesh->positions[vertices[2]] -
            parentMesh->positions[vertices[0]];
  
  return a.cross(b);
}

bool mesh::hit(const ray &ray, float tMin, float tMax, hitRecord &record) const {
  return true;
}

bool mesh::boundingBox(float time0, float time1, aabb& outputBox) const {
  return true;
}

bool model::hit(const ray &ray, float tMin, float tMax, hitRecord &record) const {
  return true;
}

bool model::boundingBox(float time0, float time1, aabb& outputBox) const {
  return true;
}

bool gltfLoad(std::string filename, shared_ptr<model> model) {
  cgltf_options options = {static_cast<cgltf_file_type>(0)};
  cgltf_data*   data;

  // read in file
  cgltf_result  result = cgltf_parse_file(&options, filename.c_str(), &data);

  if (result != cgltf_result_success)
    return false;

  // parse and load json buffer data
  result = cgltf_load_buffers(&options, data, filename.c_str());

  if (result != cgltf_result_success)
    return false;

  // for each scene
  // for each node
  // for each mesh
  for (int meshIndex = 0; meshIndex < data->meshes_count; meshIndex++) {
    cgltf_mesh*   gltfMesh = &(data->meshes[meshIndex]);

    // for each primitive->accessor->buffer view->buffer
    for (int primIndex = 0; primIndex < gltfMesh->primitives_count; primIndex++) {
      cgltf_primitive*  gltfPrim = &(gltfMesh->primitives[primIndex]);
      int               numAttributes = gltfPrim->attributes_count;

      shared_ptr<mesh>  newMesh = mesh::create();
      model->meshes.push_back(newMesh);

      // read in raw attribute data
      for (int attrIndex = 0; attrIndex < gltfPrim->attributes_count; attrIndex++) {
        cgltf_attribute*  attribute = &(gltfPrim->attributes[attrIndex]);
        
        if (attribute->type == cgltf_attribute_type_position) {
          cgltf_accessor*     a = attribute->data;
          cgltf_buffer_view*  bufferView = a->buffer_view;
          cgltf_buffer*       buffer = bufferView->buffer;

          uint8_t*  byte = (uint8_t*)buffer->data;

          if (a->type == cgltf_type_vec3) {
            vec3f*    value = (vec3f*)&(byte[bufferView->offset]);
            for (int offset = 0; offset < a->count; offset++) {
              model->meshes[primIndex]->positions.push_back(*value);
              value++;
            }
          }
        }

        if (attribute->type == cgltf_attribute_type_texcoord) {
          cgltf_accessor*     a = attribute->data;
          cgltf_buffer_view*  bufferView = a->buffer_view;
          cgltf_buffer*       buffer = bufferView->buffer;

          uint8_t*  byte = (uint8_t*)buffer->data;

          if (a->type == cgltf_type_vec2) {
            vec2f*    value = (vec2f*)&(byte[bufferView->offset]);
            for (int offset = 0; offset < a->count; offset++) {
              model->meshes[primIndex]->texcoords.push_back(*value);
              value++;
            }
          }
        }
      }

      // read in material
      cgltf_material* gltfMat = gltfPrim->material;
      if (gltfMat) {
        if (gltfMat->has_pbr_metallic_roughness) {
          cgltf_texture*      albedoMap = nullptr;
          cgltf_texture*      normalMap = nullptr;
          cgltf_texture*      metallicRoughnessMap = nullptr;
          cgltf_image*        albedoImage = nullptr;
          cgltf_image*        normalImage = nullptr;
          cgltf_image*        metallicRoughnessImage = nullptr;

          std::string textureFile;
          std::string normalMapFile;
          std::string metallicRoughnessMapFile;

          vec4f baseColor;
          float metallicness, roughness;

          cgltf_pbr_metallic_roughness* pbrMat = &gltfMat->pbr_metallic_roughness;
          cgltf_texture_view*           albedoView = &pbrMat->base_color_texture;
          cgltf_texture_view*           normalMapView = &gltfMat->normal_texture;
          cgltf_texture_view*           metallicRoughnessView = &pbrMat->metallic_roughness_texture;

          if (albedoView->texture) {
            albedoMap = albedoView->texture;
            albedoImage = albedoMap->image;

            textureFile = "../data/";
            textureFile.append(albedoImage->uri);
          }

          if (normalMapView->texture) {
            normalMap = normalMapView->texture;
            normalImage = normalMap->image;

            normalMapFile = "../data/";
            normalMapFile.append(normalImage->uri);
          }

          if (metallicRoughnessView->texture) {
            metallicRoughnessMap = metallicRoughnessView->texture;
            metallicRoughnessImage = metallicRoughnessMap->image;

            metallicRoughnessMapFile = "../data/";
            metallicRoughnessMapFile.append(metallicRoughnessImage->uri);
          }

          baseColor = vec4f(pbrMat->base_color_factor[0], pbrMat->base_color_factor[1],
                            pbrMat->base_color_factor[2], pbrMat->base_color_factor[3]);
          metallicness = pbrMat->metallic_factor;
          roughness = pbrMat->roughness_factor;

          shared_ptr<imagePNG>  albedoPNG;
          shared_ptr<imagePNG>  normalPNG;
          shared_ptr<imagePNG>  metallicRoughnessPNG;

          if (!textureFile.empty())
            albedoPNG = make_shared<imagePNG>(textureFile.c_str(), 3);

          if (!normalMapFile.empty())
            normalPNG = make_shared<imagePNG>(normalMapFile.c_str(), 3);

          if (!metallicRoughnessMapFile.empty())
            metallicRoughnessPNG = make_shared<imagePNG>(metallicRoughnessMapFile.c_str(), 3);

          newMesh->matPtr = make_shared<pbrMetallicRoughness>(
                              albedoPNG,
                              normalPNG,
                              metallicRoughnessPNG,
                              baseColor, metallicness, roughness);
        }
      }
      
      // read in triangle data
      if (gltfPrim->type == cgltf_primitive_type_triangles) {
        cgltf_accessor*     a = gltfPrim->indices;
        cgltf_buffer_view*  bufferView = a->buffer_view;
        cgltf_buffer*       buffer = bufferView->buffer;

        uint8_t*  byte = (uint8_t*)buffer->data;
        uint16_t* index = (uint16_t*)&(byte[bufferView->offset]);
        for (int idx = 0; idx < a->count; idx += 3) {
          model->meshes[primIndex]->triangles.push_back(
            triangle::create(index[idx], index[idx + 1],
                              index[idx + 2], model->meshes[primIndex]));
        }
      }
    }
  }

  cgltf_free(data);
  return true;
}

#endif