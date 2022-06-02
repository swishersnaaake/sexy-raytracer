#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

out vec3 vColor;
out vec2 vTexCoord;

void main() {
  gl_Position = vec4(pos, 1.0);
  vColor = vec3(1.0, 1.0, 1.0);
  vTexCoord = uv;
}
