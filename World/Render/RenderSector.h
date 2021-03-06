



#pragma once
#ifndef RenderSector_h__
#define RenderSector_h__

#include <unordered_map>
#include "..\tools\CoordSystem.h"
#include "Model.h"
#include <list>
#include <tuple>
#include <mutex>

// ������ ������ ������� �������� �� ������.
class RenderSector
{
public:
  void Push(const Model &model, const SPos &pos);

  void Remove(const SPos &pos);

  void Draw(class Camera &camera, Camera &light);

  void ShadowDraw(Camera & camera, PShader shader);

  int ApproxDc();

private:
  std::unordered_map<SPos, Model> mModels;
  int mDc;

  std::list<std::tuple<Model, SPos>> mPushList;
  std::list<SPos> mRemoveList;

  std::mutex mMutex;
private:
  void SyncElements();
};



#endif // RenderSector_h__