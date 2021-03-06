#pragma once
#ifndef BlockRenderStratery_h__
#define BlockRenderStratery_h__

#include "MeshPartialBlockGenerator.h"
#include "BlockTessellator.h"
#include "..\Render\Model.h"
#include "TessellatorParams.h"

// ��������� ������ ��� �����.
// �������� ��������� ������� �����.
class SplitBlockTessellator : public BlockTessellator
{
public:
  SplitBlockTessellator();
  ~SplitBlockTessellator() = default;

  void JsonLoad(const rapidjson::Value &val) override;

  PGameObjectTessellator Clone() override;

  PModel GetModel(const TessellatorParams &params, int slise = 9999) override;

private:
  PModel mModel;
  MeshPartialBlockGenerator mGenerator;
};

REGISTER_BLOCK_TESSELLATOR(SplitBlockTessellator)

#endif // BlockRenderStratery_h__
