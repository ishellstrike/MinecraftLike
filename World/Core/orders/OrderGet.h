﻿#pragma once
#include "../GameObject.h"
#include "OrderBus.h"

struct OrderGet : NumberedOrder<OrderGet>
{
	OrderGet(glm::vec3 v, PGameObject i);
	std::string to_string() const override;
	glm::vec3 pos;
	PGameObject item;

	float Tiring() const override
	{
		return 0.05f;
	}

	glm::vec3 GetPos() const override
	{
		return pos;
	}

	bool IsEquals(const Order &rhs) override
	{
		if (rhs.GetId() != GetId())
			return false;

		const auto &o = static_cast<const OrderGet &>(rhs);

		return o.pos == pos && item == item;
	}

	void Perform(const GameObjectParams & params, PGameObject performer, float work = 0) override;
};
