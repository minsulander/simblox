#include <UnitTest++/UnitTest++.h>
#include <sbx/Simulation.h>
#include <sbx/Ports.h>
#include <iostream>

using namespace sbx;

class CounterModel : public Model {
public:
	CounterModel(const std::string& name = "CounterModel") 
		: Model(name), counter(200), config(false) {}
	META_Object(test, CounterModel);
	virtual void configure() { config = true; }
	virtual void init() { counter = 0; }
	virtual void update(const double dt) { counter++; }
	virtual const char* description() const { return "counter"; }
	virtual const int getMinimumUpdateFrequency() { return 10; }
	virtual const bool isEndPoint() { return true; }
	
	const int getCount() { return counter; }
	const bool getConfig() { return config; }
protected:
	int counter;
	bool config;
};

class DepModel : public Model {
public:
	DepModel(const std::string& name = "DepModel") : Model(name)
	{
		endpoint = false;
		myorder = 0;
		registerPort(in,"in","","Input");
		registerPort(in2,"in2","","Input2");
		registerPort(out,"out","","Output");
	}
	META_Object(test, DepModel);
	virtual void init()
	{ 
		myorder = 0;
		Timer::sleep(1000);
	}
	virtual void update(const double dt)
	{ 
		out = *in;
		myorder = ++globorder;
		Timer::sleep(1000);
	}
	virtual const char* description() const { return "depmodel"; }
	void setEndPoint(const bool val) { endpoint = val; }
	virtual const bool isEndPoint() { return endpoint; }
	const int getOrder() { return myorder; }
	static int globorder;
protected:
	InPort<double> in, in2;
	OutPort<double> out;
	bool endpoint;
	int myorder;
};

int DepModel::globorder = 0;

TEST(SimulationAndModelVisitor) {
	UNITTEST_TIME_CONSTRAINT(1500);
	smrt::ref_ptr<Group> grp = new Group;
	smrt::ref_ptr<CounterModel> c1 = new CounterModel("c1");
	smrt::ref_ptr<CounterModel> c2 = new CounterModel("c2");
	grp->addChild(c1.get());
	grp->addChild(c2.get());
	CHECK_EQUAL(2, grp->getNumChildren());
	CHECK(grp->getChild(0) == c1.get());
	Simulation sim(grp.get());
	
	// Check update frequency division stuff
	c1->setUpdateFrequency(10);
	c2->setUpdateFrequency(20);
	sim.init();
	CHECK(c1->getConfig());
	CHECK(c2->getConfig());
	CHECK_EQUAL(0, c1->getCount());
	CHECK_EQUAL(0, c2->getCount());
	sim.update(0.2);
	CHECK_EQUAL(2, c1->getCount());
	CHECK_EQUAL(4, c2->getCount());
	
	// Re-initialization, check that step() advances one timestep
	sim.init();
	CHECK_EQUAL(0, c1->getCount());
	CHECK_EQUAL(0, c2->getCount());
	CHECK_EQUAL(20, sim.getFrequency());
	sim.step();
	CHECK_EQUAL(0, c1->getCount());
	CHECK_EQUAL(1, c2->getCount());
	CHECK_EQUAL(sim.getTimeStep(), sim.getTime());
	
	// Re-init and run() it this time..
	{
		UNITTEST_TIME_CONSTRAINT(120);
		sim.init();
		sim.setEndTime(10);
		sim.setRealTime(false);
		sim.run();
		CHECK_EQUAL(100, c1->getCount());
		CHECK_CLOSE(10, sim.getTime(), 0.001);
	}
	// run realtime
	{
		sbx::Timer t;
		UNITTEST_TIME_CONSTRAINT(1200);
		sim.setEndTime(11);
		sim.setRealTime(true);
		sim.run();
		CHECK(t.time_s() > 0.99);
		CHECK_CLOSE(11, sim.getTime(), 0.001);
	}
	// When paused, time should not increment and models should not be updated
	sim.init();
	sim.setPaused(true);
	sim.update(1);
	sim.step();
	CHECK_EQUAL(0, sim.getTime());
	CHECK_EQUAL(0, c1->getCount());
	CHECK_EQUAL(0, c2->getCount());
	sim.setPaused(false);
	
	// Check singleton behaviour and multiple instances
	CHECK(Simulation::instance() == &sim);
	Simulation sim2;
	CHECK(Simulation::instance() == NULL);

	// Default traversal mode is DEPENDENT - models should be updated in an order that
	// corresponds to their port connections
	smrt::ref_ptr<DepModel> d1 = new DepModel("d1");
	smrt::ref_ptr<DepModel> d2 = new DepModel("d2");
	smrt::ref_ptr<DepModel> d3 = new DepModel("d3");
	grp->addChild(d1.get());
	grp->addChild(d2.get());
	grp->addChild(d3.get());
	// set up a dependency chain
	// d3 --> d1 --> d2
	d3->getPort("out")->connect(d1->getPort("in"));
	d2->getPort("in")->connect(d1->getPort("out"));
	d2->setEndPoint(true);
	OutPort<double> endport;
	endport = 4711;
	d3->getPort("in")->connect(&endport);
	sim.step();
	CHECK(d1->getOrder() == 2 && d2->getOrder() == 3 && d3->getOrder() == 1);
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << "\n";
	
	// If no model is an EndPoint, no traversal down that dependency tree will be done
	DepModel::globorder = 0;
	sim.init();
	d2->setEndPoint(false);
	sim.step();
	CHECK(d1->getOrder() == 0 && d2->getOrder() == 0 && d3->getOrder() == 0);
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << "\n";
	
	// Add another branch (with an endpoint) and see that the dependcy chain isn't broken by 
	// the lack of an endpoint down the other branch...
	smrt::observer_ptr<DepModel> d4 = new DepModel;
	grp->addChild(d4.get());
	// d3 --> d4
	// d1 --> d2
	d3->getPort("out")->connect(d4->getPort("in"));
	d4->setEndPoint(true);
	sim.init();
	sim.step();
	CHECK(d1->getOrder() == 0 && d2->getOrder() == 0 && d3->getOrder() == 1 && d4->getOrder() == 2);
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << " " << d4->getOrder() << "\n";
	
	// Change stuff "in the middle of the simulation" and see that it "re-routes"
	// d3 -----------> d4
	// d1 --> d2 --/
	DepModel::globorder = 0;
	d2->getPort("out")->connect(d4->getPort("in2"));
	sim.step();
	CHECK(d1->getOrder() < d2->getOrder() && d2->getOrder() < d4->getOrder() && d3->getOrder() < d4->getOrder());
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << " " << d4->getOrder() << "\n";

	// let's see if deletion on dereferencing works while we're at it...
	grp->removeChild(d4.get());
	CHECK(!d4.valid());

	// Now try SEQUENTIAL traversal mode
	DepModel::globorder = 0;
	d2->setEndPoint(true);
	sim.setTraversalMode(SEQUENTIAL);
	sim.init();
	sim.step();
	CHECK(d1->getOrder() == 1 && d2->getOrder() == 2 && d3->getOrder() == 3);
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << "\n";
	
	// No check for endpoints in sequential traversal mode, it's faster and dumber
	DepModel::globorder = 0;
	sim.init();
	d2->setEndPoint(false);
	sim.step();
	CHECK(d1->getOrder() == 1 && d2->getOrder() == 2 && d3->getOrder() == 3);
	//std::cout << d1->getOrder() << " " << d2->getOrder() << " " << d3->getOrder() << "\n";
	
	// Check that we get update time statistics
	sim.doStatistics();
	sim.init();
	sim.step();
	VisitorTimeMap initstats = sim.getInitVisitor().getStatistics();
	VisitorTimeMap updatestats = sim.getUpdateVisitor().getStatistics();
	CHECK(initstats[d1.get()] != 0);
	CHECK(updatestats[d1.get()] != 0);
}

TEST(FindVisitorAndNamingStuff) {
	smrt::ref_ptr<Group> root = new Group("root");
	smrt::ref_ptr<Group> grp = new Group("grp");
	smrt::ref_ptr<Group> grp2 = new Group("grp2");
	smrt::ref_ptr<CounterModel> model = new CounterModel("model");
	smrt::ref_ptr<CounterModel> model2 = new CounterModel("model");
	root->addChild(grp.get());
	grp->addChild(model.get());
	root->addChild(grp2.get());
	grp2->addChild(model2.get());

	// adding two children with the same name to a group should not jive
	CHECK_THROW(grp->addChild(model2.get()), ModelException);
	
	FindVisitor finder;
	
	CHECK(!finder.findModel(*root.get(), "BLAH"));
	CHECK(!finder.findPort(*root.get(), "BLAH.in"));
	CHECK(!finder.findModel(*root.get(), "/grp/BLAH"));
	CHECK(!finder.findPort(*root.get(), "grp2/BLAH.out"));
	
	CHECK_EQUAL(model.get(), finder.findModel(*root.get(), "model"));
	CHECK_EQUAL(model2.get(), finder.findModel(*root.get(), "grp2/model"));
	CHECK_EQUAL(model2.get(), finder.findModel(*root.get(), "/grp2/model"));
	CHECK_EQUAL(model2.get(), finder.findModel(*root.get(), "root/grp2/model"));
	CHECK_EQUAL(model.get(), finder.findModel(*root.get(), "grp/model"));
	CHECK_EQUAL(model.get(), finder.findModel(*root.get(), "/grp/model"));
	CHECK_EQUAL(model.get(), finder.findModel(*root.get(), "root/grp/model"));
	
	CHECK_EQUAL(model.get(), finder.findModelByPath(*root.get(), "/grp/model"));
	CHECK_EQUAL(model2.get(), finder.findModelByPath(*root.get(), "/grp2/model"));
	
	CHECK_EQUAL(model->getPort("in"), finder.findPort(*root.get(), "grp/model.in"));
	CHECK_EQUAL(model2->getPort("out"), finder.findPort(*root.get(), "/grp2/model.out"));
}
