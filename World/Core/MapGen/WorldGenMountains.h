#pragma once
#include "GenStrategy.h"
class WorldGenMountains : public GenStrategy
{
public:

  // ������������ ����� GenStrategy
  virtual void Generate(Sector & s) override;
};

