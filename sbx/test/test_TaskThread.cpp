#include <UnitTest++/UnitTest++.h>
#include <sbx/TaskThread.h>
#include <sbx/Timer.h>
#include <sbx/Log.h>

using namespace sbx;

class CountTask : public Task {
public:
	CountTask(int& num) : target(num) {}
	virtual void perform() {
		Timer::sleep(1);
		target++;
	}
	int& target;
};

class SleepTask : public Task {
public:
	virtual void perform() { Timer::sleep(1); }
};

TEST (CounterThread) {
	UNITTEST_TIME_CONSTRAINT(1000);
	int count = 0;
	TaskThread tt;
	tt.start();
	tt.schedule(new CountTask(count));
	tt.wait();
	CHECK_EQUAL(1, count);
	CHECK_EQUAL(0, tt.getNumScheduled());
	for (int i = 0; i < 1000; i++)
		tt.schedule(new CountTask(count));
	CHECK(tt.getNumScheduled() > 0);
	tt.wait();
	CHECK_EQUAL(0, tt.getNumScheduled());
	tt.setDone();
	while (tt.isRunning()) { }
	CHECK_EQUAL(1001, tt.getTaskCount());
	CHECK_EQUAL(1001, count);
}

TEST (SleepThreadPool) {
	UNITTEST_TIME_CONSTRAINT(2000);
	TaskThreadPool pool(4);
	pool.start();
	CHECK(!pool.isDone());
	for (unsigned int i = 0; i < 1000; i++)
		pool.schedule(new SleepTask);
	pool.wait();
	for (unsigned int i = 0; i < pool.getNumThreads(); i++)
		CHECK(pool.getThread(i).getTaskCount() > 0);
	CHECK_EQUAL(1000, pool.getTaskCount());
	pool.waitDone();
}
