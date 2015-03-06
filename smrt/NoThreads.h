#ifndef SMRT_NOTHREADS_H
#define SMRT_NOTHREADS_H

namespace OpenThreads {

template<class T>
class ScopedLock {
public:
	ScopedLock(T& theT) {}
};

class Mutex {};

}

#endif
