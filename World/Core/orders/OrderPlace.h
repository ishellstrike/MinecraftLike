﻿#pragma once
#include "../GameObject.h"
#include "OrderBus.h"

struct OrderPlace : NumberedOrder<OrderPlace>
{
	OrderPlace(WBPos v, StringIntern i);
	std::string to_string() const override;
	WBPos pos;
	StringIntern item;

	float Tiring() const override;

	glm::vec3 GetPos() const override;

	bool IsEquals(const Order& rhs) override;

	void Perform(const GameObjectParams & params, PGameObject performer, float work = 0) override;
};
