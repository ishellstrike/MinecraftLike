#pragma once
#include "WindowBase.h"
#include <tools\StringIntern.h>

class WindowDb : public WindowBase
{
public:

  // ������������ ����� WindowBase
  virtual void Draw(glm::vec2 mainwin_size) override;

  StringIntern GetSelectedId() 
  {
	  return selected_id;
  }



  static WindowDb &Get()
  {
    static WindowDb w;
    return w;
  }

private:
	StringIntern selected_id;
};