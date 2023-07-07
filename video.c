#include "video.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// data files, do not include .data.c in header files, that will
// lead to a linker error.
#include "assets/my_font.data.c"

VideoState *video_state;

VideoState *make_video_state() {
  VideoState *vs = (VideoState *)malloc(sizeof(VideoState));
  vs->window = NULL;
  return vs;
}

void clean_video_state(VideoState *vs) {
  free(vs->window);
  free(vs);
}

void init_textures() {
  glGenTextures(128, video_state->textures);
  for (int i = 0; i < 128; i++) {
    glBindTexture(GL_TEXTURE_2D, video_state->textures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 8, 8, 0, GL_RED, GL_UNSIGNED_BYTE,
                 bitmap_font[i]);
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
}

void draw_text(const char *text, int x, int y) {
  int len = strlen(text);

  // Render each character of the string
  for (int i = 0; i < len; i++) {
    int c = text[i];
    glBindTexture(GL_TEXTURE_2D, video_state->textures[c]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(x + i * 8, y);
    glTexCoord2f(1, 0);
    glVertex2f(x + (i + 1) * 8, y);
    glTexCoord2f(1, 1);
    glVertex2f(x + (i + 1) * 8, y + 8);
    glTexCoord2f(0, 1);
    glVertex2f(x + i * 8, y + 8);
    glEnd();
  }
}

void video_init() {
  video_state = make_video_state();

  init_textures();

  // Initialize the library
  if (!glfwInit()) {
    // TODO: handle error
  }

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
  }

  // Create a windowed mode window and its OpenGL context
  video_state->window =
      glfwCreateWindow(WINDOW_W, WINDOW_H, WINDOW_TITLE, NULL, NULL);
  if (!video_state->window) {
    glfwTerminate();
  }

  // Make the video_state->window's context current
  glfwMakeContextCurrent(video_state->window);

  // { // setup basic shaders.
  //   const char *vertexShaderSource = "#version 330 core\n"
  //                                    "layout (location = 0) in vec2
  //                                    position;\n" "void\n" "main() {\n"
  //                                    "gl_Position = vec4(position,
  //                                    0.0, 1.0);\n"
  //                                    "}\n"
  //                                    ";\n";
  //
  //   const char *fragmentShaderSource =
  //       "#version 330 core\n"
  //       "out vec4 color;\n"
  //       "void main() {\n"
  //       "color = vec4(1.0, 0.0, 0.0, 1.0); // Red color\n"
  //       "};\n";
  //
  //   GLuint vertexShader, fragmentShader, shaderProgram;
  //
  //   vertexShader = glCreateShader(GL_VERTEX_SHADER);
  //   glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  //   glCompileShader(vertexShader);
  //
  //   fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  //   glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  //   glCompileShader(fragmentShader);
  //
  //   shaderProgram = glCreateProgram();
  //   glAttachShader(shaderProgram, vertexShader);
  //   glAttachShader(shaderProgram, fragmentShader);
  //   glLinkProgram(shaderProgram);
  //
  //   glDeleteShader(vertexShader);
  //   glDeleteShader(fragmentShader);
  // }
}

void video_update(u8 *is_running) {
  glfwSwapBuffers(video_state->window);
  glfwPollEvents();

  if (glfwGetKey(video_state->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(video_state->window, 1);

  // Swap front and back buffers
  glfwSwapBuffers(video_state->window);

  // actual drawing
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    {
      glBegin(GL_TRIANGLES);
      glColor3f(1.0f, 0.0f, 0.0f);
      glVertex2f(0.0f, 0.5f);
      glColor3f(0.0f, 1.0f, 0.0f);
      glVertex2f(-0.5f, -0.5f);
      glColor3f(0.0f, 0.0f, 1.0f);
      glVertex2f(0.5f, -0.5f);
      glEnd();
    }

    {
      // Draw a single pixel
      glBegin(GL_POINTS);
      glColor3f(1.0f, 0.0f, 0.0f); // Set color to red
      glVertex2f(0.8f, 0.5f);      // Set pixel position
      glEnd();
    }
  }

  printf("%d\n", glfwWindowShouldClose(video_state->window));
  *is_running = !glfwWindowShouldClose(video_state->window);
}

void video_clean() {
  glfwTerminate();
  clean_video_state(video_state);
}
