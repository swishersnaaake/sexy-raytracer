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

#define USE_OPENGL  1
#define USE_COMPUTE 1

using std::string;
using std::stringstream;

struct ssboBuffer {
  vec3f color0;
  vec3f color1;
  vec3f color2;
};

ssboBuffer  tempBuffer;

class glDevice {
  public:
    glDevice() {}

    bool init(int width, int height);
    bool rtFrame(void* frameData, int frameWidth, int frameHeight);
    void terminate();
  
  private:
    bool buildGraphicsProgram(const char* vsFilePath, const char* fsFilePath);
    bool buildComputeProgram(const char* csFilePath);
    
    GLuint  loadShader(const char* shaderPath, GLuint shaderType);

    void  setDefaultSamplers(void) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

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

    GLuint  rtFrameTex, rtFrameComputeTex;
    GLuint  rtFramePosVBO, rtFrameUVVBO, rtFrameIndexEBO, rtFrameVAO;

    GLuint  rtFrameVS, rtFrameFS, rtFrameProgram;
    GLuint  rtFrameCS, rtFrameComputeProgram;
    GLuint  SSBO;

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

GLuint glDevice::loadShader(const char* shaderPath, GLuint shaderType) {
  GLuint  shader = 0;
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

bool glDevice::buildGraphicsProgram(const char* vsFilePath, const char* fsFilePath) {
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
    glGetProgramInfoLog(rtFrameProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    glDeleteProgram(rtFrameProgram);
    rtFrameProgram = 0;

    return false;
  }

  glDeleteShader(rtFrameVS);
  glDeleteShader(rtFrameFS);

  return true;
}

bool glDevice::buildComputeProgram(const char* csFilePath) {
  int workGroupCount[3];
  for (int i = 0; i < 3; i++)
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &workGroupCount[i]);
  
  int workGroupSize[3];
  for (int i = 0; i < 3; i++)
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, i, &workGroupCount[i]);
  
  int workGroupInv;
  glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workGroupInv);

  rtFrameCS = loadShader(csFilePath, GL_COMPUTE_SHADER);
  if (!rtFrameCS)
    return false;
  
  rtFrameComputeProgram = glCreateProgram();
  glAttachShader(rtFrameComputeProgram, rtFrameCS);
  glLinkProgram(rtFrameComputeProgram);

  int   success;
  char  infoLog[512];
  glGetProgramiv(rtFrameComputeProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(rtFrameComputeProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    glDeleteProgram(rtFrameComputeProgram);
    rtFrameComputeProgram = 0;

    return false;
  }

  glDeleteShader(rtFrameCS);

  return true;
}

bool glDevice::init(int width, int height) {
  // set up glfw
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

  // set up extensions loader
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    int blah = 0;
  glfwSwapInterval(1);
  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

  // set up graphics pipe
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

  // graphics shaders
  buildGraphicsProgram("../shaders/rtFrameVS.glsl", "../shaders/rtFrameFS.glsl");

  // graphics resources
  glGenTextures(1, &rtFrameTex);
  glBindTexture(GL_TEXTURE_2D, rtFrameTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  // compute shader
  buildComputeProgram("../shaders/compute.glsl");

  // compute resources
  glGenTextures(1, &rtFrameComputeTex);
  glBindTexture(GL_TEXTURE_2D, rtFrameComputeTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
  glBindImageTexture(1, rtFrameComputeTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

  tempBuffer.color0 = vec3f(1.0f, 0, 0);
  tempBuffer.color1 = vec3f(0, 1.0f, 0);
  tempBuffer.color2 = vec3f(0, 0, 1.0f);

  glGenBuffers(1, &SSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(tempBuffer), &tempBuffer, GL_DYNAMIC_READ);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  return true;
}

bool glDevice::rtFrame(void* frameData, int texWidth, int texHeight) {
  static uint32_t frame = 0;
  if (glfwWindowShouldClose(glfwWin))
    return false;

  int fbWidth, fbHeight;

  glfwGetFramebufferSize(glfwWin, &fbWidth, &fbHeight);
  GLint  theData[4];
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
  glViewport(0, 0, fbWidth, fbHeight);
  glGetIntegerv(GL_VIEWPORT, &(theData[0]));
  glClear(GL_COLOR_BUFFER_BIT);

#if USE_COMPUTE
  // start compute
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
  tempBuffer.color0 = vec3f((frame + 0) % 3 == 0 ? 1.0f : 0,
                              (frame + 2) % 3 == 0 ? 1.0f : 0,
                              (frame + 1) % 3 == 0 ? 1.0f : 0);
  tempBuffer.color1 = vec3f((frame + 1) % 3 == 0 ? 1.0f : 0,
                              (frame + 0) % 3 == 0 ? 1.0f : 0,
                              (frame + 2) % 3 == 0 ? 1.0f : 0);
  tempBuffer.color2 = vec3f((frame + 2) % 3 == 0 ? 1.0f : 0,
                              (frame + 1) % 3 == 0 ? 1.0f : 0,
                              (frame + 0) % 3 == 0 ? 1.0f : 0);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(tempBuffer), &tempBuffer);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  glUseProgram(rtFrameComputeProgram);
  glDispatchCompute(texWidth, texHeight, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
#endif

  // start graphics
  glUseProgram(rtFrameProgram);
  glBindVertexArray(rtFrameVAO);

  glActiveTexture(GL_TEXTURE0);
#if USE_COMPUTE
  glBindTexture(GL_TEXTURE_2D, rtFrameComputeTex);
#else
  glBindTexture(GL_TEXTURE_2D, rtFrameTex);
  glTextureSubImage2D(rtFrameTex, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, frameData);
#endif
  glUniform1i(glGetUniformLocation(rtFrameProgram, "tex"), 0);
  setDefaultSamplers();

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);

  glfwSwapBuffers(glfwWin);
  glfwPollEvents();

  frame++;

  return true;
}

void glDevice::terminate() {
  glfwDestroyWindow(glfwWin);
  glfwTerminate();
}

#endif