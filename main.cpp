#include <iostream>
#include <ostream>
#include <memory>

#include "Eigen/Core"
#include "Eigen/Geometry"

#include "globals.h"
#include "color.h"
#include "hittablelist.h"
#include "sphere.h"
#include "model.h"
#include "camera.h"
#include "material.h"
#include "bvh.h"
#include "srtstbimage.h"
#include "gl.h"

using namespace Eigen;

using std::uint8_t;
using std::uint16_t;

using mat33f = Matrix<float, 3, 3/*, Eigen::RowMajor*/>;
using mat34f = Matrix<float, 3, 4/*, Eigen::RowMajor*/>;
using mat44f = Matrix<float, 4, 4/*, Eigen::RowMajor*/>;
using quatf = Quaternion<float>;

color3f rayColor(const ray &r, const color3f& background, const hittable &world, int maxBounce) {
  hitRecord record;

  if (maxBounce <= 0)
    return color3f(0, 0, 0);

  if (!world.hit(r, 0.001f, infinity, record))
    return background;

  ray     scattered;
  color3f attenuation;
  color3f emitted = record.matPtr->emitted(record.uv(0), record.uv(1), record.p);

  if (!record.matPtr->scatter(r, record, attenuation, scattered))
    return emitted;

  color3f newColor = rayColor(scattered, background, world, maxBounce - 1);
  newColor = color3f(newColor(0) * attenuation(0), newColor(1) * attenuation(1), newColor(2) * attenuation(2));
  return emitted + newColor;
}

hittableList randomScene() {
  hittableList  objects;
  hittableList  scene;
  shared_ptr<model> testModel;

  AffineCompact3f final = AffineCompact3f::Identity();
  if (0) {
    //testModel = model::create("../data/cube.gltf");
    testModel = model::create("../data/square.gltf");
    testModel->init();

    //AngleAxisf      rotate(deg2rad(180.0f), vec3f::UnitX());
    AngleAxisf      rotate(deg2rad(-15.0f), vec3f::UnitY());
    //AngleAxisf      rotate(deg2rad(45.0f), unitVector(vec3f::UnitX() +
                                                      //vec3f::UnitY()));
    AffineCompact3f trans(Translation3f(vec3f(0.0f, 1.0f, 0.0f)));
    final = trans * rotate;
  }
  else if (1) {
    //testModel = model::create("../data/masterchief-sep.gltf");
    testModel = model::create("../data/masterchief2-separate.gltf");
    testModel->init();

    AngleAxisf      rotate(deg2rad(270.0f), unitVector(vec3f::UnitY()));
    AffineCompact3f trans(Translation3f(vec3f(0.0f, 0.0f, 0.0f)));
    AffineCompact3f scale = AffineCompact3f::Identity();
    scale *= Scaling(0.075f);
    final = trans * rotate * scale;
  }
  else {
    testModel = model::create("../data/scene.gltf");
    testModel->init();

    /*AngleAxisf      rotate(deg2rad(270.0f), unitVector(vec3f::UnitY()));
    AffineCompact3f trans(Translation3f(vec3f(0.0f, 0.0f, 0.0f)));
    AffineCompact3f scale = AffineCompact3f::Identity();
    scale *= Scaling(0.075f);
    final = trans * rotate * scale;*/
  }

  for (const auto& mesh : testModel->meshes) {
    for (auto& pos : mesh->positions) {
      pos = final * pos;
    }
  }

  for (const auto& mesh : testModel->meshes) {
    for (const auto& tri : mesh->triangles) {
      objects.add(tri);
    }
  }

  //auto ground_material = make_shared<pbrMetallicRoughness>(color3f(0.5, 0.5, 0.5));
  auto checkerTex = make_shared<checker>(color3f(0.2f, 0.3f, 0.1f), color3f(0.9f, 0.9f, 0.9f));
  objects.add(make_shared<sphere>(vec3f(0,-1000,0.0f), vec3f(0,-1000,0.0f), 0, 1.0f, 1000, make_shared<pbrMetallicRoughness>(checkerTex)));

/*    for (int a = -11; a < 11; a++) {
      for (int b = -11; b < 11; b++) {
          auto choose_mat = randomFloat();
          vec3f center(a + 0.9*randomFloat(), 0.2f, b + 0.9f*randomFloat());

          if (length(center - vec3f(4, 0.2, 0)) > 0.9) {
              shared_ptr<material> sphere_material;

              if (choose_mat < 0.8) {
                  // diffuse
                  //auto albedo = randomVec3f() * randomVec3f();
                  color3f albedo(randomFloat() * randomFloat(),
                                  randomFloat() * randomFloat(),
                                  randomFloat() * randomFloat());
                  sphere_material = make_shared<pbrMetallicRoughness>(albedo);
                  vec3f  center2 = center + vec3f(0, randomFloat(0, 0.5f), 0);
                  spheres.add(make_shared<sphere>(center, center2, 0, 1.0f, 0.2f, sphere_material));
              } else if (choose_mat < 0.95f) {
                  // metal
                  auto albedo = randomVec3f(0.5f, 1);
                  auto fuzz = randomFloat(0, 0.5f);
                  sphere_material = make_shared<metal>(albedo, fuzz);
                  spheres.add(make_shared<sphere>(center, center, 0, 1.0f, 0.2f, sphere_material));
              } else {
                  // glass
                  sphere_material = make_shared<dielectric>(1.5f);
                  spheres.add(make_shared<sphere>(center, center, 0, 1.0f, 0.2f, sphere_material));
              }
          }
      }
  }*/
#if 1
  //auto material1 = make_shared<dielectric>(1.5);
  //spheres.add(make_shared<sphere>(vec3f(0, 1, 0), vec3f(0, 1, 0), 0, 1.0f, 1.0f, material1));
  auto lightMat = make_shared<diffuseLight>(color3f(250.2f, 220.9f, 110.2f));
  objects.add(make_shared<sphere>(vec3f(-7.0f, 4.0f, 6.0f), vec3f(-7.0f, 4.0f, 6.0f), 0, 1.0f, 1.0f, lightMat));

  //auto material2 = make_shared<pbrMetallicRoughness>(color3f(0.4, 0.2, 0.1));
  //spheres.add(make_shared<sphere>(vec3f(-4, 1, 0), vec3f(-4, 1, 0), 0, 1.0f, 1.0f, material2));
  //spheres.add(make_shared<sphere>(vec3f(0, 1, 2.25f), vec3f(0, 1, 2.25f), 0, 1.0f, 1.0f, material2));

  auto ironAlbedo = make_shared<imagePNG>("../data/rustediron2_basecolor-2x1.png", 3);
  auto ironNMap = make_shared<imagePNG>("../data/rustediron2_normal-2x1.png", 3);
  auto ironMMap = make_shared<imagePNG>("../data/rustediron2_metallic-2x1.png", 1);
  auto ironRMap = make_shared<imagePNG>("../data/rustediron2_roughness-2x1.png", 1);
  auto ironMat = make_shared<pbrMetallicRoughness>(ironAlbedo, ironNMap,
                                                    ironMMap, ironRMap,
                                                    vec4f(1.0f, 1.0f, 1.0f, 1.0f));
  objects.add(make_shared<sphere>(vec3f(-3.0f, 1.0f, 0.0f), vec3f(-3.0f, 1.0f, 0.0f), 0, 1.0f, 1.0f,
                                  ironMat));

  auto material3 = make_shared<metal>(color3f(0.7, 0.6, 0.5), 0.0);
  objects.add(make_shared<sphere>(vec3f(3.0f, 1.0f, 0), vec3f(3.0f, 1.0f, 0), 0, 1.0f, 1.0f, material3));
#endif
  scene.add(make_shared<bvhNode>(objects, 0, 1));
  return scene;

  //return objects;
}

int main(int, char**) {
  // camera
  //const auto  aspect = 16.0f / 9.0f;
  const auto  aspect = 2.0f;
  //vec3f       eye(12.0f, 2.0f, 3.0f);
  vec3f       eye(0.0f, 3.0f, 5.0f);
  vec3f       lookAt(0, 2.5f, 0);
  vec3f       vUp(0, 1.0f, 0);
  auto        distToFocus = 10.0f; //(eye - lookAt).length();
  auto        aperture = 0.1f; //2.0f;
  // sky blue day
  color3f     background(0.53f, 0.81f, 0.92f);

  camera      mainCamera(eye, lookAt, vUp, 70.0f, aspect, aperture, distToFocus, 0, 1.0f);

  // image
  //const int   imageHeight = 720;
  const int   imageHeight = 240;
  const int   imageWidth = static_cast<int>(imageHeight * aspect);
  //const int   numSamples = 500;
  //const int   maxBounce = 50;
  //const int   numSamples = 2000;
  const int   numSamples = 4;
  const int   maxBounce = 4;
  const vec3f samplePos(0, 0.8f, 0);
  uint8_t*    target = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * 3 * imageWidth * imageHeight));

  // world
  hittableList world = randomScene();
  glInit(imageWidth, imageHeight);

  //std::cout << "P3\n" << imageWidth << ' ' << imageHeight << "\n255\n";

  while (glFrame(target, imageWidth, imageHeight)) {
    for (int y = imageHeight -1; y >= 0; --y) {
      std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
      for (int x = 0; x < imageWidth; ++x) {
        color3f pixelColor(0, 0, 0);
        for (int s = 0; s < numSamples; ++s) {
          AngleAxisf rotate(deg2rad(15.0f) + deg2rad(90.0f * s), vec3f::UnitZ());
          mat33f     m = rotate.matrix();
          vec3f      newSamplePos = m * samplePos;

          // random samples
          auto  u = float(x + randomFloat()) / (imageWidth - 1);
          auto  v = float(y + randomFloat()) / (imageHeight - 1);

          // 4x rotated grid
          //auto  u = float((w + newSamplePos(0)) / (imageWidth - 1));
          //auto  v = float((h + newSamplePos(1)) / (imageHeight - 1));
          ray   r = mainCamera.getRay(u, v);
          pixelColor += rayColor(r, background, world, maxBounce);
        }

        //writeColor(std::cout, pixelColor, numSamples);
        writeColorTarget(target, x, imageHeight - y, imageWidth, imageHeight, 3, pixelColor, numSamples);
      }
    }
  }

  glTerminate();

  stbi_write_png("test.png", imageWidth, imageHeight, 3, target, 3 * imageWidth);
  free(target);

  std::cerr << "\nDone.\n";
}
