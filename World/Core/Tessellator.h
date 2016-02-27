// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef SectorSplicer_h__
#define SectorSplicer_h__


#include "..\tools\QueuedThread.h"
#include "..\tools\CoordSystem.h"
#include "BlockTessellator.h"
#include "SectorTessellator.h"
#include <unordered_map>
#include "..\tools\StringIntern.h"
#include <tuple>
#include <vector>


class Tessellator : public QueuedThread<Tessellator>
{
public:
  Tessellator(class RenderSector &render);

  /// ���������������� ������.

  /// ���������� ���������� � ��������� �������.
  void Set(const WBPos &pos, PGameObjectTessellator block);

  void Set(const SPos &spos, std::vector<std::tuple<size_t, StringIntern>> &&blocks);

  /// �������� �������, ��� �� ���������.
  void SayChanged(const SPos &pos);

public:
  /// ������ �� ��������������� ��� ������������� ��� ������ �����������.

  void Start();

  void Process();

  void Stop();

  PGameObjectTessellator GetBlock(const WBPos &pos);
  
private:
  std::unordered_map<SPos, std::shared_ptr<SectorTessellator>> mSectors;

  class RenderSector &mRender;

private:
  /// ����� ������ �� ������� �������.
  std::shared_ptr<SectorTessellator> FindSector(const SPos &pos);

};



#endif // SectorSplicer_h__