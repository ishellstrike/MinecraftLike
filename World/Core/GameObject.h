



#pragma once
#ifndef GameObject_h__
#define GameObject_h__

#include <map>
#include <memory>
#include <type_traits>
#include "GameObjectParams.h"
#include "Agent.h"
#include "..\tools\StringIntern.h"

using PGameObject = std::shared_ptr<GameObject>;

template<class T, class... Args>
inline std::shared_ptr<T> MakeGameObject(Args&&... args)
{
  return std::make_shared<T>(std::forward<Args>(args)...);
}

class GameObject
{
public:
  GameObject(const StringIntern &__id);
  virtual ~GameObject();

  virtual void Update(GameObjectParams &params);

  /// client
  /// ������ gui ����� ������ ��� ����������� � ���������� �����. ������ ���������� ������ ����, ����� ��������� ��������� ����
  virtual void DrawGui();

  ///client/server syncronize
  virtual void Interact(InteractParams &params);

  ///client/server paralell
  ///����������� 1 ��� ��� ������� ������ ������� �������� �������, ����������� � ���� ������, ����� ������ �������� ���������
  virtual void Afterload();

  virtual PGameObject Clone();

  Agent *GetFromFullName(const StringIntern &name);

  const Agent *GetFromFullName(const StringIntern &name) const;

  template<class T>
  T *GetFromFullName(const StringIntern &name)
  {
    return static_cast<T*>(GetFromFullName(name));
  }

  template<class T>
  const T *GetFromFullName(const StringIntern &name) const
  {
    return static_cast<const T*>(GetFromFullName(name));
  }

  template<class T>
  T *GetFromFullName(const std::string &name)
  {
    return static_cast<T*>(GetFromFullName(StringIntern(name)));
  }

  template<class T>
  const T *GetFromFullName(const std::string &name) const
  {
    return static_cast<T*>(GetFromFullName(StringIntern(name)));
  }

  void PushAgent(PAgent go);
  inline bool IsPlacable()
  {
    return placable;
  }

  StringIntern GetId();

protected:
  friend class DB;
  std::map<StringIntern, PAgent> mAgents;

  friend class TemplateItemMaterial;
  friend class TemplateMachine;
  StringIntern id;
  bool placable = false;
};



#endif // GameObject_h__
