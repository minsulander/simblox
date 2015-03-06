/*
 *  Simulation.h
 *  SimBlox
 *
 *  Created by Martin Insulander on 2008-04-19.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SIMULATION_H_
#define SIMULATION_H_

#include "Export.h"
#include "Group.h"
#include "ModelVisitor.h"
#include "Timer.h"
#include <numerix/misc.h>
#include <math.h>

class TiXmlElement;

namespace sbx
{
	
	class SIMBLOX_API Simulation : public smrt::Referenced
	{
	public:
		Simulation(Group *root = NULL);
		virtual ~Simulation();
		
		virtual void configure();
		virtual void init();
		virtual double step();
		virtual double update(const double dt);
		virtual void run();
		
		virtual void parseXML(const TiXmlElement *element);
		virtual void writeXML(TiXmlElement *element);
		
		TraversalMode getTraversalMode() { return updatevis.getTraversalMode(); }
		void setTraversalMode(TraversalMode mode);
		double getEndTime() { return endtime; }
		void setEndTime(const double time) { endtime = time; }
		bool isRealTime() { return realtime; }
		void setRealTime(const bool value) { realtime = value; }
		
		double getTime() { return time; }
		void setTimeStep(const double dt);
		double getTimeStep() { return timestep; }
		void setFrequency(const int freq) { setTimeStep(1.0/freq); }
		int getFrequency() { return (int) round(1.0/getTimeStep()); }
		void setRoot(Group *group) { root = group; }
		Group *getRoot() { return root.get(); }
		double getRealTimeSinceStart() { return timer.time_s(); }
		double getRealTimeRatio() { return realtime_ratio; }
		double getAverageRealTimeRatio() { return average_rratio; }
		void setAverageRealTimeRatioSteps(unsigned long num) { rratio_num_steps = num; }
		void clearAverageRealTimeRatio() { sum_rratio = 0; rratio_count_steps = 0; average_rratio = 0; }
		
		bool isInitialized() { return initialized; }
		void setDone(const bool val) { done = val; }
		bool isDone() { return done; }
		void setPaused(const bool val) { paused = val; }
		bool isPaused() { return paused; }
		const ConfigureVisitor& getConfigureVisitor() const { return configurevis; }
		const InitVisitor& getInitVisitor() const { return initvis; }
		const UpdateVisitor& getUpdateVisitor() const { return updatevis; }
		const DisplayVisitor& getDisplayVisitor() const { return displayvis; }
		void doStatistics(const bool value = true);
		
		void setContinuousDisplay(const bool value) { continuous_display = value; }
		bool getContinuousDisplay() { return continuous_display; }
		void display(const DisplayMode mode);
		
		static Simulation *instance();
		static void resetInstance(Simulation *instanceptr);
	protected:
		double time, diff_time, timestep, maximum_timestep;
		smrt::ref_ptr<Group> root;
		ConfigureVisitor configurevis;
		InitVisitor initvis;
		UpdateVisitor updatevis;
		DisplayVisitor displayvis;
		Timer timer;
		bool configured, initialized, done, paused;
		double start_realtime;
		double realtime_ratio, sum_rratio, average_rratio;
		unsigned long rratio_count_steps, rratio_num_steps;
		double endtime;
		bool realtime;
		bool continuous_display;
		
		static bool multiple_instances;
		static Simulation *instanceptr;
	};
	
}

#endif
