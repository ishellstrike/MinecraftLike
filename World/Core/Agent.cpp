#include "Agent.h"
#include "GameObject.h"

void Agent::Update(const GameObjectParams & params)
{
}

void Agent::Requirements()
{
}

void Agent::Afterload(GameObject * parent)
{
}

void Agent::DrawGui(float gt)
{
}

void Agent::Interact(const InteractParams & params)
{
}

void Agent::OnDestroy(const GameObjectParams & params)
{
}

void Agent::OnCreate(const GameObjectParams & params)
{
}

void Agent::OnAdjacentChanged(const GameObjectParams & params)
{
}

void Agent::JsonLoad(const rapidjson::Value & val)
{
}

void Agent::save(boost::archive::binary_oarchive& ar, const unsigned) const
{
}

void Agent::load(boost::archive::binary_iarchive& ar, const unsigned)
{
}


AgentFactory::FactoryType &AgentFactory::Get()
{
  static FactoryType object;
  return object;
}
