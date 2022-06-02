#ifndef __COLOR_H__
#define __COLOR_H__

#include "vec3.h"

#include <iostream>

using std::uint8_t;

void writeColor(std::ostream &out, color3f pixelColor, int numSamples) {
  auto  r = pixelColor(0);
  auto  g = pixelColor(1);
  auto  b = pixelColor(2);

  auto  scale = 1.0f / numSamples;
  r = sqrtf(r * scale);
  g = sqrtf(g * scale);
  b = sqrtf(b * scale);

  out << static_cast<int>(256 * clamp(r, 0.0f, 0.999f)) << ' '
    << static_cast<int>(256 * clamp(g, 0.0f, 0.999f)) << ' '
    << static_cast<int>(256 * clamp(b, 0.0f, 0.999f)) << '\n';
}

void writeColorTarget(uint8_t* data, int x, int y, int w, int h, int bpp, color3f pixelColor, int numSamples) {
  auto  r = pixelColor(0);
  auto  g = pixelColor(1);
  auto  b = pixelColor(2);

  auto  scale = 1.0f / numSamples;
  r = sqrtf(r * scale);
  g = sqrtf(g * scale);
  b = sqrtf(b * scale);

  uint8_t*  pixel = &(data[(y * w + x) * bpp]);

  pixel[0] = static_cast<uint8_t>(256 * clamp(r, 0.0f, 0.999f));
  pixel[1] = static_cast<uint8_t>(256 * clamp(g, 0.0f, 0.999f));
  pixel[2] = static_cast<uint8_t>(256 * clamp(b, 0.0f, 0.999f));
  pixel[3] = 255;
}

#endif