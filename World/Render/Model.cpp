




#include "Model.h"
#include "Resourses.h"
#include <Serealize.h>
#include"TextureManager.h"

Model::Model()
{
  mShader = Resourses::Get().GetShader("shaders/basic.glsl");
}

void Model::JsonLoad(const rapidjson::Value & val)
{
  if (val.HasMember("sprite"))
  {
    auto s = val["sprite"].GetString();

    mTexture = std::get<0>(TextureManager::Get().GetTexture(s));
    mSprite = s;
  }
}

void Model::SetSprite(const StringIntern & s)
{
  mTexture = std::get<0>(TextureManager::Get().GetTexture(s));
  mSprite = s;
}

void Model::BuildAABB(glm::vec3 VertexVTN::* p)
{
	mMeshes->BuildAABB(p);
}

bool Model::IsAabbDot()
{
	return mMeshes->IsAabbDot();
}
