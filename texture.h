#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <iostream>

#include "globals.h"
#include "rtpstbimage.h"

using std::uint8_t;

class texture {
  public:
    virtual color3f value(float u, float v, const vec3f& p) const = 0;
};

class solidColor : public texture {
  public:
    solidColor() {}
    solidColor(color3f c) : colorValue(c) {}

    solidColor(float r, float g, float b) :
                solidColor(color3f(r, g, b)) {}
    
    virtual color3f value(float u, float v, const vec3f& p) const override {
      return colorValue;
    }

  private:
    color3f colorValue;
};

class checker : public texture {
  public:
    checker() {}
    checker(shared_ptr<texture> _even, shared_ptr<texture> _odd)
            : even(_even), odd(_odd) {}
    
    checker(color3f c1, color3f c2) : even(make_shared<solidColor>(c1)), odd(make_shared<solidColor>(c2)) {}

    virtual color3f value(float u, float v, const vec3f& p) const override {
      auto  sines = sinf(10.0f * p(0)) * sinf(10.0f * p(1)) * sinf(10.0f * p(2));
      if (sines < 0)
        return odd->value(u, v, p) * 255.0f;
      else
        return even->value(u, v, p) * 255.0f;
    }
  
  private:
    shared_ptr<texture> odd, even;
};

class image3bpp : public texture {
  public:
    const static int  bpp = 3;

    image3bpp() : data(nullptr), width(0), height(0), bytesPerScanline(0) {}
    image3bpp(const char* filename) {
      auto  componentsPP = bpp;
      
      data = stbi_load(filename, &width, &height, &componentsPP, componentsPP);

      if (!data) {
        std::cerr << "ERROR: Could not load image file '" << filename << "'\n";
        width = height = 0;
      }

      bytesPerScanline = bpp * width;
    }
    image3bpp(void* data, int len, int* x, int* y, int* comp, int reqComp) {
      auto  componentsPP = bpp;
      
      //(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
      //data = stbi_load_from_memory(data, &width, &height, &componentsPP, componentsPP);
    }

    ~image3bpp() {
      delete data;
    }

    virtual color3f value(float u, float v, const vec3f& p) const override {
      if (data == nullptr)
        return color3f(1.0f, 0, 1.0f);
      
      u = clamp(u, 0, 1.0f);
      v = 1.0f - clamp(v, 0, 1.0f);

      auto  i = static_cast<int>(u * width);
      auto  j = static_cast<int>(v * height);

      if (i >= width)
        i = width - 1;
      
      if (j >= height)
        j = height - 1;
      
      auto  pixel = data + j * bytesPerScanline + i * bpp;

      return color3f(pixel[0], pixel[1], pixel[2]);
    }
  
  protected:
    uint8_t*        data;
    int             width, height;
    int             bytesPerScanline;
};

#endif