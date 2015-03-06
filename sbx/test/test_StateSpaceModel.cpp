#include <UnitTest++/UnitTest++.h>
#include <sbx/StateSpaceModel.h>
#include <numerix/RungeKuttaSolvers.h>
#include <iostream>

// A simple SISO state-space model of a DC motor, found in the documentation of a
// popular scientific computing application.

class DCMotor : public sbx::StateSpaceModel<double, 2, 1, 1> {
public:
	DCMotor() : sbx::StateSpaceModel<double, 2, 1, 1>()
	{
		solver = new numerix::RK4Solver< Eigen::Matrix<double, 2, 1> >(*this);
		registerPort(vapp, "vapp", "volt", "Applied voltage");
		registerPort(omega, "omega", "radian/second", "Output shaft rotational speed");
		registerParameter(&R, Parameter::DOUBLE, "R", "ohm");
		registerParameter(&L, Parameter::DOUBLE, "L", "henry");
		registerParameter(&Km, Parameter::DOUBLE, "Km");
		registerParameter(&Kb, Parameter::DOUBLE, "Kb");
		registerParameter(&Kf, Parameter::DOUBLE, "Kf");
		registerParameter(&J, Parameter::DOUBLE, "J","kg * m^2");
	}
	META_Object(test, DCMotor);
	virtual const char* description() const { return "Simple test of sbx::StateSpaceModel"; }
	virtual void transferInputs()
	{
		controls(0,0) = *vapp;
	}
	virtual void transferOutputs()
	{
		omega = outputs(0,0);
	}
	virtual void setupMatrices()
	{
		A(0,0) = -R/L;			A(0,1) = -Kb/L;
		A(1,0) = Km/J;			A(1,1) = -Kf/J;
		B(0,0) = 1/L;
		C(0,1) = 1;
	}
	
protected:
	sbx::InUnitPort<double> vapp;
	sbx::OutUnitPort<double> omega;
	double R, L, Km, Kb, Kf, J;
};

// This test sets up parameters according to the example, and compares the step
// and impulse responses to the result of the Octave script 
// scripts/octave/test_StateSpaceModel.m, found in the distribution.

TEST(StateSpaceModel) {
	DCMotor motor;
	motor.setParameter("R", 2);
	motor.setParameter("L", 0.5);
	motor.setParameter("Km", 0.015);
	motor.setParameter("Kb", 0.015);
	motor.setParameter("Kf", 0.2);
	motor.setParameter("J", 0.02);
	sbx::OutUnitPort<double> v_out;
	sbx::InUnitPort<double> omega_in;
	v_out.connect(motor.getPort("vapp"));
	omega_in.connect(motor.getPort("omega"));
	v_out = 1;
	motor.init();
	
	// Check step response
	for (double t = 0; t < 0.25; t += 0.01)
		motor.update(0.01);
	CHECK_CLOSE(0.0165, *omega_in, 0.0001);
	for (double t = 0.25; t < 0.5; t += 0.01)
		motor.update(0.01);
	CHECK_CLOSE(0.0292, *omega_in, 0.0001);
	for (double t = 0.5; t < 1; t += 0.01)
		motor.update(0.01);
	CHECK_CLOSE(0.0363, *omega_in, 0.0001);
	
	// Re-initializing should reset states and output ports
	motor.init();
	CHECK_CLOSE(0, *omega_in, 0.001);
	
	// Check impulse response
	v_out = 100;
	double tmax = 0, ymax = 0;
	for (double t = 0; t < 1; t += 0.01) {
		if (*omega_in > ymax) {
			ymax = *omega_in;
			tmax = t;
		}
		motor.update(0.01);
		v_out = 0;
	}
	CHECK_CLOSE(0.0813, ymax, 0.001);
	CHECK_CLOSE(0.16, tmax, 0.001);
	CHECK_CLOSE(0.0046, *omega_in, 0.001);
}


