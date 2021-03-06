#pragma once
#include <Core/agents/Agent.h>

class Foliage : public Agent
{
public:
	AGENT(Foliage)

	void JsonLoad(const rapidjson::Value &val) override;

	// ������������ ����� Agent
	virtual PAgent Clone(GameObject * parent, const std::string & name = "") override;
	virtual void Update(const GameObjectParams & params) override;
};

REGISTER_AGENT(Foliage)