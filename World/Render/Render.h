// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef RenderList_h__
#define RenderList_h__

#include <list>
#include <glm/glm.hpp>
#include "Model.h"
#include <mutex>
#include <memory>

class RenderIterator;

/// ������ ��������� (�����):
/// ������ ����� ���� ������ �������(���������������� ������),
/// ��� ��� �������� ������ ����.
/// ��� ������ � ���� ������ �������������� � �� ����� ���� ��������.
/// ���������� ������ � ����� ���������������.
/// ��� ���������� ������ � ����� �� ��� ������������ ��������.
/// �������� ������������� ���������������� ������ � ������� ������.
/// 
/// ��� ���������� � �����, ������ ���������� � ������ �� ����������.
/// � ������ ����� �������� �������, ��� ������ �� ������ �� ���������� 
/// ������������� � ������������ � ������ �������.
/// 
/// ����� �������� ����� �������� ������.� ���� ������ ����� ������ ���������
/// � ������ �� ����������, ������ ������ ��������� � ������ �� ��������.
class Render
{
public:
  Render();
  ~Render();

  static void Initialize();

  /// �������� ������ �� ���������. 
  RenderIterator PushModel(const Model &model, const glm::mat4 &matrix);

  void Draw(class Camera &camera);

private:
  void AddElements();

  void SwapMatrix();

private:
  friend RenderIterator;
  struct Element
  {
    using Iterator = std::list<Element>::iterator;

    Element(const Model &mod, const glm::mat4 &mat)
      : model(mod), matrixBack(mat)
    {}
    Model model;
    glm::mat4 matrix;
    glm::mat4 matrixBack;
    std::shared_ptr<Iterator> mIterator;
  };
  // ������ ��������� �� ���������.
  std::list<Element> mDrawList;
  // ������ ��������� �� ����������.
  std::list<Element> mAddList;
  // ������ ��������� �� ��������.
  std::list<Element> mRemoveList;

  mutable std::mutex mMutex;

  glm::ivec2 mVersion;
};

class RenderIterator
{
public:
  using ElementType = Render::Element;
  using IteratorType = std::weak_ptr<ElementType::Iterator>;

  inline RenderIterator(const IteratorType &iterator, std::mutex &mutex) noexcept
    : mIterator(iterator), mMutex(mutex)
  {}

  /// ��������� ������. ���������������� ��������.
  inline const Model &GetModel() const noexcept
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return (*mIterator.lock())->model;
  }

  /// ��������� ������� ������. ���������������� ��������.
  inline void SetMatrix(const glm::mat4 &matrix)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    (*mIterator.lock())->matrixBack = matrix;
  }

private:
  IteratorType mIterator;
  std::mutex &mMutex;
};


#endif // RenderList_h__
