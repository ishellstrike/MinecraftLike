// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef Block_h__
#define Block_h__

#include "GameObject.h"
#include "RenderAgent.h"

typedef std::shared_ptr<class Block> PBlock;

/// ����. ����� ���� ����������� ��� ������������.
/// Draw ������������ ����� ���������� ��� ��������� �����������
/// ��������� �������.
/// Draw ������������� ����� ���������� ������ �����.
/// � Draw ���������� Update ��� ���� ������ �������.
class Block : public GameObject
{
public:
  Block(const StringIntern &__id);
  ~Block();

  PGameObject Clone() override;

  void Update(GameObjectParams &params) override;

  void Draw(GameObjectParams &params);

  inline bool IsStatic() const noexcept
  {
    return mIsStatic;
  }

private:
  friend class DB;
  static const StringIntern mRenderAgentName;
  const bool mIsStatic = true;
};



#endif // Block_h__