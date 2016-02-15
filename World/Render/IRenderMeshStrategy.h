// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef IRenderMesh_h__
#define IRenderMesh_h__

#include "Vertex.h"
#include "..\Render\Shader.h"

class IRenderMeshStrategy
{
public:
  virtual ~IRenderMeshStrategy() {};

  /// ��������� � �������� �������.
  /// ��� ������� ���� �������� ���������� ������������� �������, 
  /// �� ������� ������ ����� ����� ����� ������.
  /// Attribute::size ������ �������� � ������.
  /// Attribute::offset �������� �������� � ������.
  virtual void SetAttribute(const std::vector<Attribute> &attribute, const std::vector<int> &locations) = 0;

  /// ������� ����� � �����������.
  /// @param vertex ��������� �� ������ ������ ������.
  /// @param vertexCount ���������� ��������� � ������ ������.
  /// @param index ��������� �� ������ ������ ��������.
  /// @param indexCount ���������� ��������� � ������ ��������.
  virtual void Compile(const float *vertex, size_t vertexCount, const size_t *index, size_t indexCount) = 0;

  /// ���������� �����.
  virtual void Draw() const = 0;

};


#endif // IRenderMesh_h__