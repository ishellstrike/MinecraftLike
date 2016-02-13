// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef RenderSector_h__
#define RenderSector_h__

#include <atomic>
#include "..\Render\Render.h"


/// ������ ������.
/// ����� ������: ���� ���������� ����, ��������� �� ���� ������, ����������� ������ ������,
/// � ������ ������ ����������� ������, ������� ��� � ���� �����.
/// ����� �������: ����� ����������� � ������.
class RenderSector
{
public:
  RenderSector(const glm::mat3 &transform);
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

  bool mIsChanged = true;
  bool mIsNeedBuild = false;

  std::atomic<bool> mRebuildBuffers = false;
  glm::mat3 transform;
};



#endif // RenderSector_h__