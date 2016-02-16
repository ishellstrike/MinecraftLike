// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef BlockRenderStratery_h__
#define BlockRenderStratery_h__

#include "MeshBlockGenerator.h"
#include "BlockTessellator.h"
#include "..\Render\Model.h"
#include "GameObjectParams.h"

/// ��������� ������ ��� �����.
/// �������� ��������� ������� �����.
class SplitBlockTessellator : public BlockTessellator
{
public:
  SplitBlockTessellator();
  ~SplitBlockTessellator() = default;

  void Load(const rapidjson::Value &val) override;

  PBlockTessellator Clone() override;

  const Model &GetModel(const GameObjectParams &params);

private:
  Model mModel;
  MeshBlockGenerator mGenerator;
};

REGISTER_BLOCK_TESSELLATOR(SplitBlockTessellator)

#endif // BlockRenderStratery_h__
