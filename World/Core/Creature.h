



#pragma once
#ifndef Creature_h__
#define Creature_h__

#include "GameObject.h"
#include "PositionAgent.h"

// ��������. ����� �������.
class Creature : public GameObject
{
public:
  Creature(const StringIntern &__id);
  ~Creature();

  PGameObject Clone() override;

  void Update(GameObjectParams &params) override;


private:
  static const StringIntern mPositionAgentName;

};



#endif // Creature_h__