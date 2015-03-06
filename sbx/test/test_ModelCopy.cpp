#include <UnitTest++/UnitTest++.h>
#include <sbx/Model.h>
#include <sbx/Ports.h>
#include <sbx/XMLParser.h>

using namespace sbx;

class CopyModel : public Model {
public:
	CopyModel(const std::string& name = "CopyModel") : Model(name)
	{
		registerParameter(&parf, Parameter::FLOAT, "parfait", "kg", "Weight of parfait");
		registerPort(in1, "in1", "", "An input");
		registerPort(in2, "in2", "", "An input");
		registerPort(out1, "out1", "", "An output");
	}
	CopyModel(const CopyModel& source) : Model(source), parf(source.parf)
	{
		copyParameter(source, "parfait", &parf);
		copyPort(source, "in1", in1);
		copyPort(source, "in2", in2);
		copyPort(source, "out1", out1);
	}
	META_Object(test, CopyModel);
	virtual const char* description() const { return "A model for testing copying of models"; }
	
	float parf;
	InPort<float> in1, in2;
	OutPort<float> out1;
};

TEST(SimpleCopy) {
	CopyModel source;
	source.parf = 7.56;
	CopyModel dest(source);
	CHECK_EQUAL(source.parf, dest.parf);
	CHECK_EQUAL(source.getNumParameters(), dest.getNumParameters());
	CHECK_EQUAL(source.getParameterSpec("parfait").description, dest.getParameterSpec("parfait").description);
	CHECK_EQUAL(source.getNumPorts(), dest.getNumPorts());
	CHECK_EQUAL(source.getNumInputs(), dest.getNumInputs());
	CHECK_EQUAL(source.getPortDescription("in1"), dest.getPortDescription("in1"));
	CHECK_EQUAL(source.getNumOutputs(), dest.getNumOutputs());
	for (int i = 0; i < 3; i++)
		CHECK_EQUAL(source.getPortName(i), dest.getPortName(i));
}

