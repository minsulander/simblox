#include "TaskThread.h"
#include "Log.h"
#include <limits.h>

namespace sbx {
	
	TaskThread::TaskThread()
	: OpenThreads::Thread(), done(true), busy(false), inhibit(false), taskCount(0)
	{
	}
	
	TaskThread::TaskThread(const TaskThread& source)
	: OpenThreads::Thread(), done(true), busy(false), inhibit(source.inhibit),
	tasks(source.tasks), scheduleMutex(), doneMutex(), countMutex(), taskCount(0)
	{
	}
	
	TaskThread& TaskThread::operator=(const TaskThread& source)
	{
		done = true;
		busy = false;
		inhibit = source.inhibit;
		tasks = source.tasks;
		taskCount = 0;
		return *this;
	}
	
	void TaskThread::run()
	{
		done = false;
		while (!isDone()) {
			smrt::ref_ptr<Task> task = NULL;
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(inhibitMutex);
				if (inhibit) {
					OpenThreads::ReverseScopedLock<OpenThreads::Mutex> unlock(inhibitMutex);
					OpenThreads::Thread::microSleep(10);
					continue;
				}
			}				
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(scheduleMutex);
				if (tasks.size() == 0) {
					OpenThreads::ReverseScopedLock<OpenThreads::Mutex> unlock(scheduleMutex);
					if (busy) {
						OpenThreads::ScopedLock<OpenThreads::Mutex> busylock(busyMutex);
						busy = false;
					}
					OpenThreads::Thread::microSleep(10);
					continue;
				}
				task = tasks[0].get();
				tasks.erase(tasks.begin());
			}
			if (task.valid()) {
				OpenThreads::ScopedLock<OpenThreads::Mutex> tasklock(taskMutex);
				task->perform();
				OpenThreads::ScopedLock<OpenThreads::Mutex> countlock(countMutex);
				taskCount++;
			}
		}
	}
	
	void TaskThread::schedule(Task *task)
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> schedlock(scheduleMutex);
		tasks.push_back(task);
		OpenThreads::ScopedLock<OpenThreads::Mutex> busylock(busyMutex);
		busy = true;
	}
	
	unsigned int TaskThread::getNumScheduled()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(scheduleMutex);
		return tasks.size();
	}
	
	void TaskThread::wait()
	{
		while (true) {
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(busyMutex);
			if (!busy)
				break;
			OpenThreads::ReverseScopedLock<OpenThreads::Mutex> unlock(busyMutex);
			OpenThreads::Thread::microSleep(10);
		}
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(taskMutex);
	}
	
	unsigned long TaskThread::getTaskCount()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(countMutex);
		return taskCount;
	}
	
	void TaskThread::setDone()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(doneMutex);
		done = true;
	}
	
	bool TaskThread::isDone()
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(doneMutex);
		return done;
	}
	
	void TaskThread::setInhibit(const bool value)
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(inhibitMutex);
		inhibit = value;
	}
	
	TaskThreadPool::TaskThreadPool(const unsigned int numThreads)
	{
		threads.resize(numThreads);
	}
	
	TaskThread& TaskThreadPool::getThread(const unsigned int index) { return threads[index]; }
	
	void TaskThreadPool::start()
	{
		for (size_t i = 0; i < threads.size(); i++)
			threads[i].start();
	}
	
	void TaskThreadPool::schedule(Task *task)
	{
		unsigned int min = UINT_MAX, minindex = UINT_MAX;
		for (size_t i = 0; i < threads.size(); i++) {
			unsigned int num = threads[i].getNumScheduled();
			if (min > num) {
				min = num;
				minindex = i;
			}
		}
		threads[minindex].schedule(task);
	}
	
	unsigned int TaskThreadPool::getNumScheduled()
	{
		unsigned int count = 0;
		for (size_t i = 0; i < threads.size(); i++)
			count += threads[i].getNumScheduled();
		return count;
	}
	
	void TaskThreadPool::wait()
	{
		for (size_t i = 0; i < threads.size(); i++)
			threads[i].wait();
	}
	
	unsigned long TaskThreadPool::getTaskCount()
	{
		unsigned long count = 0;
		for (size_t i = 0; i < threads.size(); i++)
			count += threads[i].getTaskCount();
		return count;
	}
	
	void TaskThreadPool::setDone()
	{
		for (size_t i = 0; i < threads.size(); i++)
			threads[i].setDone();
	}
	
	bool TaskThreadPool::isDone()
	{
		for (size_t i = 0; i < threads.size(); i++)
			if (!threads[i].isDone())
				return false;
		return true;
	}
	
	void TaskThreadPool::waitDone()
	{
		setDone();
		for (size_t i = 0; i < threads.size(); i++)
			while (threads[i].isRunning()) {}
	}
	
	void TaskThreadPool::setInhibit(const bool value)
	{
		for (size_t i = 0; i < threads.size(); i++)
			threads[i].setInhibit(value);
	}
	
	
} // namespace
