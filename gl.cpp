#include <iostream>
#include <fstream>
#include <sstream>

#include "stdio.h"
#include "stdlib.h"

#include "gl.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "srtstbimage.h"

using std::string;
using std::stringstream;

float rtFramePos[] = {
  1.0f, -1.0f, 0,
  1.0f, 1.0f, 0,
  -1.0f, 1.0f, 0,
  -1.0f, -1.0f, 0,
};

float rtFrameUVs[] = {
  1.0f, 1.0f,
  1.0f, 0.0f,
  0.0f, 0.0f,
  0.0f, 1.0f,
};

uint16_t  rtFrameIndices[] = {
  0, 1, 2,
  0, 2, 3
};

GLFWwindow* glfwWin;

unsigned int  rtFrameTexID;
unsigned int  posVBO;
unsigned int  texCoordVBO;
unsigned int  indexEBO;
unsigned int  rtFrameVAO;
unsigned int  vertexShader;
unsigned int  fragmentShader;
unsigned int  shaderProgram;

static bool loadShaders(const char* vsFilePath, const char* fsFilePath) {
  std::string   vsCode, fsCode;
  std::ifstream vsFile, fsFile;

  vsFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
  fsFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

  try {
    vsFile.open(vsFilePath);
    fsFile.open(fsFilePath);

    std::stringstream vsStream, fsStream;
    vsStream << vsFile.rdbuf();
    fsStream << fsFile.rdbuf();

    vsFile.close();
    fsFile.close();

    vsCode = vsStream.str();
    fsCode = fsStream.str();
  }
  catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
  }

  const char* vsShader = vsCode.c_str();
  const char* fsShader = fsCode.c_str();
  char        infoLog[512];

  int success;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vsShader, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fsShader, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return true;
}

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

  glGenVertexArrays(1, &rtFrameVAO);
  glGenBuffers(1, &posVBO);
  glGenBuffers(1, &texCoordVBO);
  glGenBuffers(1, &indexEBO);

  glBindVertexArray(rtFrameVAO);
  glBindBuffer(GL_ARRAY_BUFFER, posVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rtFramePos), rtFramePos, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rtFrameUVs), rtFrameUVs, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rtFrameIndices), rtFrameIndices, GL_DYNAMIC_DRAW);

  loadShaders("../shaders/vs.glsl", "../shaders/fs.glsl");

  glGenTextures(1, &rtFrameTexID);
  glBindTexture(GL_TEXTURE_2D, rtFrameTexID);

  return true;
}

bool glFrame(uint8_t* frameData, int texWidth, int texHeight) {
  if (glfwWindowShouldClose(glfwWin))
    return false;

  int fbWidth, fbHeight;

  glfwGetFramebufferSize(glfwWin, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shaderProgram);
  glBindVertexArray(rtFrameVAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rtFrameTexID);
  //glTextureSubImage2D(rtFrameTexID, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, frameData);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, frameData);
  glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);

  glfwSwapBuffers(glfwWin);
  glfwPollEvents();

  return true;
}

void glTerminate() {
  glfwDestroyWindow(glfwWin);
  glfwTerminate();
}