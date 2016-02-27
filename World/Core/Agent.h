// ============================================================================
// ==                   Copyright (c) 2015, Smirnov Denis                    ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef Agent_h__
#define Agent_h__

#include "TemplateFactory.h"
#include <boost\noncopyable.hpp>
#include "..\tools\StringIntern.h"
#include <rapidjson\document.h>
#include "GameObjectParams.h"
#include <memory>
#include <type_traits>
#include <string>

class GameObject;

using PAgent = std::shared_ptr<class Agent>;

template<class T, class... Args>
inline std::shared_ptr<T> MakeAgent(Args&&... args)
{
  return std::make_shared<T>(std::forward<Args>(args)...);
}

/// �����.
/// ��� ������ �������� ��������� �� ����� ���������� � �� ��������. ���������� ���� �������.
/// ��� ������ �������� ��������� � ������������. ����� �������������� ��� �������������
/// ����������� �������. ����� ����� �������������.
/// � �������, ������� ����������� ������������ � ������� �������, ��� ������ �������������.
class Agent
{
public:
  /// ������� ����� � ��������� ����� � ������.
  Agent(GameObject *parent, const std::string &type, const std::string &name = "");
  virtual ~Agent();

  virtual PAgent Clone(GameObject *parent, const std::string &name = "") = 0;

  virtual void Update(const GameObjectParams &params) = 0;
  virtual void Afterload(GameObject * parent);

  virtual void JsonLoad(const rapidjson::Value &val);

  /// ������� ��� ���� ������.
  const StringIntern &GetTypeName();

  /// ������� ��� ������ ���������� ����.
  const StringIntern &GetName();

  /// ������� ������ ��� ������. �������� ��� ���� � ��� ������.
  const StringIntern &GetFullName();

protected:
  GameObject *mParent;

  const StringIntern mTypeName;
  StringIntern mAgentName;
  StringIntern mFullName;
};


#define REGISTER_AGENT(type) REGISTER_ELEMENT(type, AgentFactory::Get(), StringIntern(#type))

struct AgentFactory : public boost::noncopyable
{
  using FactoryType = TemplateFactory<StringIntern, Agent>;
  static FactoryType &Get();
};

#endif // Agent_h__