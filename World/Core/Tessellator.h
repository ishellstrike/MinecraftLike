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


class Tessellator : public QueuedThread<Tessellator>
{
public:
  Tessellator(class Render &render);

  /// ���������������� ������.

  /// ���������� ���������� � ��������� �������.
  void Set(const WBPos &pos, PBlockTessellator block);

  /// �������� �������, ��� �� ���������.
  void SayChanged(const SPos &pos);

public:
  /// ������ �� ��������������� ��� ������������� ��� ������ �����������.

  void Start();

  void Process();

  void Stop();

  PBlockTessellator GetBlock(const WBPos &pos);
  
private:
  std::unordered_map<SPos, std::shared_ptr<SectorTessellator>> mSectors;

  class Render &mRender;

private:
  /// ����� ������ �� ������� �������.
  std::shared_ptr<SectorTessellator> FindSector(const SPos &pos);

};



#endif // SectorSplicer_h__