#ifndef SBX_STATESPACEMODEL_H
#define SBX_STATESPACEMODEL_H

#include "Model.h"
#include "Ports.h"
#include "Export.h"
#include <numerix/ODESolver.h>
#include <Eigen/Core>

namespace sbx {

/// A generic state-space model
template <typename T, int _states, int _controls, int _outputs>
class StateSpaceModel : public Model, protected numerix::ODESystem< Eigen::Matrix<T, _states, 1> >
{
public:
	StateSpaceModel()
	:	Model(),
		solver(NULL),
		t0(0)
	{
		A.setZero();
		B.setZero();
		C.setZero();
		D.setZero();
		states0.setZero();
		states.setZero();
		controls.setZero();
		outputs.setZero();
	}
	
	StateSpaceModel(const StateSpaceModel& source)
	:	Model(source),
		solver(NULL),
		t0(source.t0),
		A(source.A),
		B(source.B),
		C(source.C),
		D(source.D),
		states0(source.states0),
		states(source.states),
		controls(source.controls),
		outputs(source.outputs)
	{
		/// \todo shallow copy solver
	}
	
	virtual ~StateSpaceModel()
	{
		if (solver)
			delete solver;
	}
	
	virtual void init()
	{
		if (!solver)
			throw ModelException("No solver", this);
		setupInitials();
		setupMatrices();
		solver->init(t0);
		transferOutputs();
	}
	
	/// Transfer inputs from ports to the \c controls vector (or similar), called before stepping the solver
	virtual void transferInputs() = 0;
	/// Transfer outputs from the \c outputs vector to ports, called after stepping the solver
	virtual void transferOutputs() = 0;
	/// Setup initial values in the \c states0 vector and \c t0. Defaults to all-zeroes
	virtual void setupInitials() {}
	/// Setup system matrices \c A, \c B, \c C and \c D
	virtual void setupMatrices() = 0;
	
	virtual void update(const double dt)
	{
		if (!solver)
			throw ModelException("No solver", this);
		transferInputs();
		solver->step(dt);
		transferOutputs();
	}
	
protected:

	virtual Eigen::Matrix<T, _states, 1> stateInitials(const double t0) const
	{
		return states0;
	}
	virtual Eigen::Matrix<T, _states, 1> stateDerivatives(const double t, const Eigen::Matrix<T, _states, 1>& y)
	{
		return A*y + B*controls;
	}
	virtual int size() const { return _states; }
	virtual void stateUpdate(const double t, const Eigen::Matrix<T, _states, 1>& y)
	{
		states = y;
		outputs = C*states + D*controls;
	}
		
	Eigen::Matrix<T, _states, _states> A;
	Eigen::Matrix<T, _states, _controls> B;
	Eigen::Matrix<T, _outputs, _states> C;
	Eigen::Matrix<T, _outputs, _controls> D;
	Eigen::Matrix<T, _states, 1> states;
	Eigen::Matrix<T, _controls, 1> controls;
	Eigen::Matrix<T, _outputs, 1> outputs;
	Eigen::Matrix<T, _states, 1> states0;
	double t0;
	numerix::ODESolver< Eigen::Matrix<T, _states, 1> >* solver;
};

/** \class StateSpaceModel
	This is a template model class for state-space models on the form
		\f[ \dot x = A x + B u \f]
		\f[ y = C x + D u \f]
	where \e x is the state vector, \e u the control vector and \e y the output vector.
	
	Users should derive from this class (using template arguments as appropriate to the model) and
	implement transferInputs(), transferOutputs(), setupMatrices() and possibly setupInitials().
	The constructor also needs to be overridden and used to setup ports and a solver.
	
	\see test_StateSpaceModel.cpp for an example
*/
}

#endif
