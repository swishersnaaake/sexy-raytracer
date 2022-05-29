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

class glDevice {
  public:
    glDevice() {}

    bool init(int width, int height);
    bool rtFrame(uint8_t* frameData, int frameWidth, int frameHeight);
    void terminate();
  
  private:
    bool loadGraphicsProgram(const char* vsFilePath, const char* fsFilePath);
    
    unsigned int  loadShader(const char* shaderPath, unsigned int shaderType);
    unsigned int  buildGraphicsProgram(unsigned int vs, unsigned int fs);

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
    unsigned int  rtFramePosVBO, rtFrameUVVBO, rtFrameIndexEBO, rtFrameVAO;

    unsigned int  rtFrameVS, rtFrameFS, rtFrameProgram;

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

unsigned int glDevice::loadShader(const char* shaderPath, unsigned int shaderType) {
  unsigned int  shader = 0;
  std::string   shaderCode;
  std::ifstream shaderFile;

  shaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

  try {
    shaderFile.open(shaderPath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();
    shaderCode = shaderStream.str();
  }
  catch (std::ifstream::failure e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    return shader;
  }

  int   success;
  char  infoLog[512];

  const char* shaderString = shaderCode.c_str();
  shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &shaderString, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    glDeleteShader(shader);
    shader = 0;

    return shader;
  }

  return shader;
}

bool glDevice::loadGraphicsProgram(const char* vsFilePath, const char* fsFilePath) {
  rtFrameVS = loadShader(vsFilePath, GL_VERTEX_SHADER);
  if (!rtFrameVS)
    return false;
  
  rtFrameFS = loadShader(fsFilePath, GL_FRAGMENT_SHADER);
  if (!rtFrameFS)
    return false;
  
  rtFrameProgram = glCreateProgram();
  glAttachShader(rtFrameProgram, rtFrameVS);
  glAttachShader(rtFrameProgram, rtFrameFS);
  glLinkProgram(rtFrameProgram);

  int   success;
  char  infoLog[512];
  glGetProgramiv(rtFrameProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(rtFrameProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    glDeleteProgram(rtFrameProgram);
    rtFrameProgram = 0;

    return false;
  }

  glDeleteShader(rtFrameVS);
  glDeleteShader(rtFrameFS);

  return true;
}

bool glDevice::init(int width, int height) {
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
  glGenBuffers(1, &rtFramePosVBO);
  glGenBuffers(1, &rtFrameUVVBO);
  glGenBuffers(1, &rtFrameIndexEBO);

  glBindVertexArray(rtFrameVAO);
  glBindBuffer(GL_ARRAY_BUFFER, rtFramePosVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rtFramePos), rtFramePos, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, rtFrameUVVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(rtFrameUVs), rtFrameUVs, GL_DYNAMIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rtFrameIndexEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rtFrameIndices), rtFrameIndices, GL_DYNAMIC_DRAW);

  loadGraphicsProgram("../shaders/rtFrameVS.glsl", "../shaders/rtFrameFS.glsl");

  glGenTextures(1, &rtFrameTexID);
  glBindTexture(GL_TEXTURE_2D, rtFrameTexID);

  return true;
}

bool glDevice::rtFrame(uint8_t* frameData, int texWidth, int texHeight) {
  if (glfwWindowShouldClose(glfwWin))
    return false;

  int fbWidth, fbHeight;

  glfwGetFramebufferSize(glfwWin, &fbWidth, &fbHeight);
  GLint  theData[4];
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
  glViewport(0, 0, fbWidth, fbHeight);
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(rtFrameProgram);
  glBindVertexArray(rtFrameVAO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, rtFrameTexID);
  //glTextureSubImage2D(rtFrameTexID, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, frameData);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, frameData);
  glUniform1i(glGetUniformLocation(rtFrameProgram, "tex"), 0);
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

void glDevice::terminate() {
  glfwDestroyWindow(glfwWin);
  glfwTerminate();
}

#endif