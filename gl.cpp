#include "gl.h"

bool glInit(void) {
  return glfwInit();
}

void glTerminate() {
  glfwTerminate();
}