// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef RenderAgent_h__
#define RenderAgent_h__


#include <memory>
#include <type_traits>
#include <boost/core/noncopyable.hpp>
#include "TemplateFactory.h"
#include "../tools/StringIntern.h"
#include "../rapidjson/document.h"


typedef std::shared_ptr<class BlockTessellator> PBlockTessellator;

template<class T, class... Args>
inline std::shared_ptr<T> MakeBlockTessellator(Args&&... args)
{
  return std::make_shared<T>(std::forward<Args>(args)...);
}


/// ���������� ������.
/// ��������� ������ �����.
class BlockTessellator
{
public:
  virtual ~BlockTessellator() = default;

  virtual PBlockTessellator Clone() = 0;

  virtual void Load(const rapidjson::Value &val);

  /// ��������� �� ����?
  inline bool IsTransparent() const noexcept
  {
    return mTransparent;
  }

  /// �������� �� ����?
  /// ������ �������� ��������� �����������, ����� �� ��������� ����.
  inline bool IsStatic() const noexcept
  {
    return mStatic;
  }

  inline const StringIntern &GetName() const noexcept
  {
    return mName;
  }

private:
  bool mStatic = true;
  bool mTransparent = false;
  StringIntern mName;
};


#define REGISTER_BLOCK_TESSELLATOR(type) REGISTER_ELEMENT(type, BlockTessellatorFactory::Get(), StringIntern(#type))

struct BlockTessellatorFactory : public boost::noncopyable
{
  using FactoryType = TemplateFactory<StringIntern, BlockTessellator>;
  static FactoryType &Get();
};

#endif // RenderAgent_h__
