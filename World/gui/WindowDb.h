#pragma once
#include "WindowBase.h"

class WindowDb : public WindowBase
{
public:
  WindowDb();
  ~WindowDb();

  // ������������ ����� WindowBase
  virtual void Draw() override;

  ImTextureID tid;
};