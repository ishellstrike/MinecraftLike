#pragma once
#ifndef QueuedThread_h__
#define QueuedThread_h__


#include <functional>
#include <queue>
#include <mutex>
#include <memory>

class QueuedThread
{
public:
  using FunctorType = std::function<void()>;

  /// ���������������� �����������.
  QueuedThread();
  /// ���������������� ����������.
  ~QueuedThread();

  /// ���������� ���������� ������� � ������. 
  /// ���������������� ��������.
  /// ��� �������������� � ������� ������ ����������� 
  /// � ������� ������� ������.
  template<class F, class ...Args>
  FunctorType PushFunc(F func, Args... args)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return [=] { return func(args...); };
  }

protected:
  /// ���������� �������� ��� ������ ������.
  virtual void Start() = 0;

  /// ��������� ���������� � ������.
  virtual void Process() = 0;

  /// ���������� �������� ��� ���������� ������.
  virtual void Stop() = 0;


private:
  std::queue<FunctorType> mQueue;
  std::unique_ptr<std::thread> mThread;
  std::mutex mMutex;

  bool mClosed = false;
};



#endif // QueuedThread_h__

