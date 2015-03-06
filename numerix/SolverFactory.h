#ifndef NUMERIX_SOLVERFACTORY_H
#define NUMERIX_SOLVERFACTORY_H

#include "RungeKuttaSolvers.h"

namespace numerix {

template <class T>
class SolverFactory
{
public:
	static ODESolver<T>* create(const std::string& name, ODESystem<T>& system) {
		if (name == "euler")
			return new EulerSolver<T>(system);
		else if (name == "heun")
			return new HeunSolver<T>(system);
		else if (name == "rk3")
			return new RK3Solver<T>(system);
		else if (name == "rk4")
			return new RK4Solver<T>(system);
		else if (name == "midpoint")
			return new MidpointSolver<T>(system);
		return NULL;
	}
};

}

#endif
