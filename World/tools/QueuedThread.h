// ============================================================================
// ==         Copyright (c) 2016, Samsonov Andrey and Smirnov Denis          ==
// ==                  See license.txt for more information                  ==
// ============================================================================
#pragma once
#ifndef QueuedThread_h__
#define QueuedThread_h__


#include <functional>
#include <list>
#include <mutex>
#include <memory>
#include <atomic>


/// ���������� ���������� ��������� ������:
/// void Start();
/// void Process();
/// void Stop();
template<class T>
class QueuedThread
{
public:
  using FunctorType = std::function<void()>;

  /// ��������� �����.
  void Run()
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
          mQueue.swap(mQueueBack);
        }
        // ������������ �������� ���������.
        while (!mQueue.empty())
        {
          mQueue.front()();
          mQueue.pop_front();
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
    if (mThread)
    {
      mDummy = true;
      mThread->join();
    }
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
    mQueueBack.push_back([=] { return func(args...); });
  }

private:
  std::unique_ptr<std::thread> mThread;
  std::mutex mMutex;
  std::atomic<bool> mDummy = false;
  std::list<FunctorType> mQueue;
  std::list<FunctorType> mQueueBack;
};



#endif // QueuedThread_h__

