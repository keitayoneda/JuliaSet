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
int width = 1024, height = 768;
double width_length = 2.0;
double height_length =
    width_length * static_cast<float>(height) / static_cast<float>(width);
double zoom = 0.5;
double x_center = -0.1197280330103155, y_center = 0.0769413973084643;
double x_min = (-width_length / 2) / zoom + x_center,
       x_max = (width_length / 2) / zoom + x_center,
       y_min = (-height_length / 2) / zoom + y_center,
       y_max = (height_length / 2) / zoom + y_center;

GLuint programID;

GLuint vertexbuffer;

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

  return 0;
}

void drawJulia() {
  updateDrawArea();
  glfwGetFramebufferSize(window, &width, &height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, width, height);
  glOrtho(0, width, height, 0, -1, 1);

  /* draw_time += 0.001f; */
  /* c_x = 0.6f * std::cos(2 * draw_time); */
  /* c_y = 0.6f * std::sin(2 * draw_time); */

  glUniform2f(glGetUniformLocation(programID, "c"), c_x, c_y);
  glUniform2f(glGetUniformLocation(programID, "boundMin"), x_min, y_min);
  glUniform2f(glGetUniformLocation(programID, "boundMax"), x_max, y_max);
  glUniform1f(glGetUniformLocation(programID, "window_width"), width);
  glUniform1f(glGetUniformLocation(programID, "window_height"), height);
}

int main() {
  if (init() != 0) {
    return -1;
  }

  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  programID = LoadShaders(SHADER_DIR "gpu_vertex_tmp.glsl",
                          SHADER_DIR "gpu_fragment.glsl");
  static const float plain[] = {-1.0f, 1.0f,  0.0f, 1.0f, 1.0f,  0.0f,
                                -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f};

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, width, height);
  glOrtho(0, width, height, 0, -1, 1);
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(plain), plain, GL_STATIC_DRAW);

  std::chrono::time_point<std::chrono::system_clock> start, end;
  do {
    start = std::chrono::system_clock::now();
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programID);
    /* if (zoom < 1e12) { */
    /*   zoom *= 1.05f; */
    /*   draw_julia = true; */
    /* } */
    drawJulia();

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    end = std::chrono::system_clock::now();
    std::cout << "duration: "
              << std::chrono::duration<double>(end - start).count() * 1000
              << " ms" << std::endl;
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
