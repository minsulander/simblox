#include <UnitTest++/UnitTest++.h>
#include <sbx/Ports.h>
#include <sbx/Model.h>

#include <string>
#include <iostream>

using namespace sbx;

TEST(Ports) {
	InUnitPort<double> ind("m");
	OutUnitPort <double> outd("ft");
	InPort<std::string> ins;
	OutPort<std::string> outs;
	
	// Connection logic
	CHECK(!ind.isConnected() && !outd.isConnected() && !ind.isValid() && outd.isValid());
	ind.connect(&outd);
	CHECK(ind.isConnected() && outd.isConnected() && ind.isValid() && outd.isValid());
	
	// Assignment and unit conversion
	CHECK_THROW(ind = 5, PortException);
	outd = 1;
	CHECK_CLOSE(*ind,0.3048,0.0001);
	
	// Default value
	ins.setDefault("wassup");
	CHECK(ins.isValid());
	CHECK(ins.get() == "wassup");
	
	
	// Exceptions when assigning inputs, connecting incompatible ports etc.
	CHECK_THROW(ind = 4711, PortException);
	CHECK_THROW(ind.connect(&outs), PortException);
	CHECK_THROW(outs.connect(&ind), PortException);
	
	// Connection logic - inputs can only be connected to _one_ other input or output,
	//   outputs can be connected to several inputs
	InPort<int> in1, in2, in3;
	OutPort<int> out1, out2;
	CHECK_THROW(out1.connect(&out2), PortException); // output ports don't connect
	out1.connect(&in1);
	out1.connect(&in2);
	CHECK(in1.getOtherEnd() == &out1);
	in1.connect(&out2); // this disconnects the previous connection
	// check that the previous connection doesn't linger
	CHECK(in1.getOtherEnd() == &out2);
	for (int i = 0; i < out1.getNumConnections(); i++)
		CHECK(out1.getOtherEnd(i) != &in1);
	// An input port can "piggyback" another input port, to "pass on the data"
	in3.connect(&in1);
	out2 = 4711;
	in2.connect(&out2); // connect after assignment
	CHECK(*in1 == 4711);
	CHECK(*in2 == 4711);
	CHECK(*in3 == 4711);
	in1.disconnect(&out2);
	CHECK_THROW(*in3, PortException); // in3 should consider connected but invalid
	CHECK(in3.isConnected() && !in3.isValid());
	
	// Try to connect incompatible unit ports
	InUnitPort<double> ind2("radian/slug^2");
	CHECK_THROW(ind2.connect(&outd), units::UnitsException);
	outd.connect(&ind2);
}
