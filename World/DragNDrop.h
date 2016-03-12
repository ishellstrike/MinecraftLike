#pragma once
#include <Core\GameObject.h>
#include <Core\ChestSlot.h>
class DragNDrop
{
public:

  void Drag(ChestSlot go);
  ChestSlot Drop();
  static DragNDrop& Get();
  bool Busy();

private:
  //TODO: ������� ������������ �������� �� �����, ���� ���� �� ��������� �� � ���� �� ����������
  ChestSlot dragged;
};