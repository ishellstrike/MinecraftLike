﻿#pragma once
#ifndef Mouse_h__
#define Mouse_h__

#include <glm/glm.hpp>
#include <mutex>


class Mouse
{
public:
  Mouse(struct GLFWwindow &window);
  ~Mouse(void);

  float DeltaX();

  float DeltaY();

  glm::vec2 GetMoved();

  const glm::vec2 &GetPos();

  void Update();

  void SetCentring(bool b);
  bool GetCentring();

private:
  struct GLFWwindow &mWindow;

  glm::vec2 mPos;
  glm::vec2 mMoved;

  std::mutex mMutex;

  bool mIsFocused;
  bool mCentring = true;
};



#endif // Mouse_h__
