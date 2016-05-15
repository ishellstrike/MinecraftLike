#include "Ore.h"
#include <Serealize.h>
#include <Core\GameObject.h>
#include <imgui.h>
#include <Core\DB.h>
#include <Core\ChestSlot.h>

void Ore::JsonLoad(const rapidjson::Value &val)
{
	JSONLOAD(NVP(contains), NVP(size));
}

PAgent Ore::Clone(GameObject * parent, const std::string &name)
{
	auto t = MakeAgent<Ore>(*this);
	t->mParent = parent;
	return t;
}

void Ore::DrawGui()
{
}

ChestSlot Ore::DigSome()
{
	if (contains.empty())
		return{};

	int t = rand() % contains.size();
	float ch = rand() / static_cast<float>(RAND_MAX);

	--size;
	if (contains[t].chance >= ch)
	{
		return ChestSlot{ DB::Get().Create(contains[t].id), float(contains[t].count) };
	}
}

bool Ore::Expire()
{
	return size <= 0;
}
