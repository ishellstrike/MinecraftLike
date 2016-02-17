// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef BlocksLibrary_h__
#define BlocksLibrary_h__

#include "Block.h"
#include <unordered_map>
#include "BlockTessellator.h"

class DB
{
public:
  static DB &Get();

  void Registry(const StringIntern &name, PBlock block, bool isStatic = true);
  void ReloadDirectory(const std::string &dir);


  /// ������� ����.
  /// ���� ���� �����������, ������������ ��������� �� ��������� �����, ���������� � ����������.
  /// ���� ���� ������������, ��������� ����� �����.
  /// @param blockId ������������� �����.
  PBlock Create(const StringIntern &name);

  PBlockTessellator CreateTesselator(const StringIntern &name);

private:
  std::unordered_map<StringIntern, PBlock> mBlocks;
  std::unordered_map<StringIntern, PBlockTessellator> mTess;

private:
  DB() {};
  ~DB() {};
  DB(const DB &) = delete;
  DB& operator=(const DB &) = delete;

};



#endif // BlocksLibrary_h__