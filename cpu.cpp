#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <thread>

#include "shader.hpp"

GLFWwindow *window;
int width, height;
constexpr int n_col = 768, n_raw = 1024;
double width_length = 2.0;
double height_length =
    width_length * static_cast<float>(n_col) / static_cast<float>(n_raw);
double zoom = 0.5;
double x_center = -0.1197280330103155, y_center = 0.0769413973084643;
double x_min = (-width_length / 2 + x_center) / zoom,
       x_max = (width_length / 2 + x_center) / zoom,
       y_min = (-height_length / 2 + y_center) / zoom,
       y_max = (height_length / 2 + y_center) / zoom;

std::array<std::array<float, 3 * 2 * n_raw>, n_col - 1> vertices;
std::array<std::array<float, 3 * 2 * n_raw>, n_col - 1> colors;
std::array<GLuint, n_col - 1> vertexbuffers;

constexpr int n_circle_division = 20;
std::array<float, 3 * n_circle_division> circle_vertices;
std::array<GLuint, n_col - 1> colorbuffers;
GLuint programID;

double c_x = 0.197, c_y = -0.558;
bool draw_julia = false;

int count = 0;

void updateDrawArea() {
  x_min = (-width_length / 2) / zoom + x_center;
  x_max = (width_length / 2) / zoom + x_center;
  y_min = (-height_length / 2) / zoom + y_center;
  y_max = (height_length / 2) / zoom + y_center;
}

void mouse_cursor_callback(GLFWwindow *window, double x, double y) {
  static double prev_x = x, prev_y = y;
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE &&
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
    count++;

    return;
  }
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
    std::cout << "x: " << x << ", y: " << y << std::endl;
    c_x = static_cast<double>(x) / static_cast<double>(width) * width_length -
          width_length / 2.0;
    c_y = static_cast<double>(height - y) / static_cast<double>(height) *
              height_length -
          height_length / 2.0;
    std::cout << "c_x: " << c_x << ", c_y: " << c_y << std::endl;
    draw_julia = true;
    return;
  }
  if (count > 10) {
    prev_x = x;
    prev_y = y;
    count = 0;
  }
  x_center -=
      1 * (x - prev_x) / static_cast<double>(width) * width_length / zoom;
  y_center -=
      1 * (y - prev_y) / static_cast<double>(height) * height_length / zoom;

  prev_x = x;
  prev_y = y;
  draw_julia = true;
}

void scroll_callback([[maybe_unused]] GLFWwindow *window,
                     [[maybe_unused]] double x, double y) {
  if (y > 0) {
    zoom *= 1.1;
  } else if (y < 0) {
    zoom /= 1.1;
  }
  std::cout << std::setprecision(16) << "zoom: " << zoom
            << ", x_center: " << x_center << ", y_center: " << y_center
            << std::endl;
  draw_julia = true;
}

void recurence(double x, double y, double &next_x, double &next_y) {
  next_x = x * x - y * y + c_x;
  next_y = 2 * x * y + c_y;
}

double calcGradient(double x0, double y0) {
  const int max_iteration = 500;
  double x = x0, y = y0;
  for (int i = 0; i < max_iteration; ++i) {
    if (x * x + y * y > 4.0) {
      return static_cast<float>(i) / static_cast<float>(max_iteration);
    }
    recurence(x, y, x, y);
  }
  return 1.0f;
}

void calcOnePixelColor(float gradient, float &r, float &g, float &b) {
  r = -gradient * (gradient - 1.0) * 2;
  g = gradient * (2.0 - gradient);
  b = std::sqrt(gradient);
}

void calcColors(double x_min, double x_max, double y_min, double y_max) {
  std::chrono::system_clock::time_point start, end;
  start = std::chrono::system_clock::now();
#pragma omp parallel for
  for (int i = 0; i < n_col; ++i) {
    for (int j = 0; j < n_raw; ++j) {
      [[maybe_unused]] double x = static_cast<double>(i) /
                                      static_cast<double>(n_col) *
                                      (x_max - x_min) +
                                  x_min;
      [[maybe_unused]] double y = static_cast<double>(n_raw - j) /
                                      static_cast<double>(n_raw) *
                                      (y_max - y_min) +
                                  y_min;
      /* double gradient = */
      /*     static_cast<double>(i * n_raw + j) / static_cast<double>(n_col *
       * n_raw); */

      float gradient = calcGradient(x, y);
      float r, g, b;
      calcOnePixelColor(gradient, r, g, b);
      if (i < n_col - 1) {
        colors[i][6 * j] = r;
        colors[i][6 * j + 1] = g;
        colors[i][6 * j + 2] = b;
      }
      if (i > 0) {
        colors[i - 1][6 * j + 3] = r;
        colors[i - 1][6 * j + 4] = g;
        colors[i - 1][6 * j + 5] = b;
      }
    }
  }
  end = std::chrono::system_clock::now();
  /* std::cout << "calcColors: " */
  /*           << std::chrono::duration_cast<std::chrono::milliseconds>(end - */
  /*                                                                    start)
   */
  /*                  .count() */
  /*           << "ms" << std::endl; */
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
  glfwSetCursorPosCallback(window, mouse_cursor_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  initVertices();

  return 0;
}

void drawJulia() {
  updateDrawArea();
  calcColors(x_min, x_max, y_min, y_max);
  glfwGetFramebufferSize(window, &width, &height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, width, height);
  glOrtho(0, width, height, 0, -1, 1);

  /* draw_time += 0.001f; */
  /* c_x = 0.6f * std::cos(2 * draw_time); */
  /* c_y = 0.6f * std::sin(2 * draw_time); */

  glUseProgram(programID);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  for (int i = 0; i < n_col - 1; ++i) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, colorbuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, colors[i].size() * sizeof(float), &colors[i],
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, n_raw * 2);
  }

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  std::cout << "zoom: " << zoom << std::endl;
}

int main() {
  if (init() != 0) {
    return -1;
  }

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  programID = LoadShaders(SHADER_DIR "vertex.glsl", SHADER_DIR "fragment.glsl");

  for (int i = 0; i < n_col - 1; ++i) {
    glGenBuffers(1, &vertexbuffers[i]);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, vertices[i].size() * sizeof(float),
                 &vertices[i], GL_STATIC_DRAW);
  }

  for (int i = 0; i < n_col - 1; ++i) {
    glGenBuffers(1, &colorbuffers[i]);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, colors[i].size() * sizeof(float), &colors[i],
                 GL_DYNAMIC_DRAW);
  }

  drawJulia();
  std::chrono::time_point<std::chrono::system_clock> start, end;
  do {
    start = std::chrono::system_clock::now();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);
    /* if (zoom < 1e12) { */
    /*   zoom *= 1.05f; */
    /*   draw_julia = true; */
    /* } */
    if (draw_julia) {
      drawJulia();
      draw_julia = false;
    } else {

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      for (int i = 0; i < n_col - 1; ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffers[i]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, colorbuffers[i]);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, n_raw * 2);
      }
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glfwSwapBuffers(window);
    glfwPollEvents();
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    if (elapsed_seconds.count() < 0.1) {
      std::this_thread::sleep_for(std::chrono::milliseconds(
          static_cast<int>(1000 * (0.1 - elapsed_seconds.count()))));
    }
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
