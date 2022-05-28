#ifndef __GL_H__
#define __GL_H__

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern bool glInit(int width, int height);
bool glFrame(uint8_t* frameData, int texWidth, int texHeight);
extern void glTerminate();

#endif