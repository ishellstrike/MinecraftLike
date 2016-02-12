// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef ModelBlockGenerator_h__
#define ModelBlockGenerator_h__

#include <vector>
#include "..\Render\Mesh.h"
#include "..\Render\Vertex.h"


/// ��������� ������ ��� �����.
/// ��� ����������� �������������, ��� ������ ������� ����� ����� ������� ��������.
/// ��� �������� ������ ���������� � ����� ���������� ������.
/// ��� ������ ������� ����� ����� �������, ����� �� �������� �������. 
class MeshBlockGenerator
{
public:
  using VertexType = VertexVTN;

  enum Side
  {
    EMPTY = 0,

    FRONT = 1 << 0,
    RIGHT = 1 << 1,
    BACK = 1 << 2,
    LEFT = 1 << 3,
    TOP = 1 << 4,
    BOTTOM = 1 << 5,

    ALL = 0x3F,
  };

public:
  MeshBlockGenerator();

  void Create(Mesh<VertexType> &mesh, Side side);

  void SetTexture(int side, std::string texture);

  void Enable(int side, bool enabled);

  void Generate();

  inline const PMesh<VertexType> &Create(Side side) const
  {
    return mMeshArray[side];
  }

private:
  /// ������ ��������� ������� ��� ������ ���� � ������.
  std::vector<glm::vec4> mTextures;

  std::vector<PMesh<VertexType>> mMeshArray;
};



#endif // ModelBlockGenerator_h__