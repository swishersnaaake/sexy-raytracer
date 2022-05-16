#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "globals.h"

#include "Eigen/Dense"

class camera {
  public:
    camera(vec3f  eye,
            vec3f lookAt,
            vec3f up,
            float vFOV,
            float aspect,
            float aperture,
            float focusDist,
            float time0,
            float time1) {
      auto        theta = deg2rad(vFOV);
      auto        h = tan(theta / 2.0f);
      auto        vpHeight = 2.0f * h;
      auto        vpWidth = aspect * vpHeight;

      w = unitVector(eye - lookAt);
      hor = unitVector(up.cross(w));
      vert = unitVector(w.cross(hor));

      //auto        focalLen = 1.0f;

      origin = eye;
      horizontal = focusDist * vpWidth * hor;
      vertical = focusDist * vpHeight * vert;
      lleft = origin - horizontal / 2.0f - vertical / 2.0f - focusDist * w;

      lensRadius = aperture / 2.0f;
      t0 = time0;
      t1 = time1;
    }

    ray getRay(float s, float t) {
      vec3f rd = lensRadius * randomInUnitDisk();
      vec3f offset = rd(0) * hor + rd(1) * vert;

      return ray(origin + offset,
                  lleft + s * horizontal + t * vertical - origin - offset,
                  randomFloat(t0, t1));

      // no focus distance version
      //return ray(origin, lleft + s * horizontal + t * vertical - origin);
  }
    
  private:
    vec3f   origin;
    vec3f   lleft;
    vec3f   horizontal;
    vec3f   vertical;
    vec3f   w;
    vec3f   hor;
    vec3f   vert;
    float   lensRadius;
    float   t0;
    float   t1;
};

#endif