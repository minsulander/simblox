#include "ModelVisitor.h"
#include "Model.h"
#include "Group.h"
#include "Log.h"
#include "Timer.h"
#include <numerix/misc.h>
#include <math.h>

namespace sbx
{
	
	ModelVisitor::ModelVisitor()
	:	traversalmode(DEPENDENT),
	visitcount(0)
	{ 
	}
	
	void ModelVisitor::visit(Model& model)
	{
		if (traversalmode == PARALLEL) {
			if (!pool) {
				pool = new TaskThreadPool(NUM_VISITOR_TASKTHREADS);
				pool->start();
			}
			pool->setInhibit(true);
		}
		visited.clear();
		model.accept(*this);
		if (traversalmode == PARALLEL) {
			pool->setInhibit(false);
			pool->wait();
		}
		visitcount++;
	}
	
	void ModelVisitor::apply(Group& group)
	{
		if (visited[&group])
			return;
		group.traverse(*this);
		apply((Model&)group);
		visited[&group] = true;
	}
	
	void ModelVisitor::reset()
	{
		visitcount = 0;
	}
	
	TaskThreadPool* ModelVisitor::pool = NULL;
	
	ConfigureVisitor::ConfigureVisitor()
	: ModelVisitor()
	{
		traversalmode = SEQUENTIAL;
	}
	
	void ConfigureVisitor::apply(Model& model)
	{
		if (!visited[&model])
			model.configure();
		visited[&model] = true;
	}
	
	class InitTask : public Task {
	public:
		InitTask(Model& nmodel) : model(nmodel) {}
		virtual void perform() { model.init(); }
		Model& model;
	};
	
	InitVisitor::InitVisitor()
	: ModelVisitor(), minimum_frequency(-1), requested_frequency(-1), dostats(false)
	{
	}
	
	void InitVisitor::visit(Model& model)
	{
		Timer timer;
		stats.clear();
		ModelVisitor::visit(model);
		totaltime += timer.time_s();
	}
	
	void InitVisitor::apply(Model& model)
	{
		if (visited[&model])
			return;
		if (traversalmode == DEPENDENT)
			model.traverse(*this);
		Timer timer;
		if (model.getMinimumUpdateFrequency() > minimum_frequency)
			minimum_frequency = model.getMinimumUpdateFrequency();
		if (model.getUpdateFrequency() > requested_frequency)
			requested_frequency = model.getUpdateFrequency();
		dout(4) << "init " << model.getName();
		if (model.getUpdateFrequency())
			dout(4) << " " << model.getUpdateFrequency() << " Hz";
		dout(4) << "\n";

		if (traversalmode == PARALLEL)
			pool->schedule(new InitTask(model));
		else
			model.init();
		visited[&model] = true;
		if (dostats)
			stats[&model] += timer.time_s();
	}
	
	void InitVisitor::apply(Group& group)
	{
		if (visited[&group])
			return;
		Timer timer;
		dout(4) << "init traverse " << group.getName() << "\n";
		group.traverse(*this);
		dout(4) << "init " << group.getName() << "\n";
		group.init();
		visited[&group] = true;
		if (dostats)
			stats[&group] += timer.time_s();
	}
	
	void InitVisitor::reset()
	{
		ModelVisitor::reset();
		stats.clear();
		totaltime = 0;
	}
	
	class UpdateTask : public Task {
	public:
		UpdateTask(Model& nmodel, const double ndt) : model(nmodel), dt(ndt) {}
		virtual void perform() { model.update(dt); }
		Model& model;
		double dt;
	};
	
	UpdateVisitor::UpdateVisitor(const double dt)
	:	ModelVisitor(),
		dostats(false)
	{
		frequency = (int) round(1.0/dt);
	}
	
	void UpdateVisitor::visit(Model& model)
	{
		Timer timer;
		stats.clear();
		ModelVisitor::visit(model);
		totaltime += timer.time_s();
	}
	
	void UpdateVisitor::setTimeStep(const double dt)
	{
		frequency = (int) round(1.0/dt);
	}
	
	void UpdateVisitor::apply(Model& model)
	{
		if (visited[&model])
			return;
		if (traversalmode == DEPENDENT)
			model.traverse(*this);
		Timer timer;
		if (model.getUpdateFrequency() == 0 || model.getUpdateFrequency() == frequency) {
			//if (!visitcount)
			//	dout(4) << "update " << model.getName() << " at " << frequency << " Hz\n";
			// Model update frequency == 0 means the model doesn't specify what frequency it should be updated at
			update(model, 1.0/frequency);
		} else if (model.getUpdateFrequency() > frequency) {
			// Check if things are even...
			double fratio = (double) model.getUpdateFrequency() / frequency;
			int iratio = (int)round(fratio);
			if (fratio != iratio) {
				/// \todo Be more forgiving about update frequencies...
				std::stringstream ss;
				ss << "Uneven update frequencies - model wants " << model.getUpdateFrequency() << " Hz, simulation is at " << frequency << " Hz";
				throw ModelException(ss.str(), &model);
			}
			//if (!visitcount)
			//	dout(4) << "update " << model.getName() << " at " << (int) frequency*fratio << " Hz\n";
			// Model wants higher frequency - run repeated updates
			for (int i = 0; i < iratio; i++) {
				update(model, 1.0/(frequency*iratio));
			}
		} else {
			double fratio = (double) frequency / model.getUpdateFrequency();
			int iratio = (int)round(fratio);
			//if (!visitcount)
			//	dout(4) << "update " << model.getName() << " at " << (int) frequency/fratio << " Hz\n";
			// Model wants lower frequency - only update when needed
			if (iratio == 1 || (visitcount+1) % iratio == 0)
				update(model, 1.0/(frequency/iratio));
		}
		visited[&model] = true;
		if (dostats)
			stats[&model] += timer.time_s();
	}
	
	void UpdateVisitor::apply(Group& group)
	{
		if (visited[&group])
			return;
		Timer timer;
		group.traverse(*this);
		/// \todo frequency dependant update in Group just as in Model
		update(group, 1.0/frequency);
		visited[&group] = true;
		if (dostats)
			stats[&group] += timer.time_s();
	}
	
	void UpdateVisitor::update(Model& model, const double dt)
	{
		if (traversalmode == PARALLEL)
			pool->schedule(new UpdateTask(model, dt));
		else
			model.update(dt);
	}
	
	void UpdateVisitor::reset()
	{
		ModelVisitor::reset();
		stats.clear();
		totaltime = 0;
	}
	
	class DisplayTask : public Task {
	public:
		DisplayTask(Model& nmodel, const DisplayMode nmode) : model(nmodel), mode(nmode) {}
		virtual void perform() { model.display(mode); }
		Model& model;
		DisplayMode mode;
	};
	
	DisplayVisitor::DisplayVisitor()
	:	UpdateVisitor()
	{
		traversalmode = SEQUENTIAL;
	}
	
	void DisplayVisitor::visit(Model& model, const DisplayMode newmode)
	{
		mode = newmode;
		UpdateVisitor::visit(model);
	}
	
	void DisplayVisitor::apply(Model& model)
	{
		if (mode == DISPLAY_CONTINUOUS)
			UpdateVisitor::apply(model);
		else {
			Timer timer;
			model.display(mode);
			if (dostats)
				stats[&model] += timer.time_s();
		}
	}
	
	void DisplayVisitor::update(Model& model, const double dt)
	{
		if (traversalmode == PARALLEL)
			pool->schedule(new DisplayTask(model, mode));
		else
			model.display(mode);
	}
	
	FindVisitor::FindVisitor()
	:	ModelVisitor(),
	modelname(""),
	portname(""),
	modelptr(NULL),
	portptr(NULL)
	{
		traversalmode = SEQUENTIAL;
	}
	
	bool FindVisitor::modelNameMatches(const std::string& name, Model& model)
	{
		std::string::size_type lasti = name.length();
		std::string::size_type i = name.rfind("/", name.length());
		Model *pmodel = &model;
		while (i != std::string::npos) {
			std::string subname = name.substr(i+1, lasti-i-1);
			if (!pmodel || pmodel->getName() != subname)
				return false;
			pmodel = pmodel->getParent();
			if (i == 0) {
				// name starting with "/", parent should be root (have no parent)
				if (!pmodel || pmodel->getParent())
					return false;
				return true;
			}
			lasti = i;
			i = name.rfind("/", i-1);
		}
		if (!pmodel || pmodel->getName() != name.substr(0, lasti))
			return false;
		return true;
	}
	
	Model* FindVisitor::findModelByPath(Group& root, const std::string& path)
	{
		if (path[0] != '/')
			return NULL;
		Group *pgroup = &root;
		std::string::size_type lasti = 0;
		for (std::string::size_type i = path.find("/",1); i != std::string::npos; i = path.find("/",i+1)) {
			std::string childname = path.substr(lasti+1, i-lasti-1);
			for (unsigned int j = 0; j < pgroup->getNumChildren(); j++)
				if (pgroup->getChild(j)->getName() == childname)
					pgroup = pgroup->getChild(j)->asGroup();
			if (!pgroup)
				return NULL;
			lasti = i;
		}
		for (unsigned int j = 0; j < pgroup->getNumChildren(); j++)
			if (pgroup->getChild(j)->getName() == path.substr(lasti+1))
				return pgroup->getChild(j);
		return NULL;
	}
	
	Model* FindVisitor::findModel(Group& root, const std::string& name)
	{
		visited.clear();
		modelname = name;
		modelptr = NULL;
		if (name[0] == '/')
			return findModelByPath(root, name);
		root.accept(*this);
		return modelptr; 	
	}
	
	Port* FindVisitor::findPort(Group& root, const std::string& name)
	{
		visited.clear();
		modelname = "";
		portname = "";
		modelptr = 0;
		portptr = 0;
		int sepindex = name.find('.',0);
		if (sepindex != std::string::npos) {
			modelname = name.substr(0,sepindex);
			portname = name.substr(sepindex+1);
			if (modelname[0] == '/') {
				Model *absPathModel = findModelByPath(root, modelname);
				if (absPathModel) {
					apply(*absPathModel);
					return portptr;
				}
			}
			root.accept(*this);
		} else {
			modelname = root.getName();
			portname = name;
			apply((Model&)root);
		}
		return portptr; 	
	}
	
	void FindVisitor::apply(Model& model)
	{
		if (!modelptr && modelNameMatches(modelname, model)) {
			modelptr = &model;
			if (portname.length() > 0) {
				// Add a port if portname '+' is specified
				if (portname == "in+" && modelptr)
					portptr = model.addInput();
				else if (portname == "out+" && modelptr)
					portptr = model.addOutput();
				else
					portptr = model.getPort(portname);
			}
		}
		visited[&model] = true;
	}	
	
	CallbackVisitor::CallbackVisitor(VisitorCallback *callback)
	{
		this->callback = callback;
	}
	
	CallbackVisitor::~CallbackVisitor()
	{
		delete callback;
	}
	
	void CallbackVisitor::apply(Model& model)
	{
		if (callback)
			callback->apply(model);
		visited[&model] = true;
	}
	
	void CallbackVisitor::apply(Group& group)
	{
		if (callback)
			callback->preTraverse(group);
		group.traverse(*this);
		if (callback)
			callback->apply(group);
		visited[&group] = true;
	}
	
} // namespace sbx
