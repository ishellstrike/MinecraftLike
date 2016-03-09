﻿// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#include "Game.h"

#include <gl/glew.h>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/thread/thread.hpp>
#include <boost\circular_buffer.hpp>

#include <fstream>
#include <memory>
#include "Render/OpenGLCall.h"
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
#include "gui\WindowPerfomance.h"
#include "gui\WindowDb.h"
#include "gui\WindowInventory.h"
#include <Render\Resourses.h>

Game::Game()
{
  Window::WindowSystemInitialize();

  mWindow = std::make_unique<Window>(glm::uvec2(600, 600));
  mCamera = std::make_shared<Camera>();
  mWindow->SetCurrentContext();

  mSun = std::make_shared<Camera>();
  mSun->SetShadow();
  mSun->SetPos({0,0,100});
  mSun->LookAt({10,10,-100});
  depthTextureId = std::make_shared<Texture>();
  depthTextureId->DepthTexture({ 600, 600 });

  Render::Initialize();
  mRender = std::make_unique<Render>();
  mRenderSector = std::make_unique<RenderSector>();

  ImGui_ImplGlfwGL3_Init(mWindow->Get(), true);

  Initialized = true;

  //GL_CALL(glViewport(0, 0, REGISTRY_GRAPHIC.GetWindow().GetSize().x, REGISTRY_GRAPHIC.GetWindow().GetSize().y)); 

  mWorld = std::make_unique<World>();
  WindowInventory::Get().w = mWorld.get();
  mKeyBinder = std::make_unique<KeyBinder>(mWindow->GetKeyboard(), mWindow->GetMouse());

  mKeyBinder->SetCallback([world=mWorld.get()](std::unique_ptr<GameEvent> event)
  {
    world->PushEvent(std::move(event));
  });

  generateShadowFBO();
}

Game::~Game()
{
  mKeyBinder.reset();
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
  mWindow->SetResizeCallback([&](int x, int y) {
    if (y == 0)
      y = 1;
    mCamera->Resize({x,y});
    int width, height;
    glfwGetFramebufferSize(mWindow->Get(), &width, &height);
    glViewport(0, 0, width, height);
  });

  TextureManager::Get().LoadDirectory("data\\textures\\");
  TextureManager::Get().Compile();

  DB::Get().ReloadDirectory("data\\json\\");

  mWorld->GetPlayer()->SetPosition({ 0,0,50 });

  boost::thread gen_thread([]() {
    while (true)
    {
      WorldWorker::Get().Process();
      boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
    }
  });

  mSectorLoader = std::make_unique<SectorLoader>(*mWorld, SPos{}, 7);

  mTessellator = std::make_unique<Tessellator>(*mRenderSector);
  mWorld->SetTessellator(mTessellator.get());
  mTessellator->Run();

//   boost::thread update_thread([this]
//   {
//     auto currTime = static_cast<float>(glfwGetTime());
//     while (true)
//     {
//       auto lastTime = currTime;
//       currTime = static_cast<float>(glfwGetTime());
//       Update(currTime - lastTime);
//       boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
//     }
//   });

  mRender->AddModel("data/models/selection.obj", "dirt", "shaders/basic.glsl");
  
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

  //update_thread.interrupt();
  gen_thread.interrupt();

  //LOG(trace) << "update joining";
  //update_thread.join();

  LOG(trace) << "generate joining";
  gen_thread.join();

  LOG(trace) << "quit";

  return 0;
}


void Game::Update(float dt)
{
  mKeyBinder->Update();

  if (mWindow->GetKeyboard().IsKeyPress(GLFW_KEY_TAB))
  {
    mWindow->GetMouse().SetCentring(!mWindow->GetMouse().GetCentring());
  }

  if (mWindow->GetKeyboard().IsKeyDown(GLFW_KEY_F5))
  {
    static int i = -20;
    static int j = 0;
    i++;
    if (i > 20)
    {
      j++;
      i = -20;
    }
    mWorld->GetSector(glm::vec3(i, j, 0));
  }

  SPos secPos = cs::WtoS(mWorld->GetPlayer()->GetPosition());
  mSectorLoader->SetPos(secPos);

  mWorld->Update(static_cast<float>(dt));
}

void Game::generateShadowFBO()
{
  int shadowMapWidth = mWindow->GetSize().x;
  int shadowMapHeight = mWindow->GetSize().y;

  GLenum FBOstatus;

  glGenFramebuffers(1, &fboId);
  glBindFramebuffer(GL_FRAMEBUFFER, fboId);

  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId->GetId(), 0);

  FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (FBOstatus != GL_FRAMEBUFFER_COMPLETE)
    LOG(fatal) << "framebuffer incomplete";

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Game::Draw(float dt)
{
  mCamera->SetPos(mWorld->GetPlayer()->GetPosition() + glm::vec3{ 0.0f, 0.0f, 1.7f });
  mCamera->SetRot(mWorld->GetPlayer()->GetRot());
  mCamera->Update();

  mSun->LookAt(mCamera->GetPos());
  mSun->Update();

  
  
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboId));
  GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
  GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
  GL_CALL(glCullFace(GL_FRONT));
  mRenderSector->ShadowDraw(*mSun, Resourses::Get().GetShader("shaders/shadow.glsl"));

  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
  GL_CALL(glCullFace(GL_BACK));
  GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  depthTextureId->Set(TEXTURE_SLOT_1);
  mRenderSector->Draw(*mCamera, *mSun);
  mRender->Draw(*mCamera);

  WindowPerfomance &wp = WindowPerfomance::Get();
  WindowInventory &winv = WindowInventory::Get();
  WindowDb &wdb = WindowDb::Get();

  ImGui_ImplGlfwGL3_NewFrame();
  wp.DtUpdate(dt, fps.GetCount());
  wp.Draw();
  wdb.Draw();
  winv.Draw();
  ImGui::Begin("shadow");
  auto s = ImGui::GetWindowSize();
  ImGui::Image((void*)depthTextureId->GetId(), s);
  ImGui::End();
  ImGui::Render();
}

