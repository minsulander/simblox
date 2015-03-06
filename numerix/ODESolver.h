#ifndef NUMERIX_ODESOLVER_H
#define NUMERIX_ODESOLVER_H

namespace numerix {

/// Interface for a dynamic system that can be solved using an ODESolver. 
template <class T>
class ODESystem
{
public:
	/// Compute initial state vector (\f$y_0\f$)
	virtual T stateInitials(const double t0) const = 0;
	/// Compute state derivatives vector (\f$\dot y\f$)
	virtual T stateDerivatives(const double t, const T& y) = 0;
	/// Return number of state variables
	virtual int size() const = 0;
	/// New state vector passed by the solver after each step
	virtual void stateUpdate(const double t, const T& y) = 0;
};

/// Base class for a solver for ordinary differential equations.
/**
	Provides a numerical solution for systems of first-order differential equations
	on the form \f$ \dot y = f(t,y) \f$. The template implementation allows the user to
	specify a state vector of desired precision and type (e.g. \c float[],
	\c std::vector<double>, \c Eigen::Vector4d, \c Eigen::MatrixXd(75,1)).
*/
template <class T>
class ODESolver
{
public:
	ODESolver(ODESystem<T>& odenew) : ode(odenew), t(0), y(odenew.size()) {}
	/// Return a name that can be used to refer to this solver class
	virtual const char* name()=0;
	/// Initializes the solver
	virtual void init(const double t0 = 0)
	{
		t = t0;
		y = ode.stateInitials(t0);
		ode.stateUpdate(t, y);
	}
	/// Steps the solution the specified time
	virtual void step(const double dt)=0;
	ODESystem<T>& system() { return ode; }

protected:
	ODESystem<T>& ode;
	T y;
	double t;
};

}

#endif
