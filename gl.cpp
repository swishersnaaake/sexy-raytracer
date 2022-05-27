#include "stdio.h"
#include "stdlib.h"

#include "gl.h"

GLFWwindow* glfwWin;

void errorCallback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

bool glInit(int width, int height) {
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWin = glfwCreateWindow(width, height, "Sexy Ray Tracer", NULL, NULL);

  if (!glfwWin) {
    glfwTerminate();
    exit(EXIT_FAILURE);
    return false;
  }

  glfwSetErrorCallback(errorCallback);
  glfwSetKeyCallback(glfwWin, keyCallback);

  glfwMakeContextCurrent(glfwWin);
  gladLoadGL();
  glfwSwapInterval(1);
  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

  return true;
}

bool glFrame() {
  if (glfwWindowShouldClose(glfwWin))
    return false;

  int width, height;

  glfwGetFramebufferSize(glfwWin, &width, &height);
  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  glfwSwapBuffers(glfwWin);
  glfwPollEvents();

  return true;
}

void glTerminate() {
  glfwDestroyWindow(glfwWin);
  glfwTerminate();
}