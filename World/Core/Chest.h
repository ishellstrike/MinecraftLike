#pragma once
#include "Agent.h"
#include "GameObject.h"
#include <vector>
#include "ChestSlot.h"

class Chest : public Agent
{
public:
  Chest();

  void JsonLoad(const rapidjson::Value &val) override;

  // ������������ ����� Agent
  virtual PAgent Clone(GameObject * parent, const std::string & name = "") override;
  virtual void Update(const GameObjectParams & params) override;
  bool Push(PGameObject go, int count = 1, int pos = -1);
  const ChestSlot GetFirst() const;
  const ChestSlot GetFirst(int & pos) const;
  ChestSlot PopFirst();

  ChestSlot PopFirst(int & pos);

  /// client
  /// ������ gui ����� ������ ��� ����������� � ���������� �����. ������ ���������� ������ ����, ����� ��������� ��������� ����
  virtual void DrawGui();

  int columns = 10;
  int size = 40;
  std::vector<ChestSlot> slots;
  void Resize();
};

REGISTER_AGENT(Chest)