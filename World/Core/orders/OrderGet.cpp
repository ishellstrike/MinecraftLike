﻿#include "OrderGet.h"
#include <Core/agents/PositionAgent.h>
#include <glm/gtx/string_cast.hpp>
#include <core\world.h>
#include <Core/agents/Chest.h>

OrderGet::OrderGet(glm::vec3 v, PGameObject i) : pos(v), item(i)
{
}

std::string OrderGet::to_string() const
{
	return (boost::format("OrderGet: pos = %1% id = %2%") % glm::to_string(pos) % item->GetId()).str() + Order::to_string();
}

void OrderGet::Perform(const GameObjectParams & params, PGameObject performer, float work)
{
	auto c = performer->GetAgent<Creature>();

	params.world->Replace(pos, item);

	auto p = performer->GetAgent<Chest>();
	p->Push(item);

	Done();
}