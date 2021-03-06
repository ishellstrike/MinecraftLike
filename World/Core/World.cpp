#include "World.h"
#include <tuple>
#include <utility>
#include <GLFW\glfw3.h>
#include "..\tools\CoordSystem.h"
#include "MapGen\WorldWorker.h"
#include "..\Render\Render.h"
#include "Tessellator.h"
#include <Core\agents\Chest.h>
#include <Core\EventBus.h>
#include <memory>
#include <Core\agents\PositionAgent.h>
#include <Game.h>
#include <Core/agents/Farmland.h>
#include "orders/OrderPlace.h"

std::string Room::TypeName(RoomType rt)
{
	switch (rt)
	{
	case RoomType::UNSETTED: return "Unsetted";
	case RoomType::PERSONAL_ROOM: return "Personal Room";
	case RoomType::FARMLAND: return "Farmland";
	}
}

void Room::BinLoad(std::istream& val)
{
	BINLOAD(cells, name, min, max);
	int tt;
	BINLOAD(tt);
	type = static_cast<RoomType>(tt);
}
void Room::BinSave(std::ostream& val) const
{
	BINSAVE(cells, name, min, max);
	BINSAVE(static_cast<int>(type));
}

void Room::Update(float dt)
{
	static float last = 0;

	switch(type)
	{
	case RoomType::FARMLAND:
		if(last >= 1)
		{
			last -= 1;

			for(const auto & p : cells)
			{
				auto b = Game::GetWorker()->w->GetBlock(p + WBPos(0, 0, -1));
				if(b && !b->HasAgent<Farmland>())
				{
					OrderBus::Get().IssueOrder(std::make_shared<OrderPlace>(p + WBPos(0, 0, -1), StringIntern("dirt_cultivated")));
				}
			}
		}
		break;
	}

	last += dt;
}

World::World()
{
}


World::~World()
{
}

void World::Update(float dt)
{
	//mEventBus.Update(this);

	for (auto &i : mSectors)
	{
		i.second->Update(this, dt);
	}

	auto a = DB::Get().Create("allagents");
	for (auto &c : a->mAgents)
	{
		c.second->__AfterUpdate();
	}

	static float ro_delay = 0;
	ro_delay += dt;
	if (ro_delay >= 30)
	{
		ro_delay = 0;
		recipe_orders.insert(recipe_orders.begin(), delayed_recipe_orders.begin(), delayed_recipe_orders.end());
		delayed_recipe_orders.clear();
	}

	for(const auto & r : rooms)
	{
		r->Update(dt);
	}

	auto old_c = std::chrono::system_clock::to_time_t(game_time);
	struct tm old_tm;
	localtime_s(&old_tm, &old_c);

	game_time += std::chrono::milliseconds(static_cast<int>(60000000 * dt));

	auto new_c = std::chrono::system_clock::to_time_t(game_time);
	struct tm new_tm;
	localtime_s(&new_tm, &new_c);

	if (new_tm.tm_mon - old_tm.tm_mon > 0)
		EventBus::Get().Publish<EventNewMonth>(game_time);

	if (new_tm.tm_yday - old_tm.tm_yday > 0)
		EventBus::Get().Publish<EventNewDay>(game_time);

	if (new_tm.tm_year - old_tm.tm_year > 0)
		EventBus::Get().Publish<EventNewYear>(game_time);
}



std::shared_ptr<Sector> World::GetSector(const SPos &position)
{
	auto it = mSectors.find(position);
	if (it == mSectors.end())
	{
		if (auto psec = Game::GetWorker()->GetSector(position))
		{
			mSectors[position] = psec;
			psec->Draw(mTesselator);
			EventBus::Get().Publish<EventSectorReady>(psec);

			// �������� ����� ������, ������� �������� ��������, ��� � �������������.
			SPos offset[] =
			{
			  {  1,  0,  0 },
			  { -1,  0,  0 },
			  {  0,  1,  0 },
			  {  0, -1,  0 },
			  {  0,  0,  1 },
			  {  0,  0, -1 },
			};
			for (const auto &i : offset)
			{
				if (auto sector = FindSector(position + i))
				{
					sector->SayChanged();
				}
			}
			return psec;
		}

		return nullptr;
	}

	return it->second;
}

std::shared_ptr<Sector> World::FindSector(const SPos &position)
{
	static SPos last{ 99999 };
	static std::shared_ptr<Sector> plast;

	if (last == position)
		return plast;

	auto it = mSectors.find(position);
	if (it != mSectors.end())
	{
		plast = it->second;
		return it->second;
	}

	return nullptr;
}

bool World::IsWalkable(const WBPos &wbpos)
{
	auto spos = cs::WBtoS(wbpos);
	if (auto sector = FindSector(spos))
	{
		if (auto b = sector->GetBlock(cs::WBtoSB(wbpos, spos)))
			return false;
	}

	spos = cs::WBtoS(wbpos - glm::ivec3(0, 0, 1));
	if (auto sector = FindSector(spos))
	{
		if (auto b = sector->GetBlock(cs::WBtoSB(wbpos - glm::ivec3(0, 0, 1), spos)))
			return true;
	}

	return false;
}

PGameObject World::GetBlock(const WBPos &wbpos)
{
	auto spos = cs::WBtoS(wbpos);
	if (auto sector = FindSector(spos))
	{
		return sector->GetBlock(cs::WBtoSB(wbpos, spos));
	}

	return nullptr;
}

PGameObject World::GetBlock(const WBPos &wbpos, std::shared_ptr<Sector> &contains)
{
	auto spos = cs::WBtoS(wbpos);
	if (auto sector = FindSector(spos))
	{
		contains = sector;
		return sector->GetBlock(cs::WBtoSB(wbpos, spos));
	}

	contains = nullptr;
	return nullptr;
}

bool World::IsAir(const WBPos &wbpos)
{
	PSector sec;
	PGameObject go = GetBlock(wbpos, sec);
	return !go && sec;
}


static float sdt = 0;
static World *sworld = nullptr;
static glm::ivec3 neib[] = {
	{  1,  0,  0 },
	{  0,  1,  0 },
	{  0,  0,  1 },
	{ -1,  0,  0 },
	{  0, -1,  0 },
	{  0,  0, -1 }
};

PGameObject World::SetBlock(const WBPos &wbpos, PGameObject block, bool no_replace)
{
	auto spos = cs::WBtoS(wbpos);
	std::shared_ptr<Sector> sec;
	static StringIntern storage("stockpile");

	if (auto l = GetBlock(wbpos, sec))
	{
		if (no_replace)
			return nullptr;

		l->OnDestroy({ this, sec.get(), wbpos, 0 });

		if (l->GetId() == storage)
		{
			storages.remove(std::make_pair(wbpos, block));
		}
	}

	if (auto sector = FindSector(spos))
	{
		sector->SetBlock(cs::WBtoSB(wbpos, spos), block);
		if (block) // may be nullptr on place air
		{
			block->OnCreate({ this, sec.get(), wbpos, 0 });

			if (block->GetId() == storage)
			{
				storages.push_back(std::make_pair(wbpos, block));
			}
		}
	}

	for (const auto &n : neib)
	{
		if (auto l = GetBlock(wbpos + n, sec))
			l->OnAdjacentChanged({ this, sec.get(), wbpos + n, 0 });
	}

	return block;
}

PGameObject World::Spawn(const WBPos & position, PGameObject creature)
{
	if (auto s = GetSector(cs::WBtoS(position)))
	{
		s->Spawn(position, creature);
		creature->OnCreate({ this, s.get(), position, 0 });
	}
	else
		LOG(error) << "Spawning " << creature->GetId() << " in not existing sector!";

	return creature;
}

PGameObject World::Place(const SBPos & position, PGameObject item)
{
	if (auto s = GetSector(cs::WBtoS(position)))
	{
		s->Place(position, item);
		item->OnCreate({ this, s.get(), position, 0 });
	}
	else
		LOG(error) << "Placing " << item->GetId() << " in not existing sector!";

	return item;
}

PGameObject World::Replace(const SBPos & position, PGameObject item)
{
	if (auto s = GetSector(cs::WBtoS(position)))
	{
		s->Repace(position, item);
	}
	else
		LOG(error) << "Replacing " << item->GetId() << " in not existing sector!";

	return item;
}

std::list<std::pair<glm::vec3, PGameObject>>& World::GetStorages()
{
	return storages;
}

std::list<World::RecipeOrder>& World::GetRecipeOrders()
{
	return recipe_orders;
}

std::list<World::RecipeOrder>& World::GetDelayedRecipeOrders()
{
	return delayed_recipe_orders;
}

void World::QueueRecipeOrder(const RecipeOrder & ro)
{
	for (auto &r : recipe_orders)
	{
		if (*r.recipe == *ro.recipe)
		{
			r.elapsed += ro.elapsed;
			if (!r.infinite)
				r.infinite = ro.infinite;
			return;
		}
	}

	recipe_orders.push_back(ro);
}

void World::DelayRecipeOrder(const PRecipe & ro)
{
	for (auto &r : recipe_orders)
	{
		if (*r.recipe == *ro)
		{
			delayed_recipe_orders.push_back(r);
			recipe_orders.remove(r);
		}
	}
}

void World::DoneRecipeOrder(const PRecipe & ro, int count)
{
	for (auto &r : recipe_orders)
	{
		if (*r.recipe == *ro)
		{
			r.elapsed -= count;
			if (r.elapsed <= 0 && !r.infinite)
				recipe_orders.remove(r);
		}
	}
}

void World::RegisterCreature(PGameObject go)
{
	creatures.insert({ go->GetAgent<Creature>()->uid, go });
}

std::list<PGameObject> World::GetCreaturesAt(const glm::ivec3 & cell)
{
	std::list<PGameObject> list;

	auto spos = cs::WBtoS(cell);
	if (auto sector = FindSector(spos))
	{
		auto creatures = sector->GetCreatures();
		for (auto &c : creatures)
		{
			auto pos = c->GetAgent<PositionAgent>();
			if (glm::ivec3(pos->Get()) == cell)
			{
				list.push_back(c);
			}
		}
	}

	return list;
}

int World::GetActiveCount()
{
	return mSectors.size();
}

void World::SetTessellator(Tessellator *tess)
{
	mTesselator = tess;
}

Tessellator * World::GetTessellator()
{
	return mTesselator;
}

void World::SetSlise(int s)
{

	slise = s;
	for (auto &se : mSectors)
	{
		se.second->SetSlise(s);
	}

}

void World::BinLoadRooms(std::istream & val)
{
	size_t r_count;
	BINLOAD(r_count);
	for(size_t i = 0; i < r_count; i++)
	{
		PRoom r = std::make_shared<Room>();
		r->BinLoad(val);
		rooms.push_back(r);
	}
}

void World::BinSaveRooms(std::ostream& val) const
{
	BINSAVE(rooms.size());
	for(const auto & r : rooms)
	{
		r->BinSave(val);
	}
}

IngameDate World::GetTime()
{
	return game_time;
}

void World::SetTime(IngameDate g)
{
	game_time = g;
}

void World::SetTime(std::string s)
{
	std::tm tm = {};
	std::stringstream ss(s);
	ss >> std::get_time(&tm, "%b %d %Y %H:%M:%S");
	auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	SetTime(tp);
}
