#include "Recipe.h"
#include <Serealize.h>
#include <list>
#include <Core\DB.h>
#include <imgui.h>
#include <render\TextureManager.h>
#include <Game.h>
#include <gui\WindowRecipe.h>
#include <Core\agents\Chest.h>

void RecipeIn::JsonLoad(const rapidjson::Value & val)
{
	id = val.Begin()->GetString();
	if (val.Capacity() >= 2)
		count = val[1].GetInt();
}

void RecipeOut::JsonLoad(const rapidjson::Value & val)
{
	id = val.Begin()->GetString();
	if (val.Capacity() >= 2)
		count = val[1].GetInt();
	if (val.Capacity() >= 3)
		chance = val[2].GetDouble();
}

std::string myreplace(std::string s, const std::string &toReplace, const std::string &replaceWith)
{
	const auto &t = s.find(toReplace);
	if (t == std::string::npos)
		return s;

	return s.replace(t, toReplace.length(), replaceWith);
}

void DrawSprite(const StringIntern &s, ImColor c)
{
	auto &atl = TextureManager::Get().GetTexture(s);
	auto &tex = std::get<0>(atl);
	auto &atluv = std::get<1>(atl);

	auto uv = glm::vec2(atluv.x, atluv.y);
	auto uv2 = glm::vec2(atluv.z, atluv.w);

	ImGui::ImageButton(reinterpret_cast<ImTextureID>(tex->GetId()), { 32,32 }, uv2, uv, -1, c);

	if (s != "arrow_right" && ImGui::IsItemHovered())
	{
		if (ImGui::IsKeyPressed(GLFW_KEY_R))
		{
			WindowRecipe::Get().ShowRecipe(s);
		}

		if (ImGui::IsKeyPressed(GLFW_KEY_U))
		{
			WindowRecipe::Get().ShowUsing(s);
		}
	}

}

void DrawSome(const StringIntern &s, float gt, ImColor c = {0,0,0,0})
{
	static float tt = 0;
	tt += gt;

	auto ss = s.get();
	if (ss.find("tag_") != -1)
	{
		auto list = DB::Get().Taglist(ss);
		srand(int(tt));
		if (!list.empty())
			ss = list[0];
	}

	auto t = DB::Get().mObjects.find(StringIntern(ss));
	if (t != DB::Get().mObjects.end())
	{
		auto &a = *t;
		DrawSprite(a.first, c);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("%s\n%s", a.first.get().c_str(), std::get<0>(a.second)->GetDescription().c_str());
		}
	}
}

bool Recipe::DrawGui(float gt, bool & pressed)
{
	bool first = true;
	pressed = false;
	for (const auto &inp : input)
	{
		if (first)
			first = false;
		else
			ImGui::SameLine();

		DrawSome(inp.id, gt);
		auto draw_list = ImGui::GetWindowDrawList();

		if (inp.count >= 100)
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(22, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(inp.count).c_str());
		else if (inp.count >= 10)
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(16, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(inp.count).c_str());
		else
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(10, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(inp.count).c_str());
	}

	{
		auto atl = TextureManager::Get().GetTexture("arrow_right");
		auto &tex = std::get<0>(atl);
		auto &atluv = std::get<1>(atl);

		auto uv = glm::vec2(atluv.x, atluv.y);
		auto uv2 = glm::vec2(atluv.z, atluv.w);

		ImGui::SameLine();
		ImGui::ImageButton(reinterpret_cast<ImTextureID>(tex->GetId()), { 32,32 }, uv2, uv);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("craft this recipe");
			if (ImGui::IsMouseClicked(0))
				pressed = true;
		}
	}

	for (const auto &out : output)
	{
		ImGui::SameLine();
		DrawSome(out.id, gt);
		auto draw_list = ImGui::GetWindowDrawList();

		if (out.count >= 100)
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(22, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(out.count).c_str());
		else if (out.count >= 10)
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(16, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(out.count).c_str());
		else
			draw_list->AddText(ImGui::GetItemRectMax() - ImVec2(10, 13), ImGui::GetColorU32(ImGuiCol_Text), std::to_string(out.count).c_str());
	}

	return true;
}

// ������������� ��� ������ tag_* � ��������������� ������ id
std::list<PRecipe> Recipe::Expand()
{
	if (materials.empty())
		return std::list<PRecipe>();

	std::vector<StringIntern> expanded;
	for (const auto &s : materials)
	{
		if (s.get().find("tag_") != std::string::npos)
		{
			auto taglist = DB::Get().Taglist(s);
			expanded.reserve(expanded.size() + taglist.size());
			for (const auto &t : taglist)
			{
				expanded.push_back(t);
			}
			LOG(trace) << s.get() << " expanded to " << taglist.size() << " materials";
		}
	}

	std::list<PRecipe> list;
	for (const auto &s : expanded)
	{
		auto nr = static_cast<PRecipe>(std::make_shared<Recipe>(*this));
		for (auto &inp : nr->input)
		{
			inp.id = StringIntern(myreplace(inp.id.get(), "%material%", s));
		}

		for (auto &out : nr->output)
		{
			out.id = StringIntern(myreplace(out.id.get(), "%material%", s));
		}
		list.push_back(nr);
	}

	return list;
}

void Recipe::JsonLoad(const rapidjson::Value & val)
{
	JSONLOAD(NVP(input), NVP(output), NVP(tools), NVP(machine), NVP(duration), NVP(materials));
}

bool Recipe::CraftIn(Chest & c, int count, std::map<StringIntern, StringIntern> tag_map)
{
	auto backup = c.slots;

	bool enought = true;
	for (const auto &i : input)
	{
		auto check = c.GetByPredicate([&](const ChestSlot &cs)->bool {
			if (i.id.get().find("tag_") == std::string::npos)
				return cs.obj->GetId() == i.id;
			else
				return cs.obj->GetId() == tag_map[i.id];
		});
		if (check.obj == nullptr || check.count < i.count)
		{
			enought = false;
			break;
		}
	}

	if (!enought)
		return false;

	for (const auto &i : input)
	{
		auto remove = c.PopByPredicate([&](const ChestSlot &cs)->bool {
			if (i.id.get().find("tag_") == std::string::npos)
				return cs.obj->GetId() == i.id;
			else
				return cs.obj->GetId() == tag_map[i.id];
		});

		remove.count -= i.count;
		if (remove.count > 0)
			c.Push(remove.obj, remove.count);
	}

	for (const auto &o : output)
	{
		float prob = (rand() / float(RAND_MAX));
		if (o.chance >= prob)
		{
			if (!c.Push(DB::Get().Create(o.id), o.count))
			{
				c.slots = backup;
				return false;
			}
		}
	}

	return true;
}

bool DeepRecipe::DrawGui(float gt)
{
	ImColor col = { 0,0,0,0 };
	if (incomplete)
		col = { 255,0,0,128 };

	if (recipe)
	{
		for (const auto &outp : recipe->output)
		{
			DrawSome(outp.id, gt, col);
		}

		if (first_show)
		{
			//ImGui::SetNextTreeNodeOpened(true);
			first_show = false;
		}
		if (ImGui::TreeNode(""))
		{

			for (int i = 0; i < recipe->input.size(); i++)
			{
				if (input_expantion.size() > i && input_expantion[i])
					input_expantion[i]->DrawGui(gt);
				else
					DrawSome(recipe->input[i].id, gt, col);

			}

			ImGui::TreePop();
		}
	}

	return true;
}
