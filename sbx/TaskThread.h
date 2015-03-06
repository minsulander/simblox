#include "Export.h"
#include "Model.h"
#include <smrt/Referenced.h>
#include <smrt/ref_ptr.h>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

namespace sbx {
	
	class SIMBLOX_API Task : public smrt::Referenced {
	public:
		virtual void perform() = 0;
	};
	
	class SIMBLOX_API TaskThread : public OpenThreads::Thread {
	public:
		TaskThread();
		TaskThread(const TaskThread& source);
		TaskThread& operator=(const TaskThread& source);
		
		virtual void run();
		
		/// Schedule a task to be performed by this thread
		void schedule(Task *task);
		/// Get number of scheduled tasks
		unsigned int getNumScheduled();
		
		/// Wait until all scheduled tasks have been performed
		void wait();
		/// Get number of tasks performed
		unsigned long getTaskCount();
		
		/// Tells the thread to quit
		void setDone();
		bool isDone();
		
		void setInhibit(const bool value);
		
	private:
		bool done, busy, inhibit;
		std::vector< smrt::ref_ptr<Task> > tasks;
		unsigned long taskCount;
		OpenThreads::Mutex doneMutex, busyMutex, inhibitMutex, scheduleMutex, taskMutex, countMutex;
	};
	
	class SIMBLOX_API TaskThreadPool {
	public:
		TaskThreadPool(const unsigned int numThreads);
		TaskThread& getThread(const unsigned int index);
		unsigned int getNumThreads() { return threads.size(); }
		
		void start();
		
		/// Schedule a task to be performed by a thread in this pool
		void schedule(Task *task);
		/// Get number of scheduled tasks
		unsigned int getNumScheduled();
		
		/// Wait until all scheduled tasks have been performed
		void wait();
		/// Get number of tasks performed
		unsigned long getTaskCount();

		/// Tells all threads to quit
		void setDone();
		/// Returns true if all threads have quit
		bool isDone();
		/// Wait until all threads have stopped
		void waitDone();
		
		void setInhibit(const bool value);
		
	private:
		std::vector<TaskThread> threads;
	};

}
