#include "ThreadPool.h"
#include "Worker.h"
#include <iostream>

ThreadPool::ThreadPool(size_t num_threads): end_pool(false)
{
  workers.reserve(num_threads * sizeof(Worker));
  for (size_t i = 0; i < num_threads; ++i)
    workers.push_back(std::thread(Worker(*this)));
}

ThreadPool::~ThreadPool()
{
  // Stop all threads
  end_pool.store(true);
  condition.notify_all();

  // Join all
  for (auto &elem : workers)
    elem.join();
}

void ThreadPool::AddToQueue(std::function<void()> fn)
{
  {
    // Create lock and lock
    std::unique_lock<std::mutex> lck(queue_mutex);
    // Add new job
    jobs.push(std::move(fn));
  }// Unlock

  // Ask one thread to wake up and do work
  condition.notify_one(); 
}