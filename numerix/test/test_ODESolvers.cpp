#include <UnitTest++/UnitTest++.h>
#include <numerix/RungeKuttaSolvers.h>
#include <sbx/Timer.h>
#include <sbx/Log.h>
#include <math.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <Eigen/Core>

using namespace sbx;
using namespace numerix;
using namespace Eigen;

template <class T>
class TestODESystem : public ODESystem<T> {
public:
	TestODESystem(int sz) : y(sz,1) {}
	virtual T stateInitials(const double t0) const {
		T y0(y.rows(),1);
		y0.setZero();
		return y0;
	}
	virtual T stateDerivatives(const double t, const T& states) {
		stateUpdate(t, states);
		T ydot(y.rows(),1);
		for (int i = 0; i < y.rows(); i++)
			ydot[i] = y[i] + t + 1;
		return ydot;
	}
	virtual int size() const { return y.rows(); }
	virtual void stateUpdate(const double t, const T& states) { this->t = t; y = states; }
	double value() { return y[0]; }
	double error() { return fabs(y[0] - (-2-t+2*exp(t))); }
	T y;
	double t;
};

template <typename T>
class SingleSystem : public ODESystem<T> {
public:
	SingleSystem() : y(0) {}
	virtual T stateInitials(const double t0) const { return 0; }
	virtual T stateDerivatives(const double t, const T& y1) { return y1 + t + 1; }
	virtual int size() const { return 1; }
	virtual void stateUpdate(const double t, const T& y1) { this->t = t; y = y1; }
	double value() { return y; }
	double error() { return fabs(y - (-2-t+2*exp(t))); }
	T y;
	double t;
};

template <class T, class System>
struct SolverFixture
{
	SolverFixture(System& newode)
	: ode(newode)
	{
		solvers.push_back(new EulerSolver<T>(ode));
		solvers.push_back(new HeunSolver<T>(ode));
		solvers.push_back(new MidpointSolver<T>(ode));
		solvers.push_back(new RK3Solver<T>(ode));
		solvers.push_back(new RK4Solver<T>(ode));
	}
	
	std::vector< ODESolver<T>* > solvers;
	System& ode;
};

TEST(SolverTest)
{
	TestODESystem<Vector4d> testode(4);
	SolverFixture< Vector4d, TestODESystem<Vector4d> > fix(testode);

	dout(1) << "Solver accuracy\n";
	const double tfinal = 1;
	const double dt = tfinal/3;
	int winner = -1;
	double minerror = 100;
	for (int i = 0; i < fix.solvers.size(); i++) {
		UNITTEST_TIME_CONSTRAINT(500);
		ODESolver<Vector4d>& solver = *fix.solvers[i];
		solver.init();
		double t;
		for (t = 0; t < tfinal-dt/10; t += dt) {
			solver.step(dt);
		}
		dout(1) << "  " << solver.name() << ": err = " << fix.ode.error() << "\n";
		if (fix.ode.error() < minerror)
			winner = i;
		CHECK(fix.ode.error() < 1);
	}
	dout(1) << "and the winner is... " << fix.solvers[winner]->name() << "!\n";
}
	
template <class T, class System> 
void benchmarkSolvers(T v, System& ode, unsigned int numsteps)
{
	SolverFixture<T, System> fix(ode);
	const double tfinal = 1;
	const double dt = tfinal/numsteps;
	for (int i = 0; i < fix.solvers.size(); i++) {
		Timer timer;
		ODESolver<T>& solver = *fix.solvers[i];
		solver.init();
		double t;
		for (t = 0; t < tfinal-dt/10; t += dt) {
			solver.step(dt);
		}
		dout(1) << "  " << solver.name() << ": y(" << t << ") = " << ode.value() << ", err = " << ode.error() << ", tsolve = " << timer.time_s() << " seconds\n";
	}

}


TEST(SolverBenchmark)
{
	TestODESystem<VectorXd> sys50x(50);
	TestODESystem< Matrix<double, 50, 1> > sys50d(50);
	TestODESystem< Matrix<double, 1, 1> > sys1d(1);
	SingleSystem<double> sysd;
	dout(1) << "VectorXd(50)\n";
	benchmarkSolvers( VectorXd(50), sys50x, 1000 );
	dout(1) << "Vector50d\n";
	benchmarkSolvers( Matrix<double, 50, 1>(50), sys50d, 1000 );
	dout(1) << "Vector1d\n";
	benchmarkSolvers( Matrix<double, 1, 1>(1), sys1d, 10000 );
	dout(1) << "double\n";
	benchmarkSolvers( 1.0, sysd, 10000 );
}
