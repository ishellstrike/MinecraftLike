#pragma once
#include "WindowBase.h"

class World;

class WindowInventory : public WindowBase
{
public:

  WindowInventory();
  World *w;

  // ������������ ����� WindowBase
  virtual void Draw() override;

  static WindowInventory &Get()
  {
    static WindowInventory w;
    return w;
  }
};