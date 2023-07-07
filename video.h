#pragma once

#include "defines.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct VideoState {
  GLFWwindow *window;
  GLuint textures[128];
  // 128 texture glyphs, 8x8 font?
} VideoState;

extern VideoState *video_state;

void video_init();
void video_update(u8 *is_running);
void video_clean();
