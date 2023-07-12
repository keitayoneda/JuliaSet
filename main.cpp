#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <cmath>
#include <iostream>

#include "shader.hpp"

GLFWwindow *window;
int width, height;
constexpr int n_col = 512, n_raw = 256;
float width_length = 2.0f;
float x_min = -1.0f, x_max = 1.0f, y_min = -1.0f, y_max = 1.0f;
std::array<std::array<float, 3 * 2 * n_raw>, n_col - 1> vertices;
std::array<std::array<float, 3 * 2 * n_raw>, n_col - 1> colors;

float c_x = 0.3f, c_y = -0.5f;

void recurence(float x, float y, float &next_x, float &next_y) {
  next_x = x * x - y * y + c_x;
  next_y = 2 * x * y + c_y;
}

float calcGradient(float x0, float y0) {
  const int max_iteration = 100;
  float x = x0, y = y0;
  for (int i = 0; i < max_iteration; ++i) {
    if (x * x + y * y > 4.0f) {
      return static_cast<float>(i) / static_cast<float>(max_iteration);
    }
    recurence(x, y, x, y);
  }
  return 1.0f;
}

void calcColors(float x_min, float x_max, float y_min, float y_max) {
  for (int i = 0; i < n_col; ++i) {
    for (int j = 0; j < n_raw; ++j) {
      [[maybe_unused]] float x =
          static_cast<float>(i) / static_cast<float>(n_col) * (x_max - x_min) +
          x_min;
      [[maybe_unused]] float y = static_cast<float>(n_raw - j) /
                                     static_cast<float>(n_raw) *
                                     (y_max - y_min) +
                                 y_min;
      /* float gradient = */
      /*     static_cast<float>(i * n_raw + j) / static_cast<float>(n_col *
       * n_raw); */

      float gradient = calcGradient(x, y);
      if (i < n_col - 1) {
        colors[i][6 * j] = 1 - 6 * std::cos(gradient);
        colors[i][6 * j + 1] = std::sin(gradient);
        colors[i][6 * j + 2] = gradient;
      }
      if (i > 0) {
        colors[i - 1][6 * j + 3] = 1 - 6 * std::cos(gradient);
        colors[i - 1][6 * j + 4] = std::sin(gradient);
        colors[i - 1][6 * j + 5] = gradient;
      }
    }
  }
}

void initVertices() {
  for (int i = 0; i < n_col; ++i) {
    for (int j = 0; j < n_raw; ++j) {
      if (i < n_col - 1) {
        vertices[i][6 * j] =
            2 * static_cast<float>(i) / static_cast<float>(n_col - 1) - 1;
        vertices[i][6 * j + 1] =
            2 * static_cast<float>(j) / static_cast<float>(n_raw - 1) - 1;
        vertices[i][6 * j + 2] = 0.0f;
      }
      if (i > 0) {
        vertices[i - 1][6 * j + 3] =
            2 * static_cast<float>(i) / static_cast<float>(n_col - 1) - 1;
        vertices[i - 1][6 * j + 4] =
            2 * static_cast<float>(j) / static_cast<float>(n_raw - 1) - 1;
        vertices[i - 1][6 * j + 5] = 0.0f;
      }
    }
  }
}

int init() {
  if (!glfwInit()) {
    std::cerr << "failed to initialize glfw" << std::endl;
    return -1;
  }
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(1024, 768, "fractal", nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "failed to open window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "failed to initialize glew" << std::endl;
    return -1;
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  initVertices();

  return 0;
}

float draw_time = 0.0f;
int main() {
  if (init() != 0) {
    return -1;
  }

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  GLuint programID =
      LoadShaders(SHADER_DIR "vertex.glsl", SHADER_DIR "fragment.glsl");

  std::array<GLuint, n_col - 1> vertexbuffers;
  for (int i = 0; i < n_col - 1; ++i) {
    glGenBuffers(1, &vertexbuffers[i]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(float),
                 &vertices[i], GL_STATIC_DRAW);
  }

  std::array<GLuint, n_col - 1> colorbuffers;
  for (int i = 0; i < n_col - 1; ++i) {
    glGenBuffers(1, &colorbuffers[i]);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, colors[i].size() * sizeof(float), &colors[i],
                 GL_DYNAMIC_DRAW);
  }

  do {
    glfwGetFramebufferSize(window, &width, &height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    glOrtho(0, width, height, 0, -1, 1);

    draw_time += 0.01f;
    c_x = 0.6f * std::cos(2 * draw_time);
    c_y = 0.6f * std::sin(2 * draw_time);
    calcColors(x_min, x_max, y_min, y_max);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(programID);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    for (int i = 0; i < n_col - 1; ++i) {
      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glBindBuffer(GL_ARRAY_BUFFER, colorbuffers[i]);
      glBufferData(GL_ARRAY_BUFFER, colors[i].size() * sizeof(float),
                   &colors[i], GL_DYNAMIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, n_raw * 2);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glfwSwapBuffers(window);
    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
