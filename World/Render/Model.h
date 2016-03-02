// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef Model_h__
#define Model_h__

#include "Texture.h"
#include "TemplateMesh.h"
#include "Vertex.h"
#include <rapidjson\document.h>
#include <tools\StringIntern.h>

using PModel = std::shared_ptr<class Model>;

/// ������. ����� ��� ���������� ��� ���������.
class Model
{
public:
  using VertexType = VertexVTN;

  Model();

  void JsonLoad(const rapidjson::Value & val);

  enum Type
  {
    Static,
  };

  inline void SetTexture(PTexture texture)
  {
    mTexture = texture;
  }

  inline PTemplateMesh<VertexType> &GetMesh() noexcept
  {
    return mMesh;
  }

  inline const PTemplateMesh<VertexType> &GetMesh() const noexcept
  {
    return mMesh;
  }

  inline const PTexture &GetTexture() const noexcept
  {
    return mTexture;
  }

  inline Type GetType() const noexcept
  {
    return mType;
  }

  PShader &GetShader()
  {
    return mShader;
  }

  StringIntern GetSpriteName()
  {
    return mSprite;
  }

  void SetSprite(const StringIntern &s);

  void BuildAABB(glm::vec3 VertexType::* p)
  {
    glm::vec3 _min{}, _max{};
    if (!mMesh->Empty())
    {
      for (decltype(mMesh->SizeVertex()) i = 0; i < mMesh->SizeVertex(); ++i)
      {
        glm::vec3 t = mMesh->Vertex(i).*p;
        for (int j = 0; j < 3; ++j)
        {
          _max[j] = glm::max(t[j], _max[j]);
          _min[j] = glm::min(t[j], _min[j]);
        }
      }
    }
    min = glm::vec4(_min, 1);
    max = glm::vec4(_max, 1);
  }

  const std::tuple<const glm::vec4 &, const glm::vec4 &> GetAABB() const
  {
    return std::tie(min, max);
  }

private:
  glm::vec4 min, max;
  PTemplateMesh<VertexType> mMesh;
  PTexture mTexture;
  PShader mShader;

  StringIntern mSprite; //������������ ��� �������������� ����� � �������

  Type mType = Static;
};

#endif // Model_h__

