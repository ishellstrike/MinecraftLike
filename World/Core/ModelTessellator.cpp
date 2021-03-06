#include "ModelTessellator.h"
#include "..\Render\TextureManager.h"
#include "Config.h"
#include "Tessellator.h"
#include "SectorTessellator.h"
#include "..\tools\CoordSystem.h"
#include "GameObject.h"
#include <Serealize.h>
#include <Render\Resources.h>

void ModelTessellator::SetTexture(int side, const std::string &texture)
{
	for (unsigned int i = 0; i < 7; ++i)
	{
		if (side & (1 << i))
		{
			mTextures[i] = std::get<1>(TextureManager::Get().GetTexture(texture));
		}
	}
}

ModelTessellator::ModelTessellator()
{
	mModel = std::make_shared<Model>();
	mModel->SetTexture(std::get<0>(TextureManager::Get().GetTexture("stone")));
}

void ModelTessellator::JsonLoad(const rapidjson::Value & val)
{
	if (val.HasMember("all"))
	{
		SetTexture(ALL, val["all"].GetString());
	}
	else
		if (val.HasMember("sides"))
		{
			const rapidjson::Value &arr = val["sides"];
			SetTexture(FRONT,  arr.Begin()->GetString());
			SetTexture(RIGHT,  arr[1].GetString());
			SetTexture(BACK,   arr[2].GetString());
			SetTexture(LEFT,   arr[3].GetString());
			SetTexture(TOP,    arr[4].GetString());
			SetTexture(BOTTOM, arr[5].GetString());
			SetTexture(CENTER, arr[6].GetString());
		}
		else
			LOG(error) << "missed texture";

	if (val.HasMember("model"))
	{
		const std::string &mo = val["model"].GetString();
		PMesh<VertexVTN> m = std::make_shared<TemplateMesh<VertexVTN>>(*Resources::Get().GetMesh(mo));
		const auto &uv4 = mTextures[6];
		for (size_t i = 0; i < m->SizeVertex(); i++)
		{
			m->Vertex(i).texture.x = glm::mix(uv4.x, uv4.z, m->Vertex(i).texture.x);
			m->Vertex(i).texture.y = glm::mix(uv4.y, uv4.w, m->Vertex(i).texture.y);
		}
		mModel->GetMesh() = m;
	}
	else
		LOG(error) << "missed model";

	BlockTessellator::JsonLoad(val);
}

PGameObjectTessellator ModelTessellator::Clone()
{
	return PGameObjectTessellator();
}

PModel ModelTessellator::GetModel(const TessellatorParams & params, int slise)
{
	return mModel;
}
