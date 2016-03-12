#pragma once
#include "Template.h"
#include <vector>
#include <tools\StringIntern.h>

struct TemplateItemMaterial : public Template
{
  // ������������ ����� Template
  virtual void Generate() override;
  virtual void JsonLoad(const rapidjson::Value & val) override;

  std::vector<StringIntern> materials;
};

REGISTER_TEMPLATE(TemplateItemMaterial)