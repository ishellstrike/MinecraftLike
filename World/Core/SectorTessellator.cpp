// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#include "SectorTessellator.h"

#include <glm\gtc\matrix_transform.hpp>
#include <type_traits>
#include "Tessellator.h"
#include "TessellatorParams.h"
#include "..\Render\Render.h"

SectorTessellator::SectorTessellator(const SPos &pos)
  : mPos(pos)
{
  using MeshType = std::remove_reference_t<decltype(mModel.GetMesh())>::element_type;
  mModel.GetMesh() = std::make_shared<MeshType>();
  mModel.GetMesh()->Reserve(100000, 100000);

  mModelMatrix = glm::translate(mModelMatrix, cs::StoW(mPos));
}


SectorTessellator::~SectorTessellator()
{
}

void SectorTessellator::SetBlock(const SBPos &pos, PBlockTessellator block)
{
  mBlocks[cs::SBtoI(pos)] = block;
}

PBlockTessellator SectorTessellator::GetBlock(const SBPos &pos)
{
  return mBlocks[cs::SBtoI(pos)];
}

void SectorTessellator::SayChanged()
{
  mChanged = true;
}

void SectorTessellator::Update(Tessellator *tesselator, Render &render)
{
  if (!mChanged)
  {
    return;
  }

  TessellatorParams params{ tesselator, this, mPos,{} };

  for (size_t i = 0; i < mBlocks.size(); ++i)
  {
    if (mBlocks[i])
    {
      params.wbpos = cs::SBtoWB(cs::ItoSB(i), mPos);
      Push(mBlocks[i]->GetModel(params), params.wbpos);
    }
  }

  render.PushModel(mModel, mModelMatrix);
  mModel.GetMesh()->Release();
}

void SectorTessellator::Push(const Model &model, const WPos &pos)
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
