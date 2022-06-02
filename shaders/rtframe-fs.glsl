#version 430 core

out vec4  fColor;

in vec3   vColor;
in vec2   vTexCoord;

uniform sampler2D tex;

void main() {
  fColor = texture(tex, vTexCoord);// * vec4(vColor, 1.0);
  //fColor = vec4(vTexCoord.x, 0, vTexCoord.y, 1.0);
}
