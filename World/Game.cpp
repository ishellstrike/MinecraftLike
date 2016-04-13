﻿#include "Game.h"

#include <gl/glew.h>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/thread/thread.hpp>
#include <boost\circular_buffer.hpp>

#include <fstream>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include "tools/Bresenham3D.h"
#include "tools/CoordSystem.h"
#include "Core\MapGen\WorldWorker.h"
#include "Render/TextureManager.h"
#include "Core/DB.h"
#include "Core/SplitBlockTessellator.h"
#include "tools/Log.h"
#include "tools\wset.h"
#include "tools\order_casters.h"

#include "gui\imgui_impl_glfw_gl3.h"
#include "gui\WS.h"
#include <gui\WindowInventory.h>
#include <gui\WindowPerfomance.h>
#include <Render\Resources.h>
#include <tools\ray.h>
#include <core\Chest.h>
#include <core\Tool.h>
#include <Render\ParticleSystem.h>
#include <core\EventBus.h>
#include <core\OrderBus.h>

Game::Game()
{
	Window::WindowSystemInitialize();

	mWindow = std::make_unique<Window>(glm::uvec2(1024, 768));
	mCamera = std::make_shared<Camera>();
	mWindow->SetCurrentContext();

	mSun = std::make_shared<Camera>();
	mSun->SetShadow();

	Render::Initialize();
	mRender = std::make_unique<Render>();
	mRenderSector = std::make_unique<RenderSector>();

	ImGui_ImplGlfwGL3_Init(mWindow->Get(), true);

	Initialized = true;

	//GL_CALL(glViewport(0, 0, REGISTRY_GRAPHIC.GetWindow().GetSize().x, REGISTRY_GRAPHIC.GetWindow().GetSize().y)); 

	mWorld = std::make_unique<World>();
	WindowInventory::Get().w = mWorld.get();

	generateShadowFBO();
}

Game::~Game()
{
	mWorld.reset();
	mRenderSector.reset();
	mRender.reset();
	mWindow.reset();
	Window::WindowSystemFinally();
}


int Game::Run()
{
	if (!Initialized)
	{
		system("pause");
		return -1;
	}

	mCamera->Resize(mWindow->GetSize());
	mCamera->SetPos({ 0, 0, 64 });
	mCamera->LookAt({ 32, 32, 0 });
	mWindow->SetResizeCallback([&](int x, int y) {
		mCamera->Resize({ x, y });
		mSun->Resize({ x, y });
	});

	TextureManager::Get().LoadDirectory("data\\textures\\");
	TextureManager::Get().Compile();

	DB::Get().ReloadDirectory("data\\json\\");

	boost::thread gen_thread([]() {
		while (true)
		{
			WorldWorker::Get().Process();
			boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
		}
	});

	mSectorLoader = std::make_unique<SectorLoader>(*mWorld, SPos{}, 5);

	mTessellator = std::make_unique<Tessellator>(*mRenderSector);
	mTessellator->SayCamera(mCamera);
	mWorld->SetTessellator(mTessellator.get());
	mTessellator->Run();

	auto currTime = static_cast<float>(glfwGetTime());
	while (!mWindow->WindowShouldClose())
	{
		fps.Update();

		auto lastTime = currTime;
		currTime = static_cast<float>(glfwGetTime());

		Update(currTime - lastTime);
		Draw(currTime - lastTime);

		mWindow->Update();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1)); ?!
	}

	mTessellator.reset();

	gen_thread.interrupt();

	LOG(trace) << "generate joining";
	gen_thread.join();

	LOG(trace) << "quit";

	return 0;
}


void Game::Update(float dt)
{
	SPos secPos = { 0,0,0 };
	mSectorLoader->SetPos(secPos);

	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseDragging(1))
	{
		auto del = glm::vec3(ImGui::GetMouseDragDelta(1).y / mWindow->GetSize().y, ImGui::GetMouseDragDelta(1).x / mWindow->GetSize().x, 0);
		del = mCamera->GetDirection() * del;
		del.z = 0;
		mCamera->SetPos(mCamera->GetPos() - del);
	}

	for (const auto &e : EventBus::Get().ListenChannel<EventSectorReady>())
	{
		const auto &esec = std::static_pointer_cast<EventSectorReady>(e);
		if (esec->sec->GetPos() == SPos(0, 0, 0))
		{
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
			controlled.push_back(mWorld->Spawn({ 0, 0, 0 }, DB::Get().Create("caracter")));
		}
	}

	mWorld->Update(static_cast<float>(dt));
	OrderBus::Get().Update();
	EventBus::Get().Update();
}

void Game::generateShadowFBO()
{
	depthTextureId = std::make_shared<Texture>();
	depthTextureId->DepthTexture({ 1024, 1024 });

	GLenum FBOstatus;

	if (fboId)
		glDeleteFramebuffers(1, &fboId);

	fboId = -1;

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId->GetId(), 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Game::Draw(float dt)
{
	float drawtime = glfwGetTime();
	//mCamera->SetPos(mWorld->GetPlayer()->GetPosition() + glm::vec3{ 0.0f, 0.0f, 1.7f });
	//mCamera->SetRot(mWorld->GetPlayer()->GetRot());
	mCamera->Update();

	static float phi = 0;
	//phi += dt / 20.f;
	mSun->SetPos(mCamera->GetPos() + glm::vec3{ glm::sin(phi) + glm::cos(phi), 0, -glm::sin(phi) + glm::cos(phi) });
	mSun->LookAt(mCamera->GetPos());
	mSun->Update();

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
	mRenderSector->ShadowDraw(*mSun, Resources::Get().GetShader("shaders/shadow.glsl"));
	skip = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, mWindow->GetFbSize().x, mWindow->GetFbSize().y);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glCullFace(GL_BACK);
	auto col = glm::mix(glm::vec4(117.0f / 255.0f, 187.0f / 255.0f, 253.0f / 255.0f, 1.0f), glm::vec4(0, 0.1, 0.2, 1), (glm::sin(phi) - glm::cos(phi) + 1) / 2.f);
	glClearColor(col.x, col.y, col.z, col.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	depthTextureId->Set(TEXTURE_SLOT_2);
	mRenderSector->Draw(*mCamera, *mSun);

	mRender->Draw(*mCamera);

	ParticleSystem::Get().Draw(*mCamera);
	ParticleSystem::Get().Update(dt);

	auto ray = mCamera->GetRay(ImGui::GetMousePos());
	std::tuple<glm::ivec3, glm::vec3> selection_pos; // pos, normal

	static size_t select_model = mRender->AddModel("data/models/selection.obj", "selection", "shaders/basic.glsl");
	static size_t square_model = mRender->AddModel("data/models/selection.obj", "selection", "shaders/basic.glsl");

	static std::vector<size_t> order_models = [&]() -> std::vector<size_t>
	{
		std::vector<size_t> t;
		for (int i = 0; i < 1000; i++)
		{
			t.push_back(mRender->AddModel("data/models/selection.obj", "selection", "shaders/basic.glsl"));
		}
		return t;
	}();


	int j = 0;
	auto ord = OrderBus::Get().orders.begin();
	for (auto i : order_models)
	{
		if (OrderBus::Get().orders.size() > j && (*ord)->GetId() == Order::Idfor<OrderDig>())
		{
			auto od = std::static_pointer_cast<OrderDig>(*ord);
			mRender->SetModelMatrix(i, glm::translate(glm::mat4(1), od->pos));
			ord++;
		}
		else
		{
			mRender->SetModelMatrix(i, glm::translate(glm::mat4(1), glm::vec3{ 99999 }));
		}

		j++;
	}

	static std::unordered_map<glm::ivec3, PGameObject> opened_w;

	selection_pos = PickFirst(ray.origin(), ray.dir(), 100.f, [&](const glm::ivec3 &pos)->bool {
		return mWorld->GetBlock(pos).get();
	});

	mRender->SetModelMatrix(select_model, glm::translate(glm::mat4(1), glm::vec3(std::get<0>(selection_pos))));
	if (!ImGui::IsAnyItemHovered())
	{
		static glm::vec3 min, max;
		static int minmaxstate = 0;
		if (WindowTools::Get().selected != SelectedOrder::DIG_SQUARE)
		{
			min = max = glm::vec3{ 99999 };
			minmaxstate = 0;
		}

		if (ImGui::IsMouseDown(0))
		{
			switch (WindowTools::Get().selected)
			{
			case SelectedOrder::DIG_SQUARE:
				switch (minmaxstate)
				{
				case 0:
					min = std::get<0>(selection_pos);
					minmaxstate = 1;
					break;
				}
				break;
			}
		}

		if (!ImGui::IsMouseDown(0))
		{
			if (minmaxstate == 1)
			{
				max = std::get<0>(selection_pos);

				if (min.x > max.x) std::swap(min.x, max.x);
				if (min.y > max.y) std::swap(min.y, max.y);
				if (min.z > max.z) std::swap(min.z, max.z);

				for (int i = min.x; i <= max.x; i++)
					for (int j = min.y; j <= max.y; j++)
						for (int k = min.z; k <= max.z; k++)
						{
							if(auto b = mWorld->GetBlock(glm::vec3{ i,j,k }))
							{
								OrderBus::Get().IssueOrder(std::make_shared<OrderDig>(glm::vec3{ i,j,k }));
							}
						}

				minmaxstate = 0;
			}
		}

		if (ImGui::IsMouseDown(1)) {
			if (auto b = mWorld->GetBlock(std::get<0>(selection_pos)))
			{
				b->Interact(InteractParams{ mWorld.get(), std::get<0>(selection_pos), dt });
				opened_w[std::get<0>(selection_pos)] = b;
			}
		}
	}

	ImGui_ImplGlfwGL3_NewFrame();

	WS::Get().Draw(mWindow->GetSize());

	ImGui::Begin("Colony");
	int i = 0;
	for (auto &c : controlled)
	{
		if (ImGui::TreeNode((std::string("creature_") + std::to_string(i)).c_str()))
		{
			c->DrawGui();
			ImGui::TreePop();
		}
		i++;
	}
	ImGui::End();

	for (auto &w : opened_w)
	{
		ImGui::Begin((boost::format("Block UI (%1%, %2%, %3%)") % std::get<0>(w).x % std::get<0>(w).y % std::get<0>(w).z).str().c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		std::get<1>(w)->DrawGui();
		ImGui::End();
	}

	if (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
	{
		opened_w.clear();
	}

	ImGui::Render();

	WindowPerfomance::Get().DtUpdate(glfwGetTime() - drawtime, fps.GetCount(), mRenderSector->ApproxDc(), mWorld->GetActiveCount());
}

