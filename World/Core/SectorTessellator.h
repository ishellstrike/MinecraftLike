// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef SectorTessellator_h__
#define SectorTessellator_h__


#include "Config.h"
#include "BlockTessellator.h"
#include <array>
#include "..\tools\CoordSystem.h"
#include "..\Render\Model.h"
#include <glm/glm.hpp>


class SectorTessellator
{
public:
  SectorTessellator(const SPos &pos);
  ~SectorTessellator();

  /// ���������� ���������� � ����.
  void SetBlock(const SBPos &pos, PBlockTessellator tesselator);

  /// �������� ���������� �����.
  PBlockTessellator GetBlock(const SBPos &pos);

  /// �������� �������, ��� �� �������.
  void SayChanged();

  /// ���� ������ ��� �������, ��������� ��������� � �������� � ������.
  void Update(class Tessellator *tesselator, class Render &render);

private:
  std::array<PBlockTessellator, SECTOR_SIZE * SECTOR_SIZE * SECTOR_SIZE> mBlocks;

  SPos mPos;

  Model mModel;
  glm::mat4 mModelMatrix;

  bool mChanged = false;

private:

  void Push(const Model &model, const WPos &pos);

};



#endif // SectorTessellator_h__