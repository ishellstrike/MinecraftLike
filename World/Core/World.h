// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef World_h__
#define World_h__

#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>
#include <list>
#include "Sector.h"
#include "Player.h"
#include "../Render/Render.h"

class Tessellator;

class World
{
public:
  World();
  ~World();

  void Update(float dt);

  /// �������� ������. ���� ������ �� ��������, ����� ����������� ������� 
  /// ��������� ������ �� ���������� ��������.
  std::shared_ptr<Sector> GetSector(const SPos &position);

  PGameObject GetBlock(const WBPos &position);

  /// ���������� ���� � �������� �������.
  /// ������������ ���� ����� ������.
  void SetBlock(const WBPos &pos, PGameObject block);

  Player *GetPlayer();

  void SetTessellator(Tessellator *tess);

private:
  std::unordered_map<SPos, std::shared_ptr<Sector>> mSectors;

  Sector *mCurrentSector;

  std::unique_ptr<Player> mPlayer;

private:
  /// ����� ������ �� ������� �������.
  std::shared_ptr<Sector> FindSector(const SPos &position);

  Tessellator *mTesselator;
};



#endif // World_h__