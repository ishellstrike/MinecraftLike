﻿#ifndef WINDOW_GL_H
#define WINDOW_GL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

class Window
{
public:
  Window(const glm::uvec2 &size);
  ~Window();

  // Инициализация оконной системы.
  static void WindowSystemInitialize();

  // Завершить работу с оконной системой.
  static void WindowSystemFinally();

  // Установить текущий контекст для данного окна.
  void SetCurrentContext();

  // Должно ли окно закрыться?
  bool WindowShouldClose();

  void Update();

  static const glm::uvec2 GetSize();
  static const glm::uvec2 GetFbSize();

  // Установить имя окна.
  void SetTitle(const std::string &title);

  GLFWwindow *Get();

  void SetResizeCallback(const std::function<void(int, int)> &f);

private:

  struct WindowDeleter
  {
    void operator()(GLFWwindow *window) const
    {
      printf("window delete\n");
      glfwDestroyWindow(window);
    }
  };

  static void ResizeCallback(GLFWwindow*, int, int);
  static std::function<void(int, int)> mResf;

  static std::unique_ptr<GLFWwindow, WindowDeleter> mWindow;

  static glm::uvec2 mSize;
  static glm::ivec2 fbsize;
};

#endif // WINDOW_GL_H
