#ifndef WORKER_H
#define WORKER_H

class ThreadPool;
class Worker
{
public:
  Worker(ThreadPool &p) : pool(p) {}
  void operator()();
  Worker& operator=(const Worker&) = delete;
private:
  ThreadPool &pool;
};

#endif
