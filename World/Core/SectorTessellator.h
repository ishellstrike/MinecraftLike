



#pragma once
#ifndef SectorTessellator_h__
#define SectorTessellator_h__


#include "Config.h"
#include "BlockTessellator.h"
#include <array>
#include "..\tools\CoordSystem.h"
#include "..\Render\Model.h"
#include <glm/glm.hpp>
#include "SectorBase.h"


class SectorTessellator : public SectorBase<PGameObjectTessellator>
{
public:
	SectorTessellator(const SPos &pos);

	// �������� �������, ��� �� �������.
	void SayChanged();

	// ���� ������ ��� �������, ��������� ��������� � �������� � ������.
	bool Update(class Tessellator *tesselator, class RenderSector &render);

	// ������� ���������� �������� � Resources
	static void Init();

	inline bool IsChanged() const noexcept
	{
		return mChanged;
	}

	inline SPos GetPos() const noexcept
	{
		return mPos;
	}

private:
	SPos mPos;
	bool mChanged = false;
	void __PushMmodel(const Model &model, const SBPos &pos);
};



#endif // SectorTessellator_h__