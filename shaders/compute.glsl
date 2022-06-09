#version 430

float EPSILON = 0.00001;

layout(local_size_x = 1, local_size_y = 1) in;

/*layout(std430, binding = 0) buffer dataForThis {
  float color[3][3];
} dataIn;*/

struct ray {
  vec3  o;
  vec3  dir;
  float t;
};

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

struct hitRecord {
  vec4  p;          // 3 used
  vec4  normal;     // 3 used
  vec4  tangent;    // 3 used
  vec4  bitangent;  // 3 used
  vec4  uv;         // 2 used
  vec4  t;          // 1 used

  bvec4 frontFace;  // 1 used
  ivec4 matIndex;   // 1 used
};

void recordSetFaceNormal(in ray r, in vec3 outwardNormal, inout hitRecord record) {
  record.frontFace.x = dot(r.dir, outwardNormal) < 0;
  record.normal = record.frontFace.x ? vec4(outwardNormal, 0) : vec4(-outwardNormal, 0);
}

bool aabbHit(in ray r, in vec4 aabbMin, in vec4 aabbMax, inout float tMin, inout float tMax) {
  vec3 nDir = normalize(r.dir);
  for (int axis = 0; axis < 3; axis++) {
    float  t0 = min((aabbMin[axis] - r.o[axis]) / r.dir[axis],
                      (aabbMax[axis] - r.o[axis]) / r.dir[axis]);
    float  t1 = min((aabbMin[axis] - r.o[axis]) / r.dir[axis],
                      (aabbMax[axis] - r.o[axis]) / r.dir[axis]);
    
    tMin = max(t0, tMin);
    tMax = max(t1, tMax);

    if (tMax <= tMin)
      return false;
  }

  return true;
}

vec3 triangleGetNormal(in vec4 positions[3]) {
  vec3 a = vec3(positions[1].xyz) - vec3(positions[0].xyz);
  vec3 b = vec3(positions[2].xyz) - vec3(positions[0].xyz);
  
  return cross(a, b);
}

bool triangleHit(in ray ray, in vec4 positions[3], in vec4 UVs[3], in float tMin, in float tMax,
                  out hitRecord record) {
  vec3  v0 = vec3(positions[0].xyz);
  vec3  v1 = vec3(positions[1].xyz);
  vec3  v2 = vec3(positions[2].xyz);
  vec2  uv0 = vec2(UVs[0].st);
  vec2  uv1 = vec2(UVs[1].st);
  vec2  uv2 = vec2(UVs[2].st);

  vec3  normal = triangleGetNormal(positions);
  float area = length(normal);

  float NdotDir = dot(normal, ray.dir);

  // check if parallel
  if (abs(NdotDir) < EPSILON)
    return false;
  
  if (dot(ray.dir, normal) > 0)
    return false;
  
  float d = -dot(normal, v0);
  float t = -(dot(normal, ray.o) + d) / NdotDir;

  if (t < tMin)
    return false;

  vec3  p = ray.o + t * ray.dir;
  vec3  outer;
  float u, v;

  // edge 0
  vec3 edge0 = v1 - v0;
  vec3 vp0 = p - v0;
  outer = cross(edge0, vp0);
  if (dot(normal, outer) < 0)
    return false;
  
  // edge 1
  vec3 edge1 = v2 - v1;
  vec3 vp1 = p - v1;
  outer = cross(edge1, vp1);
  if (dot(normal, outer) < 0)
    return false;
  
  // edge 2
  vec3 edge2 = v0 - v2;
  vec3 vp2 = p - v2;
  outer = cross(edge2, vp2);
  if (dot(normal, outer) < 0)
    return false;

  // passed hit test
  // generate barycentric coordinates
  float d0, d1, d2, r0, r1, r2;
  d0 = distance(p, v0);
  d1 = distance(p, v1);
  d2 = distance(p, v2);

  float denom = (1.0f / d0) + (1.0f / d1) + (1.0f / d2);
  r0 = (1.0f / d0) / denom;
  r1 = (1.0f / d1) / denom;
  r2 = (1.0f / d2) / denom;

  u = r0 * uv0.x + r1 * uv1.x + r2 * uv2.x;
  v = 1.0f - (r0 * uv0.y + r1 * uv1.y + r2 * uv2.y);

  // blah outwardNormal redundant here
  vec3 outwardNormal = normalize(normal);
  record.t.x = t;
  //record.p = ray.at(record.t);
  recordSetFaceNormal(ray, outwardNormal, record);
  record.uv.st = vec2(u, v);
  //record.matPtr = parentMesh->matPtr;
  //calcTangentBasis(outwardNormal, record.tangent, record.bitangent);

  return true;
}

void main() {
  uint  stack[32];
  uint  stackIndex = 0;
  uint  nodeIndex = 0;
  ray   ray;
  vec4  pixel = vec4(0, 0, 1.0, 1.0);
  hitRecord record;
  float tMin = 0.0001;
  float tMax = 9999.0;

  stack[stackIndex++] = 0;

  // traverse aabb nodes
  while (stackIndex != 0) {
    nodeIndex = stack[stackIndex - 1];
    stackIndex--;

    if (hittables.entry[nodeIndex].leftAndRight.r != -1) {
      // if ray intersects box
      if (aabbHit(ray, hittables.entry[nodeIndex].boxMin,
                  hittables.entry[nodeIndex].boxMax, tMin, tMax)) {
        stack[stackIndex++] = hittables.entry[nodeIndex].leftAndRight.g;
        stack[stackIndex++] = hittables.entry[nodeIndex].leftAndRight.r;
      }
    }
    else {
      // if ray intersects triangle
      if (triangleHit(ray, hittables.entry[nodeIndex].positions,
                      hittables.entry[nodeIndex].UVs, tMin, tMax, record))
        pixel = vec4(1.0f, 0, 0, 1.0f);
    }
  }

  ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

  imageStore(imgOutput, pixelCoords, pixel);
}