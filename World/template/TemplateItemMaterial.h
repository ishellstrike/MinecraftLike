#pragma once
#include "Template.h"
#include <vector>
#include <tools\StringIntern.h>

class TemplateItemMaterial : public Template
{
public:

  // ������������ ����� Template
  virtual void Generate() override;
  virtual void JsonLoad(const rapidjson::Value & val) override;

  std::vector<StringIntern> materials;
};

REGISTER_TEMPLATE(TemplateItemMaterial)