



#pragma once
#ifndef World_h__
#define World_h__

#include <unordered_map>
#include <glm/glm.hpp>
#include <memory>
#include <list>
#include "Sector.h"

#include "../Render/Render.h"
#include "EventBus.h"


class Tessellator;

class World
{
public:
  World();
  ~World();

  void Update(float dt);

  struct RecipeOrder
  {
	  RecipeOrder(PRecipe r, int e, bool i) : recipe(r), elapsed(e), infinite(i) {}

	  PRecipe recipe;
	  int elapsed = 1;
	  bool infinite = false;
  };

  // �������� ������. ���� ������ �� ��������, ����� ����������� ������� 
  // ��������� ������ �� ���������� ��������.
  std::shared_ptr<Sector> GetSector(const SPos &position);

  PGameObject GetBlock(const WBPos &position);

  PGameObject GetBlock(const WBPos & wbpos, std::shared_ptr<Sector> &contains);

  bool IsWalkable(const WBPos & wbpos);

  // ���������� ���� � �������� �������.
  // ������������ ���� ����� ������.
  PGameObject SetBlock(const WBPos &pos, PGameObject block);

  PGameObject Spawn(const WBPos &position, PGameObject creature);

  PGameObject Place(const SBPos & position, PGameObject item);
  PGameObject Replace(const SBPos & position, PGameObject item);


  std::list<std::pair<glm::vec3, PGameObject>> &GetStorages();
  std::list<RecipeOrder> &GetRecipeOrders();
  void QueueRecipeOrder(const RecipeOrder &ro);

  int GetActiveCount();

  void SetTessellator(Tessellator *tess);
  Tessellator * GetTessellator();

private:
  std::unordered_map<SPos, std::shared_ptr<Sector>> mSectors;
  std::list<std::pair<glm::vec3, PGameObject>> storages;

  std::list<RecipeOrder> recipe_orders;

  Sector *mCurrentSector;

private:
  // ����� ������ �� ������� �������.
  std::shared_ptr<Sector> FindSector(const SPos &position);

  Tessellator *mTesselator;
};

#endif // World_h__