#version 430

layout(local_size_x = 1, local_size_y = 1) in;

/*layout(std430, binding = 0) buffer dataForThis {
  float color[3][3];
} dataIn;*/

struct hittableEntry {
  vec4   positions[3];   // for triangle nodes
  vec4   UVs[3];         // for triangle nodes
  ivec4  matIndex;       // for triangle nodes

  vec4   boxMin, boxMax; // for bvh nodes
  ivec4  leftAndRight;   // for bvh nodes
};

layout(std430, binding = 0) buffer hittableBuffer {
  hittableEntry entry[];
} hittables;

layout(rgba32f, binding = 1) uniform image2D imgOutput;
//layout(rgba8ui, binding = 1) uniform uimage2D imgOutput;

void main() {
  uint index = gl_GlobalInvocationID.x % 400;

  vec4 pixel = vec4(hittables.entry[index].UVs[0].r,
                    hittables.entry[index].UVs[0].g,
                    hittables.entry[index].UVs[0].b,
                    hittables.entry[index].UVs[0].a);
  
  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  imageStore(imgOutput, pixelCoords, pixel);
}