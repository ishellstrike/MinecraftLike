#pragma once
#include "GenStrategy.h"
class WorldGenMountains : public GenStrategy
{
public:

	void WorldPass(World& w);
	// ������������ ����� GenStrategy
  virtual void Generate(Sector & s) override;
};

