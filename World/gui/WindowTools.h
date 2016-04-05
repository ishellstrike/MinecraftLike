#pragma once
#include "WindowBase.h"
#include <tools\StringIntern.h>

class World;

enum class SelectedOrder
{
	NONE,
	DIG_SQUARE
};

class WindowTools : public WindowBase
{
public:

	WindowTools();

	// ������������ ����� WindowBase
	virtual void Draw(glm::vec2 mainwin_size) override;

	static WindowTools &Get()
	{
		static WindowTools w;
		return w;
	}

	SelectedOrder selected;
};