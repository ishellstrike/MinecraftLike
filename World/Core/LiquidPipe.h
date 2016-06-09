#pragma once
#include "Agent.h"
#include "GameObject.h"
#include <vector>
#include "ChestSlot.h"

class LiquidPipe : public Agent
{
public:
	AGENT(LiquidPipe)

	void JsonLoad(const rapidjson::Value &val) override;

	// ������������ ����� Agent
	virtual PAgent Clone(GameObject * parent, const std::string & name = "") override;
	virtual void Update(const GameObjectParams & params) override;
	virtual void DrawGui(float gt) override;

	void OnDestroy(const GameObjectParams & params) override;

	void OnCreate(const GameObjectParams & params) override;

	inline bool IsLocked() 
	{
		return mLocked;
	}

	inline float GetLiquidCount()
	{
		return liq.count;
	}

	inline float SetLiquidCount(float c)
	{
		return liq.count = c;
	}

	StringIntern GetLiquidID();

	bool SetLiquid(ChestSlot cs);

	bool CanAccept(PGameObject i);

	bool PushLiquid(ChestSlot cs);
	

private:
	bool mLocked = false;
	bool mDisabled = false;
	ChestSlot liq;
	float max_volume;
};

REGISTER_AGENT(LiquidPipe)