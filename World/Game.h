﻿// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#ifndef GAME_H
#define GAME_H

#include <memory>
#include "Window\Window.h"
#include "Render\Camera.h"
#include "Render\Render.h"
#include "Core\World.h"
#include "Core\SectorLoader.h"


class Game
{
public:
  Game();
  ~Game();

  int Run();

  void Initialize();

  void Update(double dt);

  void Draw(double dt);

private:

  bool Initialized = false;

  std::unique_ptr<Window> mWindow;
  std::unique_ptr<Render> mRender;
  std::shared_ptr<Camera> mCamera;
  std::unique_ptr<World> mWorld;

  std::unique_ptr<SectorLoader> mSectorLoader;
};

#endif // GAME_H
