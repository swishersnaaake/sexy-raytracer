#ifndef __GL_H__
#define __GL_H__

#include <iostream>
#include <fstream>
#include <sstream>

//#include "stdio.h"
//#include "stdlib.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using std::string;
using std::stringstream;

class gl3D {
  public:
    gl3D() {}

    bool init(int width, int height);
    bool rtFrame(uint8_t* frameData, int frameWidth, int frameHeight);
    void terminate();
  
  private:
    bool loadShaders(const char* vsFilePath, const char* fsFilePath);

    static void errorCallback(int error, const char* description) {
      //fprintf(stderr, "Error: %s\n", description);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }
    }
  
  private:
    GLFWwindow*   glfwWin;
    unsigned int  rtFrameTexID;
    unsigned int  posVBO;
    unsigned int  texCoordVBO;
    unsigned int  indexEBO;
    unsigned int  rtFrameVAO;
    unsigned int  vertexShader;
    unsigned int  fragmentShader;
    unsigned int  shaderProgram;

    const float     rtFramePos[12] = {
      1.0f, -1.0f, 0,
      1.0f, 1.0f, 0,
      -1.0f, 1.0f, 0,
      -1.0f, -1.0f, 0,
    };

    const float     rtFrameUVs[8] = {
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
      0.0f, 1.0f,
    };

    const uint16_t  rtFrameIndices[6] = {
      0, 1, 2,
      0, 2, 3
    };
};

bool gl3D::loadShaders(const char* vsFilePath, const char* fsFilePath) {
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

bool gl3D::init(int width, int height) {
  if (!glfwInit()) {
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWin = glfwCreateWindow(width, height, "Sexy Ray Tracer", NULL, NULL);

  if (!glfwWin) {
    glfwTerminate();
    return false;
  }

  glfwSetErrorCallback(errorCallback);
  glfwSetKeyCallback(glfwWin, keyCallback);

  glfwMakeContextCurrent(glfwWin);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    int blah = 0;
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

bool gl3D::rtFrame(uint8_t* frameData, int texWidth, int texHeight) {
  if (glfwWindowShouldClose(glfwWin))
    return false;

  int fbWidth, fbHeight;

  glfwGetFramebufferSize(glfwWin, &fbWidth, &fbHeight);
  GLint  theData[4];
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
  glad_glViewport(0, 0, fbWidth, fbHeight);
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
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

void gl3D::terminate() {
  glfwDestroyWindow(glfwWin);
  glfwTerminate();
}

#endif