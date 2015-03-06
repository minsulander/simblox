#ifndef NUMERIX_RUNGEKUTTASOLVERS_H
#define NUMERIX_RUNGEKUTTASOLVERS_H

#include "ODESolver.h"

namespace numerix
{

/// First order Runge-Kutta (Euler's method) solver for systems of ordinary differential equations.
template <class T>
class EulerSolver : public ODESolver<T>
{
public:
	EulerSolver(ODESystem<T>& ode) : ODESolver<T>(ode) {}
	virtual const char* name() { return "Euler"; }
	virtual void step(const double dt)
	{
		ODESystem<T>& ode = this->ode; T& y = this->y; double& t = this->t;
		y = y + dt * ode.stateDerivatives(t, y);
		t += dt;
		ode.stateUpdate(t, y);
	}
};


/// Second order Runge-Kutta (Heun's method) solver for systems of ordinary differential equations.
template <class T>
class HeunSolver : public ODESolver<T>
{
public:
	HeunSolver(ODESystem<T>& ode) : ODESolver<T>(ode) {}
	virtual const char* name() { return "Heun"; }
	virtual void step(const double dt)
	{
		ODESystem<T>& ode = this->ode; T& y = this->y; double& t = this->t;
		T	k1(ode.size()), 
			k2(ode.size()); 
			
		k1 = ode.stateDerivatives(t, y);
		k2 = ode.stateDerivatives(t + 0.5*dt, y + 0.5*k1*dt);
		y = y + k2*dt;
		t += dt;
		ode.stateUpdate(t, y);
	}
};

/// Third-order Runge-Kutta solver for systems of ordinary differential equations.
template <class T>
class RK3Solver : public ODESolver<T>
{
public:
	RK3Solver(ODESystem<T>& ode) : ODESolver<T>(ode) {}
	virtual const char* name() { return "RK3"; }
	virtual void step(const double dt)
	{
		ODESystem<T>& ode = this->ode; T& y = this->y; double& t = this->t;
		T	k1(ode.size()), 
			k2(ode.size()), 
			k3(ode.size());
				
		k1 = ode.stateDerivatives(t, y);
		k2 = ode.stateDerivatives(t + 0.5*dt, y + 0.5*k1*dt);
		k3 = ode.stateDerivatives(t + dt, y - k1*dt + 2.0*k2*dt);
		y = y + (k1 + 4.0*k2 + k3) / 6.0 * dt;
		t += dt;
		ode.stateUpdate(t, y);
	}
};

/// Fourth-order Runge-Kutta solver for systems of ordinary differential equations.
template <class T>
class RK4Solver : public ODESolver<T>
{
public:
	RK4Solver(ODESystem<T>& ode) : ODESolver<T>(ode) {}
	virtual const char* name() { return "RK4"; }
	virtual void step(const double dt)
	{
		ODESystem<T>& ode = this->ode; T& y = this->y; double& t = this->t;
		/// \todo should these vectors be members instead to hinder allocation/deallocation every step? (same for Heun and RK3)
		T	k1(ode.size()), 
			k2(ode.size()), 
			k3(ode.size()), 
			k4(ode.size());
			
		k1 = ode.stateDerivatives(t, y);
		k2 = ode.stateDerivatives(t + 0.5*dt, y + 0.5*k1*dt);
		k3 = ode.stateDerivatives(t + 0.5*dt, y + 0.5*k2*dt);
		k4 = ode.stateDerivatives(t + dt, y + k3*dt);
		y = y + (k1 + 2.0*k2 + 2.0*k3 + k4) / 6.0 * dt;
		t += dt;
		ode.stateUpdate(t, y);
	}
};

template <class T>
class MidpointSolver : public ODESolver<T>
{
public:
	MidpointSolver(ODESystem<T>& ode) : ODESolver<T>(ode) {}
	virtual const char* name() { return "Midpoint"; }
	virtual void step(const double dt)
	{
		ODESystem<T>& ode = this->ode; T& y = this->y; double& t = this->t;
		y = y + dt * ode.stateDerivatives(t + dt/2, y + dt/2*ode.stateDerivatives(t, y) );
		t += dt;
		ode.stateUpdate(t, y);
	}
};

} // namespace sbxNumerics

#endif

