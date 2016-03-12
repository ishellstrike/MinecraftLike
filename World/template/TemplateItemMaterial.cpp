#include "TemplateItemMaterial.h"
#include <core\GameObject.h>
#include <core\DB.h>
#include <Render\TextureManager.h>

void TemplateItemMaterial::Generate()
{
  Expand(materials);
  
  for (const auto &s : materials)
  {
    auto ngo = go->Clone();
    ngo->id = go->id + StringIntern("_") + s;

    auto basemodel = DB::Get().GetModel(go->id);
    auto nmodel = std::make_shared<Model>(*basemodel);
    TextureManager::Get().LoadTextureMultiplied(basemodel->GetSpriteName(), s);
    nmodel->SetSprite(basemodel->GetSpriteName() + StringIntern("_") + s);
    DB::Get().PushModel(ngo->id, nmodel);


    DB::Get().Registry(ngo->id, std::shared_ptr<GameObject>(ngo));
  }
}

void TemplateItemMaterial::JsonLoad(const rapidjson::Value & val)
{
  JSONLOAD(NVP(materials));
}
