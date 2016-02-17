// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#include "PositionAgent.h"



PositionAgent::PositionAgent()
  : Agent(nullptr, "PositionAgent", "")
{
}

PositionAgent::PositionAgent(GameObject *parent, const std::string &name)
  : Agent(parent, "PositionAgent", name)
{
}


PositionAgent::PositionAgent(const PositionAgent &object, GameObject *parent, const std::string &name)
  : Agent(parent, "PositionAgent", name)
{

}

PositionAgent::~PositionAgent()
{
}

PAgent PositionAgent::Clone(GameObject *parent, const std::string &name)
{
  return MakeAgent<PositionAgent>(*this, parent, name);
}

void PositionAgent::JsonLoad(const rapidjson::Value & val)
{
  Agent::JsonLoad(val);
}

void PositionAgent::Update(const GameObjectParams &params)
{

}
