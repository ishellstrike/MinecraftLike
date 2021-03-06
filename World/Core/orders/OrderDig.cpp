#include "Core/agents/Chest.h"
#include "OrderDig.h"
#include <boost/format/format_fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Core/agents/PositionAgent.h"
#include "core/agents/Ore.h"
#include <core/World.h>

OrderDig::OrderDig(glm::vec3 v) : pos(v)
{
}

std::string OrderDig::to_string() const
{
	return (boost::format("OrderDig: pos = %1%") % glm::to_string(pos)).str() + Order::to_string();
}

void OrderDig::Perform(const GameObjectParams & params, PGameObject performer, float work)
{
	auto c = performer->GetAgent<Creature>();

	auto remove = true;
	if (auto orb = params.world->GetBlock(pos))
	{
		if (auto ore = orb->GetAgent<Ore>())
		{
			remove = false;
			auto ch = performer->GetAgent<Chest>();
			auto cs = ore->DigSome();
			ch->Push(cs.obj, cs.count);

			if (ore->Expire())
				remove = true;
		}
	}

	if (remove)
	{
		params.world->SetBlock(pos, nullptr);
		Done();
	}
}