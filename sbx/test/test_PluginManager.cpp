#include <UnitTest++/UnitTest++.h>
#include <sbx/PluginManager.h>
#include <sbx/ModelFactory.h>
#include <sbx/Simulation.h>

using namespace sbx;

namespace test {
	
	class PlugModel : public Model {
	public:
		PlugModel() : Model(), counter(0) {}
		META_Object(test, PlugModel);
		virtual void init() { counter = 0; }
		virtual void update(const double dt) { counter++; }
		virtual const char* description() const { return "counter"; }
		virtual const int getMinimumUpdateFrequency() { return 10; }
		virtual const bool isEndPoint() { return true; }
		
		const int getCount() { return counter; }
	protected:
		int counter;
	};
	
	class DummyPlugin : public Plugin {
	public:
		DummyPlugin() { clear(); }
		virtual ~DummyPlugin() { deleted = true; }
		void clear() { preinit = postinit = preupdate = postupdate = pauseupdate = deleted = false; }
		
		virtual const char* getName() { return "DummyPlugin"; }
		virtual const char* getDescription() { return "Dummy test plugin"; }
		virtual const char* getAuthor() { return "Martin Insulander"; }
		virtual const char* getLicense() { return "SimBlox"; }
		virtual const float getVersion() { return 4711; }
		
		virtual void registerModels() {  }
		virtual void preInitialize() { preinit = true; }
		virtual void postInitialize() { postinit = true; }
		virtual void preUpdate(const double timestep) { preupdate = true; }
		virtual void postUpdate(const double timestep) { postupdate = true; }
		virtual void pauseUpdate(const double timestep) { pauseupdate = true; }
		
		static bool preinit, postinit, preupdate, postupdate, pauseupdate, deleted;
	};
	bool DummyPlugin::preinit = false;
	bool DummyPlugin::postinit = false;
	bool DummyPlugin::preupdate = false;
	bool DummyPlugin::postupdate = false;
	bool DummyPlugin::pauseupdate = false;
	bool DummyPlugin::deleted = false;
	
	REGISTER_Object(test, PlugModel);
	
}

using namespace test;

TEST(PluginManager) {
	UNITTEST_TIME_CONSTRAINT(500);
	// Register the plugin, force Simulation to be singleton (since other tests may have messed with
	// multiple Simulations), and check that it registers and we can instantiate a model
	DummyPlugin *theplug = new DummyPlugin;
	PluginManager::instance().registerPlugin(theplug);
	CHECK(PluginManager::instance().getNumPlugins() == 1);
	CHECK(PluginManager::instance().getPlugin(0) == theplug);
	Simulation sim;
	Simulation::resetInstance(&sim);
	CHECK(Simulation::instance());
	smrt::ref_ptr<Group> root = new Group;
	root->addChild(ModelFactory::instance().create("test::PlugModel"));
	Simulation::instance()->setRoot(root.get());
	
	// When the simulation is singleton, the plugin preInitialize() etc methods should be called properly
	Simulation::instance()->init();
	CHECK(DummyPlugin::preinit);
	CHECK(DummyPlugin::postinit);
	CHECK(!DummyPlugin::preupdate);
	CHECK(!DummyPlugin::postupdate);
	Simulation::instance()->setRealTime(false);
	Simulation::instance()->update(1);
	CHECK(DummyPlugin::preinit);
	CHECK(DummyPlugin::postinit);
	CHECK(DummyPlugin::preupdate);
	CHECK(DummyPlugin::postupdate);
	CHECK(!DummyPlugin::pauseupdate);
	Simulation::instance()->setPaused(true);
	Simulation::instance()->update(1);
	CHECK(DummyPlugin::pauseupdate);
	
	/// \todo Load a dummy plugin from an actual file. This could be combined with a HelloWorld plugin.
	
	// Unloading should delete it...
	PluginManager::instance().unload();
	CHECK(DummyPlugin::deleted);
}
