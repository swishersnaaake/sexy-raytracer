#version 430

layout(local_size_x = 1, local_size_y = 1) in;

layout(std430, binding = 0) buffer dataForThis {
  float color[3][3];
} dataIn;

layout(rgba32f, binding = 1) uniform image2D imgOutput;

void main() {
  uint index = gl_GlobalInvocationID.x % 3;

  //vec4 pixel = vec4(1.0, 0, 1.0, 1.0);
  vec4 pixel = vec4(dataIn.color[index][0], dataIn.color[index][1], dataIn.color[index][2], 1.0);

  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  imageStore(imgOutput, pixelCoords, pixel);
}