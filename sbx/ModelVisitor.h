#ifndef MODELVISITOR_H_
#define MODELVISITOR_H_

#include "Export.h"
#include "Model.h"
#include "Group.h"
#include "TaskThread.h"
#include <map>

namespace sbx
{
	
	#define NUM_VISITOR_TASKTHREADS 4
	
	enum TraversalMode { SEQUENTIAL, DEPENDENT, PARALLEL };
	
	typedef std::map<const Model*, bool> VisitedMap;
	
	class SIMBLOX_API ModelVisitor
	{
	public:
		ModelVisitor();
		virtual ~ModelVisitor() {}
		virtual void visit(Model& model);
		
		virtual void apply(Model& model)=0;
		virtual void apply(Group& group);
		
		void setTraversalMode(TraversalMode mode) { traversalmode = mode; }
		TraversalMode getTraversalMode() const { return traversalmode; }
		
		virtual void reset();
		unsigned long getVisitCount() const { return visitcount; }
		
	protected:
		VisitedMap visited;	
		TraversalMode traversalmode;
		unsigned long visitcount;
		static TaskThreadPool *pool;
	};
	
	class SIMBLOX_API ConfigureVisitor : public ModelVisitor
	{
	public:
		ConfigureVisitor();
		virtual void apply(Model& model);
	};
	
	typedef std::map<const Model*, double> VisitorTimeMap;
	
	class SIMBLOX_API InitVisitor : public ModelVisitor
	{
	public:
		InitVisitor();
		virtual void visit(Model& model);
		virtual void apply(Model& model);
		virtual void apply(Group& group);
		
		virtual const int getMinimumUpdateFrequency() const { return minimum_frequency; }
		virtual const int getRequestedUpdateFrequency() const { return requested_frequency; }
		
		virtual void reset();
		void doStatistics(const bool value = true) { dostats = value; }
		const VisitorTimeMap& getStatistics() const { return stats; }
		//double getStatistics(const Model* model) const { return stats[model]; }
		double getTotalTime() const { return totaltime; }
		
	protected:
		int minimum_frequency, requested_frequency;
		bool dostats;
		VisitorTimeMap stats;
		double totaltime;
	};
	
	class SIMBLOX_API UpdateVisitor : public ModelVisitor
	{
	public:
		UpdateVisitor(const double dt = 1.0/30);
		virtual void visit(Model& model);
		virtual void apply(Model& model);
		virtual void apply(Group& group);
		virtual void update(Model& model, const double dt);
		
		void setTimeStep(const double dt);
		double getTimeStep() const { return 1.0/frequency; }
		
		virtual void reset();
		void doStatistics(const bool value = true) { dostats = value; }
		const VisitorTimeMap& getStatistics() const { return stats; }
		//double getStatistics(const Model* model) const { return stats[model]; }
		double getTotalTime() const { return totaltime; }
	protected:
		int frequency;
		bool dostats;
		VisitorTimeMap stats;
		double totaltime;
	};
	
	class SIMBLOX_API DisplayVisitor : public UpdateVisitor
	{
	public:
		DisplayVisitor();
		virtual void visit(Model& model, const DisplayMode mode = DISPLAY_USER);
		virtual void apply(Model& model);
		virtual void update(Model& model, const double dt);
	protected:
		DisplayMode mode;
	};
	
	class SIMBLOX_API FindVisitor : public ModelVisitor
	{
	public:
		FindVisitor();
		bool modelNameMatches(const std::string& name, Model& model);
		Model* findModelByPath(Group& root, const std::string& path);
		Model* findModel(Group& root, const std::string& name);
		Port* findPort(Group& root, const std::string& name);
		
		virtual void apply(Model& model);
		
	protected:
		std::string modelname, portname;
		Model* modelptr;
		Port* portptr;
	};
	
	class SIMBLOX_API VisitorCallback
	{
	public:
		virtual void apply(Model& model) {};
		virtual void apply(Group& group) {};
		virtual void preTraverse(Group& group) {};
	};
	
	class SIMBLOX_API CallbackVisitor : public ModelVisitor
	{
	public:
		CallbackVisitor(VisitorCallback* callback);
		virtual ~CallbackVisitor();
		virtual void apply(Model& model);
		virtual void apply(Group& group);
		
	protected:
		VisitorCallback *callback;
	};
	
} // namespace sbx

#endif /*MODELVISITOR_H_*/
