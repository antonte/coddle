#include "thread_pool.hpp"
#include <optional>

ThreadPool::ThreadPool()
{
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
    pool.emplace_back(&ThreadPool::run, this);
}

ThreadPool::~ThreadPool()
{
  {
    std::lock_guard<std::mutex> guard(mutex);
    done = true;
  }
  newJob.notify_all();
  for (auto &&p : pool)
    p.join();
}

void ThreadPool::addJob(std::function<void()> &&job, std::function<void()> &&afterJob)
{
  std::lock_guard<std::mutex> guard(mutex);
  jobs.push_back(std::make_pair(job, afterJob));
  newJob.notify_one();
}

void ThreadPool::waitForOne()
{
  std::unique_lock<std::mutex> lock(mutex);
  if (!afterJobs.empty())
  {
    for (auto &&job : afterJobs)
      job();
    afterJobs.clear();
    return;
  }
  if (jobs.empty())
    return;

  jobDone.wait(lock);

  for (auto &&job : afterJobs)
    job();
  afterJobs.clear();
}

bool ThreadPool::empty()
{
  return jobs.empty();
}

void ThreadPool::run()
{
  for (;;)
  {
    auto job = [this]() -> std::optional<std::pair<std::function<void()>, std::function<void()>>> {
      std::unique_lock<std::mutex> lock(mutex);
      while (jobs.empty())
      {
        if (done)
          return std::nullopt;
        newJob.wait(lock);
        if (done)
          return std::nullopt;
      }

      auto res = jobs.back();
      jobs.pop_back();
      return res;
    }();

    if (!job)
      return;

    job->first();
    {
      std::lock_guard<std::mutex> guard(mutex);
      afterJobs.push_back(job->second);
    }
    jobDone.notify_one();
  }
}
