﻿#include "Game.h"

#include <gl/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/thread/thread.hpp>
#include <boost/circular_buffer.hpp>

#include <memory>
#include <vector>
#include "tools/Bresenham3D.h"
#include "tools/CoordSystem.h"
#include "Core/MapGen/WorldWorker.h"
#include "Render/TextureManager.h"
#include "Core/DB.h"
#include "Core/SplitBlockTessellator.h"
#include "tools/Log.h"
#include "tools/wset.h"

#include "gui/imgui_impl_glfw_gl3.h"
#include "imgui.h"
#include "imgui\imgui_internal.h"
#include "gui/WS.h"
#include <gui/WindowInventory.h>
#include <gui/WindowCraftQueue.h>
#include <gui/WindowPerfomance.h>
#include <Render/Resources.h>
#include <tools/ray.h>
#include <core/agents/Chest.h>
#include <Render/ParticleSystem.h>
#include <core/EventBus.h>
#include <core/orders/OrderBus.h>
#include "gui/WindowRooms.h"

#include "Core/agents/Ore.h"
#include <core/orders/OrderDig.h>
#include <Core/orders/OrderPlace.h>
#include "gui/WindowDb.h"
#include "gui/WindowTools.h"
#include <boost/filesystem/convenience.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include "gui/WindowEsc.h"
//#include "imgui/imgui_user.h"
#include <boost/exception/diagnostic_information.hpp>
#include "gui/WindowOverlay.h"
#include "Localize.h"
#include "Core/orders/OrderCombined.h"
#include "Core/orders/OrderFindAndPick.h"

//
#include "GamePhase_LoadMenu.h"
#include "MapTemplate.h"

GamePhase_Game::GamePhase_Game()
{
	mRenderSector = std::make_unique<RenderSector>();
	mWorld = std::make_unique<World>();

	mTessellator = std::make_unique<Tessellator>(*mRenderSector);
	mTessellator->SayCamera(Game::GetCamera());
	mWorld->SetTessellator(mTessellator.get());
	mTessellator->Run();
	WS::Get().w = mWorld.get();

	DB::Get().ReloadDirectory("data\\json\\");
	MapTemplates::Get().ReloadDirectory("data\\patterns\\other");

	Game::SetWorker(std::make_unique<WorldWorker>(mWorld));

	gen_thread = boost::thread([&]() {
		while (true)
		{
			Game::GetWorker()->Process();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}
	});

	mSectorLoader = std::make_unique<SectorLoader>(*mWorld, SPos{}, 5);

	Currency::Get().AddCurrency(10000);
	mWorld->SetTime("Jun 1 2100 12:00:00");

	initialized = true;
}

void GamePhase_Game::generateShadowFBO()
{
	depthTextureId = std::make_shared<Texture>();
	depthTextureId->DepthTexture({ 1024, 1024 });

	GLenum FBOstatus;

	if (fboId)
		glDeleteFramebuffers(1, &fboId);

	fboId = -1;

	try
	{
		glGenFramebuffers(1, &fboId);
		glBindFramebuffer(GL_FRAMEBUFFER, fboId);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId->GetId(), 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	catch (std::exception& e)
	{
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			LOG(error) << "Shadow fbo exception: " << boost::diagnostic_information(e);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GamePhase_Game::~GamePhase_Game()
{
	gen_thread.interrupt();
	gen_thread.join();

	LOG(trace) << "quit";

	mTessellator->Interrupt();
	mTessellator.release();

	mWorld.reset();
	mRenderSector.reset();
}

void GamePhase_Game::Draw(float dt)
{
	float drawtime = static_cast<float>(glfwGetTime());
	//mCamera->SetPos(mWorld->GetPlayer()->GetPosition() + glm::vec3{ 0.0f, 0.0f, 1.7f });
	//mCamera->SetRot(mWorld->GetPlayer()->GetRot());
	Game::GetCamera()->Update();

	static float phi = 0.f;
	//phi += dt / 20.f;
	Game::GetSun()->SetPos(Game::GetCamera()->GetPos() + glm::vec3{ glm::sin(phi) + glm::cos(phi), 0, -glm::sin(phi) + glm::cos(phi) });
	Game::GetSun()->LookAt(Game::GetCamera()->GetPos());
	Game::GetSun()->Update();

	static bool wire = true;

	if (ImGui::IsKeyPressed(GLFW_KEY_F2, false))
		wire = wire ? (glPolygonMode(GL_FRONT_AND_BACK, GL_LINE), false) :
		(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL), true);

	static int skip = 1;
	skip++;

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glViewport(0, 0, depthTextureId->GetSize().x, depthTextureId->GetSize().y);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glCullFace(GL_FRONT);
	mRenderSector->ShadowDraw(*Game::GetSun(), Resources::Get().GetShader("shaders/shadow.glsl"));
	skip = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Game::GetWindow()->GetFbSize().x, Game::GetWindow()->GetFbSize().y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glCullFace(GL_BACK);
	auto col = glm::mix(glm::vec4(117.0f / 255.0f, 187.0f / 255.0f, 253.0f / 255.0f, 1.0f), glm::vec4(0, 0.1, 0.2, 1), (glm::sin(phi) - glm::cos(phi) + 1) / 2.f);
	glClearColor(col.x, col.y, col.z, col.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	depthTextureId->Set(TEXTURE_SLOT_2);
	mRenderSector->Draw(*Game::GetCamera(), *Game::GetSun());

	Game::GetRender()->Draw(*Game::GetCamera());

	ParticleSystem::Get().Draw(*Game::GetCamera());
	ParticleSystem::Get().Update(dt);

	auto ray = Game::GetCamera()->GetRay(ImGui::GetMousePos());
	std::tuple<glm::ivec3, glm::vec3> selection_pos; // pos, normal

	static auto select_model = Game::GetRender()->AddModel("data/models/selection.obj", "selection_y", "shaders/basic.glsl");
	static auto square_model = Game::GetRender()->AddModel("data/models/selection.obj", "selection_y", "shaders/basic.glsl");

	int j = 0;
	auto ord = OrderBus::Get().orders.begin();
	for (auto &i : OrderBus::Get().orders)
	{
		if (i->GetId() == Order::Idfor<OrderDig>())
		{
			auto od = std::static_pointer_cast<OrderDig>(i);
			auto tex_tuple = TextureManager::Get().GetTexture("selection_y");
			auto tex4 = std::get<1>(tex_tuple);
			glm::vec2 uv1 = { tex4.x, tex4.y };
			glm::vec2 uv2 = { tex4.z, tex4.w };

			Game::GetSpriteBatch()->AddCube(od->pos + glm::vec3(0, 0, 1.05), od->pos + glm::vec3(1, 1, 1.05), uv1, uv2, std::get<0>(tex_tuple));
		}
	}

	if (WindowRooms::Get().GetVisibility())
	{
		for (auto &i : mWorld->rooms)
		{
			for (auto &j : i->cells)
			{
				auto tex_tuple = TextureManager::Get().GetTexture("selection_y");
				auto tex4 = std::get<1>(tex_tuple);
				glm::vec2 uv1 = { tex4.x, tex4.y };
				glm::vec2 uv2 = { tex4.z, tex4.w };

				Game::GetSpriteBatch()->AddCube(glm::vec3(j) + glm::vec3(0, 0, 1.05), glm::vec3(j) + glm::vec3(1, 1, 1.05), uv1, uv2, std::get<0>(tex_tuple), { 1,0,0,1 });
			}
		}
	}


	static std::unordered_map<glm::ivec3, PGameObject> opened_w;

	if (WindowTools::Get().selected != SelectedOrder::DIG_SQUARE || WindowTools::Get().selected != SelectedOrder::PLACE_BLOCK)
	{
		selection_pos = PickFirst(ray.origin(), ray.dir(), 100.f, [&](const glm::ivec3 &pos)->bool {
			return pos.z == mWorld->GetSlise();
		});
	}
	else
		selection_pos = PickFirst(ray.origin(), ray.dir(), 100.f, [&](const glm::ivec3 &pos)->bool {
		return !!mWorld->GetBlock(pos).get();
	});

	Game::GetRender()->SetModelMatrix(select_model, glm::translate(glm::mat4(1), glm::vec3(std::get<0>(selection_pos) + 1000)));
	if (!ImGui::IsPosHoveringAnyWindow(ImGui::GetMousePos()) && !Settings::Get().pause)
	{
		static glm::vec3 min, max;
		static int minmaxstate = 0;
		if (WindowTools::Get().selected != SelectedOrder::MARK_AS_ROOM && WindowTools::Get().selected != SelectedOrder::DIG_CIRCLE &&
			WindowTools::Get().selected != SelectedOrder::DIG_SQUARE)
		{
			min = max = glm::vec3{ std::numeric_limits<float>().max() };
			minmaxstate = 0;
		}

		if (ImGui::IsMouseDown(0))
		{
			switch (WindowTools::Get().selected)
			{
			case SelectedOrder::MARK_AS_ROOM:
			case SelectedOrder::DIG_CIRCLE:
			case SelectedOrder::DIG_SQUARE:
				switch (minmaxstate)
				{
				case 0:
					min = std::get<0>(selection_pos);
					minmaxstate = 1;
					break;
				}
				break;
			case SelectedOrder::PLACE_BLOCK:
				auto or_get = std::make_shared<OrderFind>(WindowDb::Get().GetSelectedId(), 1);
				auto or_pl = std::make_shared<OrderPlace>(std::get<0>(selection_pos), WindowDb::Get().GetSelectedId());
				auto or_comb = std::make_shared<OrderCombined>();
				or_comb->PushOrder(or_get);
				or_comb->PushOrder(or_pl);

				OrderBus::Get().IssueOrder(or_comb);
				break;
			}
		}

		if (minmaxstate == 1)
		{
			if (!ImGui::IsMouseDown(0))
			{

				max = std::get<0>(selection_pos);

				glm::ivec3 s1 = { glm::min(min.x, max.x), glm::min(min.y, max.y), glm::min(min.z, max.z) };
				glm::ivec3 s2 = { glm::max(min.x, max.x), glm::max(min.y, max.y), glm::max(min.z, max.z) };

				auto r = std::make_shared<Room>();
				r->min = s1;
				r->max = s2;

				for (auto i = s1.x; i <= s2.x; i++)
					for (auto j = s1.y; j <= s2.y; j++)
						for (auto k = s1.z; k <= s2.z; k++)
						{
							if (WindowTools::Get().selected == SelectedOrder::DIG_SQUARE ||
								(WindowTools::Get().selected == SelectedOrder::DIG_CIRCLE && glm::distance(glm::vec3(i, j, k), glm::vec3(s1 + s2) / 2.f) <= glm::distance(glm::vec3(s1), glm::vec3(s2)) / 2.f))
								if (auto b = mWorld->GetBlock(glm::vec3{ i,j,k }))
								{
									if (Settings::Get().dig_ores || !b->HasAgent<Ore>())
										OrderBus::Get().IssueOrder(std::make_shared<OrderDig>(glm::vec3{ i,j,k }));
								}

							if (WindowTools::Get().selected == SelectedOrder::MARK_AS_ROOM)
							{
								r->cells.push_back({ i,j,k });
								WindowRooms::Get().selected = r;
								if (*(--mWorld->rooms.end()) != r)
									mWorld->rooms.push_back(r);
							}
						}

				minmaxstate = 0;
			}
			else
			{
				max = std::get<0>(selection_pos);

				glm::ivec3 s1 = { glm::min(min.x, max.x), glm::min(min.y, max.y), glm::min(min.z, max.z) };
				glm::ivec3 s2 = { glm::max(min.x, max.x), glm::max(min.y, max.y), glm::max(min.z, max.z) };

				for (int i = s1.x; i <= s2.x; i++)
					for (int j = s1.y; j <= s2.y; j++)
						for (int k = s1.z; k <= s2.z; k++)
						{
							if (WindowTools::Get().selected == SelectedOrder::DIG_SQUARE || WindowTools::Get().selected == SelectedOrder::MARK_AS_ROOM ||
								(WindowTools::Get().selected == SelectedOrder::DIG_CIRCLE && glm::distance(glm::vec3(i, j, k), glm::vec3(s1 + s2) / 2.f) <= glm::distance(glm::vec3(s1), glm::vec3(s2)) / 2.f))
							{
								auto tex_tuple = TextureManager::Get().GetTexture("selection_y");
								auto tex4 = std::get<1>(tex_tuple);
								glm::vec2 uv1 = { tex4.x, tex4.y };
								glm::vec2 uv2 = { tex4.z, tex4.w };

								Game::GetSpriteBatch()->AddCube(glm::vec3(i, j, k + 1.05), glm::vec3(i + 1, j + 1, k + 1.05), uv1, uv2, std::get<0>(tex_tuple), glm::vec4(0, 1, 0, 1));
							}
						}
			}
		}

		if (WindowTools::Get().selected == SelectedOrder::NONE && ImGui::IsMouseDown(0)) {
			if (auto b = mWorld->GetBlock(std::get<0>(selection_pos)))
			{
				opened_w[std::get<0>(selection_pos)] = b;
			}
		}

		float sel_x = std::get<0>(selection_pos).x;
		float sel_y = std::get<0>(selection_pos).y;
		float sel_z = std::get<0>(selection_pos).z;

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf((const GLfloat*)&Game::GetCamera()->GetProject());
		glMatrixMode(GL_MODELVIEW);
		glm::mat4 MV = Game::GetCamera()->GetView();
		glLoadMatrixf((const GLfloat*)&MV[0][0]);
		glUseProgram(0);
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(sel_x - 32, sel_y, sel_z);
		glVertex3f(sel_x + 32, sel_y, sel_z);
		glVertex3f(sel_x - 32, sel_y + 1, sel_z);
		glVertex3f(sel_x + 32, sel_y + 1, sel_z);
		for (int i = -32; i < 32; i++)
		{
			glVertex3f(sel_x + i, sel_y, sel_z);
			glVertex3f(sel_x + i, sel_y + 1, sel_z);
		}

		glColor3f(0, 1, 0);
		glVertex3f(sel_x + 1, sel_y - 32, sel_z);
		glVertex3f(sel_x + 1, sel_y + 32, sel_z);
		glVertex3f(sel_x + 1, sel_y - 32, sel_z + 1);
		glVertex3f(sel_x + 1, sel_y + 32, sel_z + 1);
		for (int i = -32; i < 32; i++)
		{
			glVertex3f(sel_x + 1, sel_y + i, sel_z);
			glVertex3f(sel_x + 1, sel_y + i, sel_z + 1);
		}

		glColor3f(0, 0, 1);
		glVertex3f(sel_x, sel_y + 1, sel_z - 32);
		glVertex3f(sel_x, sel_y + 1, sel_z + 32);
		glVertex3f(sel_x + 1, sel_y + 1, sel_z - 32);
		glVertex3f(sel_x + 1, sel_y + 1, sel_z + 32);
		for (int i = -32; i < 32; i++)
		{
			glVertex3f(sel_x, sel_y + 1, sel_z + i);
			glVertex3f(sel_x + 1, sel_y + 1, sel_z + i);
		}

		glColor3f(1, 1, 1);
		glVertex3f(sel_x, sel_y, sel_z);
		glVertex3f(sel_x + 1, sel_y, sel_z);

		glVertex3f(sel_x, sel_y, sel_z);
		glVertex3f(sel_x, sel_y + 1, sel_z);

		glVertex3f(sel_x, sel_y, sel_z);
		glVertex3f(sel_x, sel_y, sel_z + 1);

		glVertex3f(sel_x + 1, sel_y + 1, sel_z + 1);
		glVertex3f(sel_x, sel_y + 1, sel_z + 1);

		glVertex3f(sel_x + 1, sel_y + 1, sel_z + 1);
		glVertex3f(sel_x + 1, sel_y, sel_z + 1);

		glVertex3f(sel_x + 1, sel_y + 1, sel_z + 1);
		glVertex3f(sel_x + 1, sel_y + 1, sel_z);

		glVertex3f(sel_x + 1, sel_y, sel_z + 1);
		glVertex3f(sel_x + 1, sel_y, sel_z);

		glVertex3f(sel_x + 1, sel_y, sel_z);
		glVertex3f(sel_x + 1, sel_y + 1, sel_z);

		glVertex3f(sel_x, sel_y, sel_z + 1);
		glVertex3f(sel_x, sel_y + 1, sel_z + 1);

		glVertex3f(sel_x, sel_y + 1, sel_z + 1);
		glVertex3f(sel_x, sel_y + 1, sel_z);

		glEnd();
	}

	Game::GetSpriteBatch()->SetCam(Game::GetCamera());
	Game::GetSpriteBatch()->Render();

	ImGui_ImplGlfwGL3_NewFrame();
	{
		auto b = mWorld->GetBlock(static_cast<WBPos>(std::get<0>(selection_pos)));
		if (b && !Settings::Get().pause)
		{
			//ImGui::SetNextWindowPos({ Game::GetWindow()->GetSize().x / 2.f - 70, 0 });
			//ImGui::SetNextWindowSize({ 140,50 });

			//ImGui::Begin("Selected");
			//b->DrawGui({}, dt);
			//ImGui::Text("%s", b->GetDescription().c_str());
			//ImGui::End();
		}

		ImGui::Begin("Colony", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		int i = 0;
		for (auto &c : mWorld->creatures)
		{
			if (ImGui::TreeNode((std::string("creature_") + std::to_string(i)).c_str()))
			{
				c.second->DrawGui({}, dt);
				ImGui::TreePop();
			}
			i++;
		}
		ImGui::End();

	}

	WS::Get().Draw(Game::GetWindow()->GetSize(), dt);

	//ImGui::Begin("Colony", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	//int i = 0;
	//for (auto &c : mWorld->controlled)
	//{
	//	if (ImGui::TreeNode((std::string("creature_") + std::to_string(i)).c_str()))
	//	{
	//		c->DrawGui(dt);
	//		ImGui::TreePop();
	//	}
	//	i++;
	//}
	//ImGui::End();

	for (auto &w : opened_w)
	{
		ImGui::Begin((boost::format("Block UI (%1%, %2%, %3%)") % std::get<0>(w).x % std::get<0>(w).y % std::get<0>(w).z).str().c_str(),
			nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

		GameObjectParams gop;
		gop.world = mWorld.get();
		gop.pos = std::get<0>(w);
		std::get<1>(w)->DrawGui(gop, dt);

		glm::vec2 pos_win = ImGui::GetWindowPos();
		glm::vec2 size = ImGui::GetWindowSize();
		auto pos_block = Game::GetCamera()->Project(static_cast<glm::vec3>(std::get<0>(w)) + glm::vec3(0.5f,0.5f,1));

		glm::vec2 poses[] = { { pos_win.x + size.x + 3, pos_win.y + size.y / 2.f }, // right
			{ pos_win.x - 3, pos_win.y + size.y / 2.f }, // left
			{ pos_win.x + size.x / 2.f, pos_win.y + size.y + 3 }, // bottom
			{ pos_win.x + size.x / 2.f, pos_win.y - 3 } }; // top 

		float min = glm::distance(poses[0], pos_block);
		int di = 0;
		for (size_t i = 1; i < 4; i++)
		{
			float d = glm::distance(poses[i], pos_block);
			if (d < min)
			{
				min = d;
				di = i;
			}
		}

		Color c = mWorld->GetSlise() == std::get<0>(w).z ? Color::White : Color::Gray;

		auto& dd = ImGui::GetCurrentContext()->OverlayDrawList;
		dd.AddLine(poses[di], pos_block, c);
		switch (di)
		{
		case 0:
			dd.AddLine({pos_win.x + size.x + 3, pos_win.y}, { pos_win.x + size.x + 3, pos_win.y + size.y }, c);
			break;
		case 1:
			dd.AddLine({ pos_win.x - 3, pos_win.y }, { pos_win.x - 3, pos_win.y + size.y }, c);
			break;
		case 2:
			dd.AddLine({ pos_win.x, pos_win.y + size.y + 3 }, { pos_win.x + size.x, pos_win.y + size.y + 3 }, c);
			break;
		case 3:
			dd.AddLine({ pos_win.x, pos_win.y - 3 }, { pos_win.x + size.x, pos_win.y - 3 }, c);
			break;
		}

		ImGui::End();

	}

	if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		if (WindowEsc::Get().GetVisibility())
			WindowEsc::Get().Toggle();
		else
			if (WindowTools::Get().selected != SelectedOrder::NONE)
				WindowTools::Get().selected = SelectedOrder::NONE;
			else
				if (!opened_w.empty())
					opened_w.clear();
				else
					WindowEsc::Get().Toggle();
	}

	if (!ImGui::IsPosHoveringAnyWindow(ImGui::GetMousePos()))
	{
		int slise = mWorld->GetSlise();
		float prewheel = ImGui::GetIO().MouseWheel;
		mWorld->SetSlise(static_cast<int>(slise + glm::sign(prewheel)));
	}

	ImGui::Render();

	WindowPerfomance::Get().DtUpdate(dt, Game::GetFps()->GetCount(), mRenderSector->ApproxDc(), mWorld->GetActiveCount());
}

void GamePhase_Game::Update(float dt)
{
	if (Settings::Get().pause)
		return;

	SPos secPos = { 0,0,0 };
	mSectorLoader->SetPos(Game::GetCamera()->GetPos() / float(SECTOR_SIZE));


	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseDragging(1))
	{
		auto del = glm::vec3(ImGui::GetMouseDragDelta(1).y / Game::GetWindow()->GetSize().y, ImGui::GetMouseDragDelta(1).x / Game::GetWindow()->GetSize().x, 0);
		del = Game::GetCamera()->GetDirection() * del * 4.f;
		del.z = 0;
		Game::GetCamera()->SetPos(Game::GetCamera()->GetPos() - del);
	}

	auto map_ready = EventBus::Get().ListenChannel<EventMapReady>();
	if(!map_ready.empty())
	{
		MapTemplates::Get().templates[StringIntern("spawn_prepare")]->ForceApplyToWorld(mWorld.get(), { -2,-2, 0 });

		auto c = DB::Get().Create("caracter");
		c->GetAgent<Chest>()->Push(DB::Get().Create("nutrition"), 100);
		c->GetAgent<Chest>()->Push(DB::Get().Create("nail_material_iron"), 200);
		c->GetAgent<Chest>()->Push(DB::Get().Create("stick"), 50);
		c->GetAgent<Chest>()->Push(DB::Get().Create("stockpile"), 5);
		c->GetAgent<Chest>()->Push(DB::Get().Create("water_boiler_iron"), 2);

		mWorld->Spawn({ 0, 0, 0 }, c);
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
		mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter"));
	}

	mWorld->Update(static_cast<float>(dt));
	Currency::Get().Update(dt);
	OrderBus::Get().Update(dt);
	EventBus::Get().Update();
}

bool GamePhase_Game::IsInitialized()
{
	return initialized;
}

GamePhase_MainMenu::GamePhase_MainMenu()
{
}

GamePhase_MainMenu::~GamePhase_MainMenu()
{
}

void GamePhase_MainMenu::Draw(float gt)
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static std::unique_ptr<GamePhase> game;

	ImGui_ImplGlfwGL3_NewFrame();
	{
		ImGui::SetNextWindowSize({ Game::GetWindow()->GetSize().x / 2.f, Game::GetWindow()->GetSize().y / 2.f });
		ImGui::SetNextWindowPosCenter();

		ImGui::Begin("Main menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
		if (ImGui::Button("Quick start"))
		{
			auto date_time = boost::posix_time::microsec_clock::universal_time();
			std::stringstream sstream;
			sstream << date_time;
			std::string fname = sstream.str();
			std::replace(fname.begin(), fname.end(), '-', '_');
			std::replace(fname.begin(), fname.end(), '.', '_');
			std::replace(fname.begin(), fname.end(), ':', '_');
			std::replace(fname.begin(), fname.end(), ' ', '_');
			Settings::Get().save_file = "Save\\" + fname + ".world";

			static boost::thread game_load([&]()
			{
				game = std::make_unique<GamePhase_Game>();
			});
		}
		if (ImGui::Button("Load game"))
		{
			Game::SetGamePhase(std::make_unique<GamePhase_LoadMenu>());
		}

		if (game && game->IsInitialized())
		{
			static_cast<GamePhase_Game*>(game.get())->generateShadowFBO();
			TextureManager::Get().Compile();
			Game::SetGamePhase(std::move(game));
		}

		Game::DrawLoading();
		ImGui::End();
	}
	ImGui::Render();
}

void GamePhase_MainMenu::Update(float gt)
{
}

Game::Game()
{
	Window::WindowSystemInitialize();

	mWindow = std::make_unique<Window>(glm::uvec2(1024, 768));
	mWindow->SetCurrentContext();

	Render::Initialize();
	mRender = std::make_unique<Render>();

	ImGui_ImplGlfwGL3_Init(mWindow->Get(), true);
	//ImGui::SetupImGuiStyle(true, 0.8f);

	Localize::instance().Init();

	Initialized = true;

	//GL_CALL(glViewport(0, 0, REGISTRY_GRAPHIC.GetWindow().GetSize().x, REGISTRY_GRAPHIC.GetWindow().GetSize().y)); 
}

Game::~Game()
{
	mRender.reset();
	mWindow.reset();
	Window::WindowSystemFinally();
}


void Game::GameRun()
{
	auto currTime = static_cast<float>(glfwGetTime());
	while (!mWindow->WindowShouldClose())
	{
		fps.Update();

		auto lastTime = currTime;
		currTime = static_cast<float>(glfwGetTime());

		if (game_phase)
		{
			game_phase->Update(currTime - lastTime);

			currTime = static_cast<float>(glfwGetTime());

			Settings::Get().pause = false;
			game_phase->Draw(currTime - lastTime);
		}

		mWindow->Update();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1)); ?!
	}
}

int Game::Run()
{
	if (!Initialized)
	{
		system("pause");
		return -1;
	}

	GetSun()->SetShadow();

	GetCamera()->Resize(mWindow->GetSize());
	mWindow->SetResizeCallback([&](int x, int y) {
		if (y < 1)
			y = 1;
		if (x < 1)
			x = 1;
		GetCamera()->Resize({ x, y });
		GetSun()->Resize({ x, y });
	});

	TextureManager::Get().LoadDirectory("data\\textures\\");
	//TextureManager::Get().Compile();

	sb = std::make_unique<SpriteBatch>();

	game_phase = std::make_unique<GamePhase_MainMenu>();
	GameRun();

	return 0;
}

Window* Game::GetWindow()
{
	return mWindow.get();
}

Render* Game::GetRender()
{
	return mRender.get();
}

SpriteBatch* Game::GetSpriteBatch()
{
	return sb.get();
}

FpsCounter* Game::GetFps()
{
	return &fps;
}

Camera* Game::GetCamera()
{
	return Settings::Get().GetCamera();
}

Camera* Game::GetSun()
{
	return Settings::Get().GetSun();
}

WorldWorker* Game::GetWorker()
{
	return world_worker.get();
}

void Game::SetWorker(std::unique_ptr<WorldWorker> &&ww)
{
	world_worker = std::move(ww);
}

void Game::SetGamePhase(std::unique_ptr<GamePhase>&& gp)
{
	game_phase = std::move(gp);
}

void Game::SetLoadPhase(float per, std::string des, int ph, int max_ph)
{
	percent = per;
	descr = des;
	phase = ph;
	max_phase = max_ph;
}

void Game::SetLoadPhase(float per, std::string des, int ph)
{
	percent = per;
	descr = des;
	phase = ph;
}

void Game::SetLoadPhase(float per, std::string des)
{
	percent = per;
	descr = des;
}

void Game::SetLoadPhase(float per)
{
	percent = per;
}

void Game::DrawLoading()
{
	if (!descr.empty())
	{
		if (max_phase != 0)
		{
			ImGui::Text("stage %d from %d", phase, max_phase);
		}
		ImGui::ProgressBar(percent);
		ImGui::Text("%s", descr.c_str());
	}
}

std::unique_ptr<Window> Game::mWindow;
std::unique_ptr<Render> Game::mRender;
std::unique_ptr<SpriteBatch> Game::sb;
FpsCounter Game::fps;
std::unique_ptr<GamePhase> Game::game_phase;
std::unique_ptr<WorldWorker> Game::world_worker;
float Game::percent;
std::string Game::descr;
int Game::phase;
int Game::max_phase;

