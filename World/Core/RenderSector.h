// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef RenderSector_h__
#define RenderSector_h__

#include <atomic>
#include "..\Render\Render.h"
#include <glm/glm.hpp>
#include "..\tools\CoordSystem.h"


/// ������ ������.
/// ����� ������: ���� ���������� ����, ��������� �� ���� ������, ����������� ������ ������,
/// � ������ ������ ����������� ������, ������� ��� � ���� �����.
/// ����� �������: ����� ����������� � ������.
class RenderSector
{
public:
  RenderSector(const SPos &pos);
  ~RenderSector();

  /// ��������, ��� ��������� ����������.
  void Changed();

  /// ����� �� ����������� ���������?
  bool IsNeedBuild() const;

  void Push(const Model &model, const glm::vec3 &pos);

  void Update();

  void Draw(Render &render);

private:
  Model mModel;
  glm::mat4 mModelMatrix;

  bool mIsChanged = true;
  bool mIsNeedBuild = false;

  std::atomic<bool> mRebuildBuffers = false;

};



#endif // RenderSector_h__