#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace turret {

template <typename T> class SharedQueue {
public:
  SharedQueue();
  ~SharedQueue();

  T get();
  T get_ready();

  void push_back(const T &item);
  void push_back(T &&item);

  int size();
  bool empty();

private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cond;
};

template <typename T> SharedQueue<T>::SharedQueue() {}

template <typename T> SharedQueue<T>::~SharedQueue() {}

template <typename T> T SharedQueue<T>::get() {
  std::unique_lock<std::mutex> mlock(m_mutex);
  while (m_queue.empty()) {
    m_cond.wait(mlock);
  }
  T e = std::move(m_queue.front());
  m_queue.pop();
  return e;
}

template <typename T> T SharedQueue<T>::get_ready() {
  std::unique_lock<std::mutex> mlock(m_mutex);
  if (m_queue.empty()) {
    return nullptr;
  }
  T e = std::move(m_queue.front());
  m_queue.pop();
  return e;
}

template <typename T> void SharedQueue<T>::push_back(const T &item) {
  std::unique_lock<std::mutex> mlock(m_mutex);
  m_queue.push(item);
  mlock.unlock();      // unlock before notificiation to minimize mutex con
  m_cond.notify_one(); // notify one waiting thread
}

template <typename T> void SharedQueue<T>::push_back(T &&item) {
  std::unique_lock<std::mutex> mlock(m_mutex);
  m_queue.push(std::move(item));
  mlock.unlock();      // unlock before notificiation to minimize mutex con
  m_cond.notify_one(); // notify one waiting thread
}

template <typename T> int SharedQueue<T>::size() {
  std::unique_lock<std::mutex> mlock(m_mutex);
  int size = m_queue.size();
  mlock.unlock();
  return size;
}

template <typename T> bool SharedQueue<T>::empty() { return size() == 0; }


} // namespace turret