// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef QueuedThread_h__
#define QueuedThread_h__


#include <functional>
#include <queue>
#include <mutex>
#include <memory>


/// ���������� ���������� ��������� ������:
/// void Start();
/// void Process();
/// void Stop();
template<class T>
class QueuedThread
{
public:
  using FunctorType = std::function<void()>;

  /// ���������������� �����������.
  QueuedThread()
  {
    mThread = std::make_unique<decltype(mThread)::element_type>([this]
    {
      static_cast<T*>(this)->Start();
      while (true)
      {
        // ������� ���� ����� ������ �����������.
        if (mDummy)
        {
          break;
        }
        // ������ ������� �������, ��� �� �������� � ������� ��� �������������.
        {
          std::lock_guard<std::mutex> lock(mMutex);
          std::swap(mQueueBack, mQueue);
        }
        // ������������ �������� ���������.
        while (!mQueue.empty())
        {
          mQueue.front()();
          mQueue.pop();
        }

        // ��������� ������� ���������.
        static_cast<T*>(this)->Process();
      }
      static_cast<T*>(this)->Stop();
    });
  }

  /// ���������������� ����������.
  ~QueuedThread()
  {
    mDummy = true;
    mThread->join();
  }

  /// ���������� ���������� ������� � ������. 
  /// ����������� �����.
  /// ���������������� ��������.
  /// ��� �������������� � ������� ������ ����������� 
  /// � ������� ������� ������.
  template<class F, class ...Args>
  void PushFunc(F func, Args... args)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    mQueueBack.push([=] { return func(args...); });
  }

private:
  std::unique_ptr<std::thread> mThread;
  std::mutex mMutex;
  std::atomic<bool> mDummy = false;
  std::queue<FunctorType> mQueue;
  std::queue<FunctorType> mQueueBack;
};



#endif // QueuedThread_h__

