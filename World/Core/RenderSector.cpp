// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#include "RenderSector.h"
#include "Sector.h"
#include <GLFW\glfw3.h>
#include <type_traits>
#include "..\tools\Log.h"
#include <glm\gtc\matrix_transform.hpp>



RenderSector::RenderSector(const SPos &pos)
{
  using MeshType = std::remove_reference_t<decltype(mModel.GetMesh())>::element_type;
  mModel.GetMesh() = std::make_shared<MeshType>();
  mModel.GetMesh()->Reserve(100000, 100000);

  mModelMatrix = glm::translate(mModelMatrix, cs::StoW(pos));
}



RenderSector::~RenderSector()
{
}

void RenderSector::Changed()
{
  mIsChanged = true;
}

bool RenderSector::IsNeedBuild() const
{
  return mIsNeedBuild;
}

void RenderSector::Push(const Model &model, const glm::vec3 &pos)
{
  // ���� ������ ���� ����������� � ������ ����������� - ������������� ������.
  if (mIsNeedBuild && model.GetType() == Model::Static)
  {
    auto &dst = mModel.GetMesh();
    const auto &src = model.GetMesh();
    if (dst->Empty())
    {
      mModel.SetTexture(model.GetTexture());
    }
    if (mModel.GetTexture() == model.GetTexture())
    {
      size_t size = dst->SizeVertex();
      dst->Push(*src);
      for (size_t i = size; i < dst->SizeVertex(); ++i)
      {
        dst->Vertex(i).vertex += pos;
      }
    }
    else
    {
      LOG(warning) << "������� ���� � ������� ��������. �������� �� ���������.";
    }
  }
}

void RenderSector::Update(Render &render)
{
  // ������ ������ ��� �������������. � ����� ������� �� ��� ����������.
  if (mIsNeedBuild)
  {
    render.Draw(mModel, mModelMatrix);
    mIsNeedBuild = false;
    mRebuildBuffers = true;
  }

  // ������ ��� �������, ����� ��� �����������.
  if (mIsChanged)
  {
    // ���� ����� �������������, �� ������� ������.
    if (!mRebuildBuffers)
    {
      mModel.GetMesh()->Reserve(24 * SECTOR_SIZE * SECTOR_SIZE * SECTOR_SIZE,
        36 * SECTOR_SIZE * SECTOR_SIZE * SECTOR_SIZE);

      mIsNeedBuild = true;
      mIsChanged = false;
    }
  }
}

void RenderSector::Draw(Render &render)
{
  // ������ ������.
  // ���� ������ ��� �������, ������ ����, ��� ������ ������ ���� ����������.
  // ���� ���� � ����������� ���������� - ������������� ������.
  
//   if (mRebuildBuffers)
//   {
//     auto currentTime = glfwGetTime();
//     mModel.GetMesh()->GetStrategy().UseShader(render.GetShader());
//     mModel.GetMesh()->Compile();
//     mModel.GetMesh()->Release();
// 
//     LOG(trace) << "ListGen: " << glfwGetTime() - currentTime;
//     mRebuildBuffers = false;
//   }

  //render.Draw(mModel, mModelMatrix);
}
