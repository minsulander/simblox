#include <UnitTest++/UnitTest++.h>
#include <sbx/Model.h>
#include <sbx/Ports.h>
#include <sbx/XMLParser.h>

using namespace sbx;

class DummyModel : public Model {
public:
	DummyModel() : Model() {
		registerPort(out,"out","","Dummy output");
		registerPort(in,"in","","Dummy input");
		registerParameter(&paramstring,Parameter::STRING,"string","","String test");
		registerParameter(&paramdouble,Parameter::DOUBLE,"double","kg","Double test");
		thetime = 0;
		inited = false;
		count_con = count_dis = 0;
	}
	META_Object(test, DummyModel);
	virtual void init() { 
		inited = true;
		count_con = count_dis = 0;
	}

	virtual void update(const double dt) { 
		if (!inited)
			throw ModelException("I haven't been initialized",this);
		thetime += dt; 
		out = thetime*10;
	}
	virtual const char* description() const { return "dummy"; }
	virtual const int getMinimumUpdateFrequency() { return 30; }
	
	const double getTheTime() { return thetime; }
	const double getValueAtInput() { return in.get(); }
	
	virtual void onPortConnect(Port *port, Port *otherend) { count_con++; }
	virtual void onPortDisconnect(Port *port, Port *otherend) { count_dis++; }

	const int getConnectCount() { return count_con; }
	const int getDisconnectCount() { return count_dis; }
	
protected:
	OutPort<double> out;
	InPort<double> in;
	double thetime;
	bool inited;
	int count_con, count_dis;
	std::string paramstring;
	double paramdouble;
};

TEST(DummyModel) {
	UNITTEST_TIME_CONSTRAINT(500);
	DummyModel dummy1, dummy2;
	
	// Correct initialization - no parents, registered its ports, no warn/err flags, no dependencies
	CHECK(dummy1.getParent() == NULL);
	CHECK(dummy1.getNumPorts() == 2);
	CHECK(dummy1.getNumParameters() == 2);
	CHECK(dummy1.isOK());
	CHECK(!dummy1.hasDataDependants());
	CHECK(!dummy1.hasDataProviders());
	// Dynamic allocation of ports should not be possible by default
	CHECK(dummy1.addInput() == NULL);
	CHECK(dummy2.addOutput() == NULL);
	
	// Check correct parsing
	TiXmlElement element("DummyModel");
	element.SetAttribute("frequency",10);
	TiXmlElement *param = new TiXmlElement("string");
	param->SetAttribute("value","wassup");
	element.LinkEndChild(param);
	param = new TiXmlElement("double");
	param->SetAttribute("unit","lbs"); // test unit conversion lbs->kg while we're at it
	param->SetAttribute("value","103.86");
	element.LinkEndChild(param);
	// Check that an exception is thrown for frequency below minimum
	CHECK_THROW(dummy1.parseXML(&element), ModelException);
	element.SetAttribute("frequency",60);
	dummy1.parseXML(&element);
	std::string str;
	double dbl;
	dummy1.getParameter("string",str);
	dummy1.getParameter("double",dbl);
	// Check parameters and unit conversion
	CHECK(str == "wassup");
	CHECK_CLOSE(dbl,47.11,0.001);
	CHECK_THROW(dummy1.getParameter("string",dbl), ModelException); // illegal type
	CHECK_EQUAL("double",dummy1.getParameterName(0));
	CHECK_EQUAL("kg",dummy1.getParameterSpec("double").unit);
	
	// Check dependencies through ports
	dummy1.getPort("out")->connect(dummy2.getPort("in"));
	CHECK(dummy1.hasDataDependants());
	CHECK(!dummy1.hasDataProviders());
	CHECK(!dummy2.hasDataDependants());
	CHECK(dummy2.hasDataProviders());
	CHECK(dummy1.providesFor(&dummy2));
	CHECK(dummy2.dependsOn(&dummy1));
	CHECK(dummy1.getDataDependants().at(0) == &dummy2);
	CHECK(dummy2.getDataProviders().at(0) == &dummy1);
	/// \todo Check dependsLoosely/dependsStrongly or should those be deleted?
	
	// Check init/update behaviour as specified in the class
	CHECK_THROW(dummy1.update(0.1),ModelException);
	dummy1.init();
	dummy1.update(0.1);
	CHECK(dummy1.getTheTime() == 0.1);
	CHECK(dummy2.getValueAtInput() == 1);
}

TEST(PortNotifications) {

	DummyModel d1, d2;
	// Check all combinations of connections, make sure notifications to the model works
	d1.getPort("out")->connect(d2.getPort("in"));
	CHECK_EQUAL(1, d1.getConnectCount());
	CHECK_EQUAL(1, d2.getConnectCount());
	d1.getPort("out")->disconnect();
	CHECK_EQUAL(1, d1.getDisconnectCount());
	CHECK_EQUAL(1, d2.getDisconnectCount());
	
	d2.getPort("in")->connect(d1.getPort("out"));
	CHECK_EQUAL(2, d1.getConnectCount());
	CHECK_EQUAL(2, d2.getConnectCount());
	d2.getPort("in")->disconnect();
	CHECK_EQUAL(2, d1.getDisconnectCount());
	CHECK_EQUAL(2, d2.getDisconnectCount());
}
