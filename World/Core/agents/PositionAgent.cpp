#include "PositionAgent.h"
#include <Core/orders/OrderBus.h>
#include <Core\GameObject.h>
#include <core\agents/PhysicAgent.h>
#include <core\EventBus.h>
#include <core\World.h>
#include <imgui.h>
#include <glm\gtx\string_cast.hpp>
#include <fstream>
#include <Core\agents\Chest.h>
#include <queue>
#include <unordered_set>
#include <Render\ParticleSystem.h>
#include <Core/orders/OrderGet.h>
#include <Core/orders/OrderWander.h>
#include <Core/orders/OrderSleep.h>
#include <Core/orders/OrderTalk.h>
#include <Core/orders/OrderCraft.h>
#include <Core/orders/OrderEat.h>
#include <Core/orders/OrderDrop.h>
#include <gui/WindowDb.h>
#include "LiquidPipe.h"
#include <gui/WindowStorages.h>
#include <Core/agents/ChestBlock.h>
#include <Core/orders/OrderCombined.h>
#include <gui/WindowRecipe.h>
#include <Render/TextureManager.h>
#include <Core/orders/OrderFindAndPick.h>

PAgent PositionAgent::Clone(GameObject *parent, const std::string &name)
{
	auto t = MakeAgent<PositionAgent>(*this);
	t->mParent = parent;
	return t;
}

void PositionAgent::JsonLoad(const rapidjson::Value & val)
{
	Agent::JsonLoad(val);
}

void PositionAgent::Update(const GameObjectParams &params)
{

}

bool PositionAgent::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("Position: %s", glm::to_string(mPos).c_str());
		return true;
	}

	return false;
}

PAgent Controlable::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Controlable>(*this);
	t->mParent = parent;
	return t;
}

void Controlable::Update(const GameObjectParams & params)
{
	auto c = mParent->GetAgent<Creature>();

	if (!c->order || c->order->GetId() == Order::Idfor<OrderWander>())
	{
		const auto &all_orders_queue = OrderBus::Get().orders;
		glm::vec3 nearest{ 99999 };
		POrder nearest_order;
		auto p = mParent->GetAgent<PositionAgent>();

		for (const auto &i : all_orders_queue)
		{
			if (!i->IsTaken() && glm::distance(p->Get(), i->GetPos()) < glm::distance(p->Get(), nearest))
			{
				if (auto tire = mParent->GetAgent<ActivityConsumer>())
				{
					if (tire->IsTired() && i->Tiring() > 0) // tired creatures cannot take tiring orders
						continue;

					if (auto prof = mParent->GetAgent<ProfessionPerformer>())
					{
						if (!prof->CanPeformOrder(i))
							continue;
					}
				}

				nearest = i->GetPos();
				nearest_order = i;
			}
		}

		if (auto ch = mParent->GetAgent<Chest>()) //empty pockets order
		{
			auto &i = ch->GetFirst();
			if (i.obj)
			{
				auto& storages = Storages::Get().List();
				if (!storages.empty())
				{
					auto finded = std::find_if(storages.begin(), storages.end(), [&](const std::tuple<glm::vec3, PGameObject> & stor) -> bool
					{
						return std::get<1>(stor)->GetAgent<Chest>()->CanPush(i.obj, i.count);
					});

					if (finded != storages.end())
					{
						auto tpos = std::get<0>(*finded);
						if (!storages.empty() && (glm::distance(p->Get(), tpos) < glm::distance(p->Get(), nearest)))
						{
							nearest_order = std::make_shared<OrderDrop>(std::get<0>(*finded), i.obj, i.count);
							nearest = tpos;
						}
					}
				}
			}
		}

		if (!c->personal.empty() && !nearest_order)
		{
			if (!c->personal.begin()->get())
			{
				c->personal.pop_front();
			}
			else
				if (glm::distance(p->Get(), (*c->personal.begin())->GetPos()) < glm::distance(p->Get(), nearest))
				{
					nearest = (*c->personal.begin())->GetPos();
					nearest_order = c->personal.front();
					c->personal.pop_front();
				}
		}

		{
			auto &ro = params.world->GetRecipeOrders();
			auto &storages = params.world->GetStorages();
			if (!storages.empty() && !ro.empty() && ro.begin()->elapsed > 0)
			{
				auto tpos = storages.begin()->first;
				if (!storages.empty() && (glm::distance(p->Get(), tpos) < glm::distance(p->Get(), nearest)))
				{
					auto i = *ro.begin();

					nearest_order = std::make_shared<OrderCraft>(storages.begin()->first, i.recipe, i.elapsed);
					nearest = tpos;
				}
			}
		}

		if (nearest_order)
		{
			nearest_order->Take();
			c->order = nearest_order;
			if (nearest_order->GetId() == Order::Idfor<OrderCraft>())
			{
				auto &ro = params.world->GetRecipeOrders();
				params.world->DoneRecipeOrder(ro.begin()->recipe);
			}
		}
	}
}


PAgent Creature::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Creature>(*this);
	t->mParent = parent;
	return t;
}

void Creature::DiscardCurrentOrder()
{
}

void Creature::look_around(const GameObjectParams & params)
{
	if (auto m = mParent->GetAgent<Anatomic>())
	{
		for (int i = -3; i <= 3; i++)
			for (int j = -3; j <= 3; j++)
			{
				auto cpos = params.pos + glm::vec3{ i, j, 0 };
				if (auto b = params.world->GetBlock(cpos))
				{
					if (b->HasAgent<Chest>())
					{
						m->Interest("storage", cpos);
					}
				}
			}
	}
}

void Creature::Update(const GameObjectParams & params)
{
	auto p = mParent->GetAgent<PositionAgent>();

	if (order && order->Tiring() > 0)
		if (auto tire = mParent->GetAgent<ActivityConsumer>())
		{
			tire->Tire(order->Tiring());
			if (tire->IsTired())
			{
				OrderBus::Get().IssueDelayedOrder(order);
				order->SetState(Order::State::Initial);
				order = nullptr;
			}
		}

	if (order)
	{
		order->Update(params, mParent->shared_from_this());

		if (order->GetState() == Order::State::Done || order->IsCanceled())
		{
			order = nullptr;
		}
		else if (order->GetState() == Order::State::Dropped)
		{
			OrderBus::Get().IssueDelayedOrder(order);
			order->SetState(Order::State::Initial);
			order = nullptr;
		}
	}
}

bool Creature::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		if (order)
			ImGui::Text("Order: %s", order->to_string());
		else
			ImGui::Text("Order: none");

		if (ImGui::TreeNode("Self orders"))
		{
			if (personal.empty())
				ImGui::Text("empty");

			for (const auto &i : personal)
			{
				if (i)
					ImGui::Text(i->to_string().c_str());
			}
			ImGui::TreePop();
		}
	}
	return true;
}

void Creature::Requirements()
{
	//REQUIRE(Chest);
}

void Creature::OnCreate(const GameObjectParams & params)
{
	uid = global_uid;
	global_uid++;

	params.world->RegisterCreature(mParent->shared_from_this());
}

void Creature::OnDestroy(const GameObjectParams& params)
{
	EventBus::Get().Publish<EventCreatureDeath>(mParent->shared_from_this());
}

void Creature::AddPersonal(POrder o, float priority /* = 0.5 */)
{
	std::remove_if(personal.begin(), personal.end(), [&](const POrder &p)->bool {
		if (!p)
			return true;
		return p->IsEquals(o);
	});

	personal.push_front(o);
}

PAgent WalkingPossibility::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<WalkingPossibility>(*this);
	t->mParent = parent;
	return t;
}

void WalkingPossibility::Update(const GameObjectParams & params)
{
}

bool WalkingPossibility::DrawGui(const GameObjectParams& params, float gt)
{
	ImGui::Text("Can walk");
	return true;
}

PAgent CrawlingPossibility::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<CrawlingPossibility>(*this);
	t->mParent = parent;
	return t;
}

void CrawlingPossibility::Update(const GameObjectParams & params)
{
}

bool CrawlingPossibility::DrawGui(const GameObjectParams& params, float gt)
{
	ImGui::Text("Can crawl");
	return true;
}

PAgent WaterConsumer::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<WaterConsumer>(*this);
	t->mParent = parent;
	return t;
}

void WaterConsumer::Update(const GameObjectParams & params)
{
	water -= params.dt / 10.f;
}

bool WaterConsumer::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("Water: %g", water);
	}

	if (water <= 0)
	{
		ImGui::TextColored({ 1,0,0,1 }, "Dying of thirst");
		mParent->GetAgent<Anatomic>()->Think("I'm dying of thirst :(");
	}
	else if (water <= full / 3.f)
	{
		ImGui::TextColored({ 1,1,0,1 }, "Strong thirst");
		mParent->GetAgent<Anatomic>()->Think("I'm feel strong thirst :(");
	}
	else if (water <= full / 1.5f)
	{
		ImGui::TextColored({ 0.5,1,0,1 }, "Thirst");
		mParent->GetAgent<Anatomic>()->Think("I'm feel thirst :(");
	}
	else
	{
		ImGui::TextColored({ 0,1,0,1 }, "Full");
	}
	return true;
}

PAgent CalorieConsumer::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<CalorieConsumer>(*this);
	t->mParent = parent;
	return t;
}

void CalorieConsumer::Update(const GameObjectParams & params)
{
	calorie -= params.dt / 10.f;

	if (want_to_eat)
		if (auto c = mParent->GetAgent<Creature>())
		{
			auto &storages = params.world->GetStorages();
			auto tpos = storages.begin()->first;
			if (storages.size() > 0)
			{
				auto ch = storages.begin()->second->GetAgent<Chest>();
				auto i = ch->GetByPredicate([](const ChestSlot &o)->bool
				{
					return o.obj && o.obj->GetAgent<Food>();
				});

				if (i.obj)
				{
					c->AddPersonal(std::make_shared<OrderEat>(tpos, i.obj));
					want_to_eat = false;
				}
			}
		}
}

bool CalorieConsumer::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("Calorie: %g", calorie);
	}

	if (calorie <= 0)
	{
		ImGui::TextColored({ 1,0,0,1 }, "Starwing");
		mParent->GetAgent<Anatomic>()->Think("I'm starwing :(");
		want_to_eat = true;
	}
	else if (calorie <= full / 3.f)
	{
		ImGui::TextColored({ 1,1,0,1 }, "Very hungry");
		mParent->GetAgent<Anatomic>()->Think("I'm very hungry :(");
		want_to_eat = true;
	}
	else if (calorie <= full / 1.5f)
	{
		ImGui::TextColored({ 0.5,1,0,1 }, "Hungry");
		mParent->GetAgent<Anatomic>()->Think("I'm hungry :(");
		want_to_eat = true;
	}
	else
	{
		ImGui::TextColored({ 0,1,0,1 }, "Full");
	}
	return true;
}

PAgent Morale::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Morale>(*this);
	t->mParent = parent;
	return t;
}

void Morale::Update(const GameObjectParams & params)
{
}

bool Morale::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("Morale: %g", morale);
	}

	if (morale >= 50)
	{
		ImGui::TextColored({ 0,1,0.5,1 }, "High morale");
	}
	else if (morale >= 40)
	{
		ImGui::TextColored({ 0,1,0,1 }, "Normal morale");
	}
	else if (morale >= 30)
	{
		ImGui::TextColored({ 0.5,1,0,1 }, "Slightly fatigued");
	}
	else if (morale >= 20)
	{
		ImGui::TextColored({ 1,1,0,1 }, "Moderately fatigued");
	}
	else
	{
		ImGui::TextColored({ 1,0,0,1 }, "Seriously fatigued");
		mParent->GetAgent<Anatomic>()->Think("Life is pointless :(");
	}
	return true;
}

Anatomic::Anatomic()
{
	minds.resize(25);
}

PAgent Anatomic::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Anatomic>(*this);
	t->mParent = parent;
	return t;
}

void Anatomic::Update(const GameObjectParams & params)
{
}

void Anatomic::Afterload(GameObject * parent)
{
}

bool Anatomic::DrawGui(const GameObjectParams& params, float gt)
{
	if (ImGui::TreeNode("Mind"))
	{
		for (int i = 0; i < minds.size(); i++)
		{
			if (!(minds[i].mind.empty()))
				ImGui::Text(minds[i].mind.c_str());
		}
		ImGui::TreePop();
	}
	return true;
}

void Anatomic::Think(const boost::format &s)
{
	Think(s.str());
}

void Anatomic::Think(const std::string &s)
{
	auto f = std::find_if(minds.begin(), minds.end(), [&](const Mind &m) -> bool { return m.mind == s; });
	if (f == minds.end())
	{
		minds.push_back({ s });
	}
}

void Anatomic::Interest(const std::string & s, glm::vec3 & p)
{
	interest_points[StringIntern(s)] = p;
	Think((boost::format("I see some %1% :|") % s).str());
}

PAgent Named::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Named>(*this);
	t->mParent = parent;
	return t;
}

void Named::Update(const GameObjectParams & params)
{
}

std::vector<std::string> LoadNames(const std::string &name)
{
	std::ifstream file(name.c_str());
	std::vector<std::string> t;
	t.reserve(200);

	if (file.is_open())
	{
		std::string line = "";
		while (getline(file, line))
		{
			t.push_back(line);
		}
		file.close();
		return std::move(t);
	}
	else
	{
		LOG(fatal) << name << "missed";
		return{};
	}
}

void Named::OnCreate(const GameObjectParams & params)
{
	static std::vector<std::string> fname = []() -> auto { return LoadNames("data/loc/female_names.txt"); }(),
		mname = []() -> auto { return LoadNames("data/loc/male_names.txt"); }(),
		sname = []() -> auto { return LoadNames("data/loc/second_names.txt"); }();

	if (gender == 'f')
	{
		name = fname[rand() % fname.size()] + " " + sname[rand() % sname.size()];
	}
	else
	{
		name = mname[rand() % mname.size()] + " " + sname[rand() % sname.size()];
	}
}

bool Named::DrawGui(const GameObjectParams& params, float gt)
{
	ImGui::Text("Name: %s", name.c_str());
	return true;
}

PAgent DeathDrop::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<DeathDrop>(*this);
	t->mParent = parent;
	return t;
}

void DeathDrop::Update(const GameObjectParams & params)
{
}

void DeathDrop::OnDestroy(const GameObjectParams & params)
{
	auto i = DB::Get().Create(id);
	params.world->Place(params.pos, i);
	OrderBus::Get().IssueOrder(std::make_shared<OrderGet>(params.pos, i));
}

void DeathDrop::JsonLoad(const rapidjson::Value & val)
{
	JSONLOAD(NVP(id));
}

PAgent Wander::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Wander>(*this);
	t->mParent = parent;
	return t;
}

void Wander::Update(const GameObjectParams & params)
{
	if (auto c = mParent->GetAgent<Creature>())
	{
		auto p = mParent->GetAgent<PositionAgent>();
		auto npos = glm::ivec3(p->Get().x + rand() % 3 - 1, p->Get().y + rand() % 3 - 1, p->Get().z);
		if (params.world->IsWalkable(npos))
		{
			//c->AddPersonal(std::make_shared<OrderWander>(npos));
		}
	}
}

PAgent Food::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Food>(*this);
	t->mParent = parent;
	return t;
}

void Food::JsonLoad(const rapidjson::Value & val)
{
	JSONLOAD(NVP(nutrition));
}

PAgent ActivityConsumer::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<ActivityConsumer>(*this);
	t->mParent = parent;
	return t;
}

void ActivityConsumer::Update(const GameObjectParams & params)
{
	activity -= params.dt / 5.f;

	if (activity <= 5)
	{
		if (auto c = mParent->GetAgent<Creature>())
		{
			c->AddPersonal(std::make_shared<OrderSleep>(mParent->GetAgent<PositionAgent>()->Get()));
		}
	}
}

bool ActivityConsumer::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("Activity: %g", activity);
	}

	if (activity >= 50)
	{
		ImGui::TextColored({ 0,1,0.5,1 }, "Rested");
	}
	else if (activity >= 40)
	{
		ImGui::TextColored({ 0,1,0,1 }, "Not tired");
	}
	else if (activity >= 30)
	{
		ImGui::TextColored({ 0.5,1,0,1 }, "Slightly tired");
	}
	else if (activity >= 20)
	{
		ImGui::TextColored({ 1,1,0,1 }, "Moderately tired");
	}
	else
	{
		ImGui::TextColored({ 1,0,0,1 }, "Seriously tired");
		mParent->GetAgent<Anatomic>()->Think("I'm so tired :(");
	}
	return true;
}

void ActivityConsumer::Tire(float t)
{
	activity -= t;
}

bool ActivityConsumer::IsTired() const
{
	return activity <= 20;
}

bool ActivityConsumer::IsRested() const
{
	return activity >= 100;
}

size_t Creature::global_uid = 0;

std::string Relationships::to_string() const
{
	return std::to_string(value);
}

PAgent Talker::Clone(GameObject * parent, const std::string & name)
{
	auto t = MakeAgent<Talker>(*this);
	t->mParent = parent;
	return t;
}

void Talker::Update(const GameObjectParams & params)
{
	if (auto c = mParent->GetAgent<Creature>())
	{
		auto p = mParent->GetAgent<PositionAgent>();
		auto in_cell = params.world->GetCreaturesAt(p->Get());
		if (in_cell.size() > 1)
			for (const auto &in : in_cell)
			{
				auto it = in_cell.begin();
				std::advance(it, rand() % in_cell.size());
				c->AddPersonal(std::make_shared<OrderTalk>(*it));
			}
	}
}

bool Talker::DrawGui(const GameObjectParams& params, float gt)
{
	if (ImGui::TreeNode("Relationships"))
	{
		for (auto &r : relationships)
		{
			ImGui::Text(r.second.with.c_str());
		}
		ImGui::TreePop();
	}
	return true;
}

PAgent ProfessionPerformer::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<ProfessionPerformer>(*this);
	t->mParent = parent;

	auto pp = t->prof;
	t->prof.clear();

	for (const auto & p : pp)
	{
		t->prof.push_back(p->Clone());
	}

	return t;
}

void ProfessionPerformer::Update(const GameObjectParams& params)
{
	for (const auto &p : prof)
	{
		p->Perform(params, mParent->shared_from_this());
	}
}

bool ProfessionPerformer::DrawGui(const GameObjectParams& params, float gt)
{
	if (ImGui::TreeNode("Professions"))
	{
		for (const auto &p : prof)
		{
			ImGui::Text(p->Name().c_str());
		}
		ImGui::TreePop();
	}
	return true;
}

bool ProfessionPerformer::CanPeformOrder(POrder o)
{
	std::list<POrder> ords;
	if (o->GetId() == Order::Idfor<OrderCombined>())
	{
		auto com = std::static_pointer_cast<OrderCombined>(o);
		if (com->need_to_perform == 0)
		{
			for (auto & oc : com->orders)
			{
				ords.push_back(oc);
			}
		}
		else
		{
			ords.push_back(o);
		}
	}
	else
		ords.push_back(o);

	bool all_ok = true;
	for (auto & or : ords)
	{
		bool ok = false;
		for (const auto &p : prof)
		{
			if (p->CanPeformOrder(or ) && p->GetActive())
				ok = true;
		}

		if (!ok)
		{
			all_ok = false;
			break;
		}
	}

	return all_ok;
}

void ProfessionPerformer::JsonLoad(const rapidjson::Value& val)
{
	std::vector<ProfLoadHelper> prof;
	JSONLOAD(sge::make_nvp("data", prof));

	for (const auto &p : prof)
	{
		auto t = ProfessionFactory::Get().Create(p.name);
		t->SetLevel(static_cast<ProfessionLevel>(p.level));
		t->SetActive(p.active);

		this->prof.push_back(t);
	}
}

float ProfessionPerformer::GetSalary()
{
	float sal = 0;
	for (const auto & p : prof)
	{
		if (p->GetActive())
			sal += p->GetCost();
	}

	return sal;
}

void ProfessionPerformer::ProfLoadHelper::JsonLoad(const rapidjson::Value& val)
{
	JSONLOAD(NVP(name), NVP(level), NVP(active));
}

PAgent ChainDestruction::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<ChainDestruction>(*this);
	t->mParent = parent;
	return t;
}

void ChainDestruction::Update(const GameObjectParams& params)
{
}

bool ChainDestruction::DrawGui(const GameObjectParams& params, float gt)
{
	return true;
}

void ChainDestruction::OnDestroy(const GameObjectParams& params)
{
	if (destroyed)
		return;

	destroyed = true;

	std::vector<glm::vec3> neib = { { 0,  0, -1 }, {  0, 0, 1 },
								   { 1,  0,  0 }, { -1, 0, 0 },
								   { 0, -1,  0 }, {  0, 1, 0 } };

	for (const auto &n : neib)
	{
		if (auto ne = params.world->GetBlock(params.pos + n))
		{
			auto dest_it = std::find(destroys.begin(), destroys.end(), ne->GetId());
			if (dest_it != destroys.end())
				params.world->SetBlock(params.pos + n, nullptr);
		}
	}
}

void ChainDestruction::JsonLoad(const rapidjson::Value& val)
{
	JSONLOAD(NVP(destroys));
}

PAgent Workshop::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<Workshop>(*this);
	t->mParent = parent;
	return t;
}

void Workshop::Update(const GameObjectParams& params)
{
}

struct CompleteCraft
{
	CompleteCraft(const RecipeIn * i) : input(i) {}
	const RecipeIn * input;
	StringIntern selected;
};

std::vector<CompleteCraft> to_complete;
std::vector<const RecipeIn *> already_completed;

bool Workshop::DrawGui(const GameObjectParams& params, float gt)
{
	auto rec = DB::Get().GetMachineRecipe(mParent->GetId());
	static PRecipe r_pop;
	for (const auto &a : rec)
	{
		bool pressed = false;
		a->DrawGui(gt, pressed);
		if (pressed)
		{
			r_pop = a;
			ImGui::OpenPopup("Select exact materials");
			to_complete.clear();
			already_completed.clear();
			for (const auto & r : r_pop->input)
			{
				auto ofs = r.id.get().find("tag_");
				if (ofs != std::string::npos)
					to_complete.emplace_back(&r);
				else
					already_completed.push_back(&r);
			}
		}

		ImGui::Separator();
	}

	bool open = true;
	if (ImGui::BeginPopupModal("Select exact materials", &open))
	{
		Settings::Get().pause = true;
		std::list<ChestSlot*> all_items;
		const auto &all = Storages::Get().List();
		int inrow = 0;
		for (const auto &s : all)
		{
			auto ch = std::get<1>(s)->GetAgent<Chest>();
			for (auto & i : ch->slots)
			{
				if (i.obj)
					all_items.push_back(&i);
			}
		}

		int i = 0;
		int completed = 0;
		for (auto & r : to_complete)
		{
			ImGui::BeginGroup();
			ImGui::Text(r.input->id.get().substr(4).c_str());
			ImGui::BeginChild(ImGui::GetID(reinterpret_cast<void *>(i)), ImVec2(ImGui::GetWindowWidth() * 0.95f, 100.0f), true);

			auto taglist = DB::Get().Taglist(r.input->id);
			int jj = 0;
			for (const auto & t : taglist)
			{
				auto res = std::find_if(all_items.begin(), all_items.end(), [&](const ChestSlot * s)->bool {
					return s->obj->GetId() == t;
				});

				if (res != all_items.end())
				{
					if (jj < 9)
						ImGui::SameLine(), jj++;
					else
						jj = 0;

					(*res)->DrawGui(t == r.selected);
					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
						r.selected = t;
				}

				if (!r.selected.get().empty() && t == r.selected)
					completed++;
			}

			ImGui::EndChild();
			ImGui::EndGroup();
			i++;
		}

		if (ImGui::Button("Done"))
			if (completed == to_complete.size())
			{
				auto comb = std::make_shared<OrderCombined>();

				std::map<StringIntern, StringIntern> tag_map;
				for (const auto & c : to_complete)
				{
					auto ord = std::make_shared<OrderFind>(c.selected, c.input->count);
					comb->PushOrder(ord);
					tag_map[c.input->id] = c.selected;
				}
				for (const auto & c : already_completed)
				{
					auto ord = std::make_shared<OrderFind>(c->id, c->count);
					comb->PushOrder(ord);
				}
				auto cord = std::make_shared<OrderCraft>(params.pos, r_pop, 1);
				cord->tag_map = tag_map;

				comb->PushOrder(cord);

				if (!performer.get().empty())
					comb->need_to_perform = ProfessionFactory::Get().Create(performer)->GetId();

				OrderBus::Get().IssueOrder(comb);
			}

		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	return true;
}

void Workshop::JsonLoad(const rapidjson::Value& val)
{
	JSONLOAD(NVP(machine), NVP(performer));
}

//-------------------------------------------------------------------

PAgent EnergyProducer::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<EnergyProducer>(*this);
	t->mParent = parent;
	return t;
}

void EnergyProducer::Update(const GameObjectParams& params)
{
}

bool EnergyProducer::DrawGui(const GameObjectParams& params, float gt)
{
	ImGui::Text("EnergyProducer");
	ImGui::Text("producing %fv %fa", voltage, amperage);
	ImGui::Text("buffer %f / %f", buffer, buffer_size);

	return true;
}

void EnergyProducer::JsonLoad(const rapidjson::Value& val)
{
	JSONLOAD(NVP(amperage), NVP(voltage), NVP(buffer_size));
}

void EnergyProducer::ProduceEnergy(float power)
{
	buffer += power / voltage;

	if (buffer > buffer_size)
		buffer = buffer_size;
}

//-------------------------------------------------------------------

PAgent EnergyConsumer::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<EnergyConsumer>(*this);
	t->mParent = parent;
	return t;
}

void EnergyConsumer::Update(const GameObjectParams& params)
{
}

bool EnergyConsumer::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("EnergyConsumer");
		return true;
	}

	return false;
}

void EnergyConsumer::JsonLoad(const rapidjson::Value& val)
{
}

//-------------------------------------------------------------------

PAgent EnergyWire::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<EnergyWire>(*this);
	t->mParent = parent;
	return t;
}

void EnergyWire::Update(const GameObjectParams& params)
{
}

bool EnergyWire::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("EnergyWire");
		return true;
	}

	return false;
}

void EnergyWire::JsonLoad(const rapidjson::Value& val)
{
}

//-------------------------------------------------------------------

PAgent SteamGenerator::Clone(GameObject* parent, const std::string& name)
{
	auto t = MakeAgent<SteamGenerator>(*this);
	t->mParent = parent;
	return t;
}

void SteamGenerator::Update(const GameObjectParams& params)
{
	if (auto lp = mParent->GetAgent<LiquidPipe>())
	{
		if (lp->GetLiquidID() == StringIntern("material_steam"))
		{
			auto count = lp->GetLiquidCount();
			lp->SetLiquidCount(0);

			if (auto ep = mParent->GetAgent<EnergyProducer>())
			{
				ep->ProduceEnergy(count * efficiency);
			}
		}
	}
}

bool SteamGenerator::DrawGui(const GameObjectParams& params, float gt)
{
	if (Settings::Get().IsDebug())
	{
		ImGui::Text("SteamGenerator");
		ImGui::Text("efficiency %f%", efficiency * 100);
		return true;
	}

	return false;
}

void SteamGenerator::JsonLoad(const rapidjson::Value& val)
{
	JSONLOAD(NVP(efficiency));
}