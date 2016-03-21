#pragma once
#include "Agent.h"

class Tool : public Agent
{
public:
	Tool();

	void JsonLoad(const rapidjson::Value &val) override;

	// ������������ ����� Agent
	virtual PAgent Clone(GameObject * parent, const std::string & name = "") override;
	virtual void Update(const GameObjectParams & params) override;

private:
	StringIntern tool;
};

REGISTER_AGENT(Tool)