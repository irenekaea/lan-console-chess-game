#include "Worker.h"
#include "ThreadPool.h"

void Worker::operator()()
{
  std::function<void()> job;

  for (;;)
  {
    {
      // Create lock and lock
      std::unique_lock<std::mutex> lock(pool.queue_mutex);

      // Look for job
      while (!pool.end_pool.load() && pool.jobs.empty())
      {
        // If the jobs queue is empty, wait for new job
        pool.condition.wait(lock);
      }

      // Terminate all threads
      if (pool.end_pool)
        return;

      job = pool.jobs.front();
      pool.jobs.pop();
    } // Release lock

    // Run job
    if (job != nullptr)
      job();
  }
}
