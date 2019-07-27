#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional> 
#include <vector>
#include <queue>
#include <atomic>

class ThreadPool
{
public:
  ThreadPool(size_t num_threads);
  ~ThreadPool();

  void AddToQueue(std::function<void()> fn);

private:
  friend class Worker;
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> jobs;

  std::mutex queue_mutex;
  std::condition_variable condition;
  std::atomic<bool> end_pool;
};



#endif
