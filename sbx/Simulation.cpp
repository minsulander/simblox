/*
 *  Simulation.cpp
 *  SimBlox
 *
 *  Created by Martin Insulander on 2008-04-19.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Simulation.h"
#include "ModelVisitor.h"
#include "PluginManager.h"
#include "XMLParser.h"
#include "Log.h"

namespace sbx
{
	
	Simulation::Simulation(Group *newroot) :
	time(0),
	diff_time(0),
	timestep(0),
	maximum_timestep(0),
	done(false),
	configured(false),
	initialized(false),
	paused(false),
	start_realtime(0),
	realtime_ratio(0),
	endtime(0),
	realtime(true),
	rratio_num_steps(0),
	average_rratio(0),
	continuous_display(true)
	{
		root = newroot;
		if (!instanceptr)
			instanceptr = this;
		else
			multiple_instances = true;
	}
	
	Simulation::~Simulation()
	{
		if (instanceptr == this)
			instanceptr = NULL;
	}
	
	void Simulation::configure()
	{
		configurevis.reset();
		if (root.valid())
			configurevis.visit(*root);
		configured = true;
		dout(1) << "Simulation configured\n";
	}
	
	void Simulation::init()
	{
		// Reset visitor counters etc
		initvis.reset();
		updatevis.reset();
		displayvis.reset();
		
		// Configure and initialize models
		if (!configured)
			configure();
		time = 0;
		if (!multiple_instances)
			PluginManager::instance().preInitialize();
		
		if (root.valid())
			initvis.visit(*root);
		maximum_timestep = 1.0/initvis.getMinimumUpdateFrequency();
		
		// Set time step based on model requested update frequencies
		done = false;
		initialized = true;	
		if (timestep <= 0) {
			if (initvis.getRequestedUpdateFrequency() <= 0)
				setTimeStep(updatevis.getTimeStep()); // set to default (30 Hz)
			else
				setTimeStep(1.0/initvis.getRequestedUpdateFrequency());
		} else
			setTimeStep(timestep); // see setTimeStep() - this checks for max timestep / min freq
		
		if (!multiple_instances)
			PluginManager::instance().postInitialize();
		
		dout(1) << "Simulation initialized - "
		<< "running " << (realtime ? "realtime " : "")
		<< "at " << 1.0/timestep << " Hz";
		if (endtime > 0)
			dout(1) << " for " << endtime << " seconds";
		if (paused)
			dout(1) << ", start paused";
		dout(1) << "\n";
		
		if (root.valid())
			displayvis.visit(*root, DISPLAY_INITIAL);
	}
	
	double Simulation::step()
	{
		if (done)
			return 0;
		if (!initialized)
			init();
		if (time <= 0) {
			timer.setStartTick();
			start_realtime = timer.time_s();
			sum_rratio = 0;
			rratio_count_steps = 0;
		}
		double step_start_time = timer.time_s();
		if (paused) {
			if (!multiple_instances)
				PluginManager::instance().pauseUpdate(timestep);
		} else {
			if (!multiple_instances)
				PluginManager::instance().preUpdate(timestep);
			if (root.valid()) {
				updatevis.visit(*root);
				if (continuous_display)
					displayvis.visit(*root, DISPLAY_CONTINUOUS);
			}
			if (!multiple_instances)
				PluginManager::instance().postUpdate(timestep);
			time += timestep;
			
			if (endtime > 0 && time+timestep/2 >= endtime) {
				if (root.valid())
					displayvis.visit(*root, DISPLAY_FINAL);
				dout(5) << "Simulation finished at t=" << time << " seconds\n";
				done = true;
			}
		}
		double step_time = timer.time_s() - step_start_time;
		if (!paused) {
			realtime_ratio = timestep/step_time;
			sum_rratio += realtime_ratio;
			rratio_count_steps++;
			if (rratio_num_steps == 0) {
				average_rratio = sum_rratio / rratio_count_steps;
			} else if (rratio_count_steps >= rratio_num_steps) {
				average_rratio = sum_rratio / rratio_count_steps;
				rratio_count_steps = 0;
				sum_rratio = 0;
			}
		}
		return step_time;
	}
	
	double Simulation::update(const double dt)
	{
		if (paused)
			return step();
		diff_time += dt;
		double step_time = 0;
		while (diff_time >= timestep) {
			step_time += step();
			diff_time -= timestep;
		}
		return step_time;
	}
	
	void Simulation::run()
	{
		done = false;
		if (!initialized)
			init();
		double makeup_time = 0;
		while (!done) {
			double start_time = timer.time_s();
			
			double step_time = update(timestep);
			
			// If realtime, pause until next timestep
			if (realtime) {
				while (step_time < timestep-makeup_time) {		
					Timer::sleep(10);
					step_time = timer.time_s()-start_time;
				}
				makeup_time += step_time-timestep;
			}
		}
		//dout(DEBUG) << "Sim time " << time << ", real time " << timer.time_s()-start_realtime 
		//			<< ", average realtime ratio " << (sum_rratio/steps) << "\n";
	}
	
	void Simulation::parseXML(const TiXmlElement *element)
	{
		std::string str = XMLParser::parseString(element,"traversal",true,"");
		if (str.length() > 0) {
			if (str == "dependent")
				setTraversalMode(DEPENDENT);
			else if (str == "sequential")
				setTraversalMode(SEQUENTIAL);
			else if (str == "parallel")
				setTraversalMode(PARALLEL);
			else
				throw ParseException("Unknown traversal mode '" + str + "'", element);
		}
		realtime = XMLParser::parseBoolean(element,"realtime",true,realtime);
		endtime = XMLParser::parseDouble(element,"endtime","second",true,endtime);
		double step = XMLParser::parseDouble(element,"step","second",true,0);
		int frequency = XMLParser::parseInt(element,"frequency","Hz",true,0);
		if (step && frequency)
			throw ParseException("Both 'step' and 'frequency' specified in Simulation", element);
		if (step)
			setTimeStep(step);
		if (frequency)
			setFrequency(frequency);
		paused = XMLParser::parseBoolean(element,"paused",true,paused);
		continuous_display = XMLParser::parseBoolean(element, "continuous_display", true, continuous_display);
		
		if (element->FirstChildElement("plugins")) {
			std::string addpath = XMLParser::parseStringAttribute(element->FirstChildElement("plugins"), "path", true, "");
			PluginManager::instance().getPath().addFront(addpath, FilePath::dirname(XMLParser::instance().getCurrentFile()));
			if (addpath.length() > 0)
				dout(5) << "New plugin path (inserted " << addpath << " relative to document " << FilePath::dirname(XMLParser::instance().getCurrentFile()) << "): " << PluginManager::instance().getPath() << "\n";
			for (const TiXmlElement *el = element->FirstChildElement("plugins")->FirstChildElement(); el; el = el->NextSiblingElement()) {
				if (!PluginManager::instance().hasLoaded(el->Value()))
					PluginManager::instance().loadPlugin(el->Value());
				Plugin *plugin = PluginManager::instance().getPlugin(el->Value());
				if (!plugin)
					throw PluginException(std::string(el->Value()) + " loaded but not registered");
				plugin->parseXML(el);
			}
		}
		/// \todo remove models here...
		if (element->FirstChildElement("models")) {
			dout(2) << "Root group\n";
			root = new Group;
			root->setName("Root");
			root->parseXML(element->FirstChildElement("models"));
			dout(3) << "  " << root->getNumChildren() << " children\n";
		}
	}
	
	void Simulation::writeXML(TiXmlElement *element)
	{
		std::string str;
		switch (initvis.getTraversalMode()) {
			case DEPENDENT: str = "dependent"; break;
			case SEQUENTIAL: str = "sequential"; break;
			case PARALLEL: str = "parallel"; break;
		}
		XMLParser::setString(element, "traversal", str);
		XMLParser::setBoolean(element, "realtime", realtime);
		if (endtime > 0) {
			XMLParser::setDouble(element, "endtime", endtime);
			XMLParser::setString(element, "endtime", "second", "unit");
		}
		if (timestep > 0) {
			XMLParser::setDouble(element, "step", timestep);
			XMLParser::setString(element, "step", "second", "unit");
		}
		if (paused)
			XMLParser::setBoolean(element, "paused", paused);
		XMLParser::setBoolean(element, "continuous_display", continuous_display);
		if (PluginManager::instance().getNumPlugins() > 0) {
			TiXmlElement *pluginselement = new TiXmlElement("plugins");
			for (int i = 0; i < PluginManager::instance().getNumPlugins(); i++) {
				Plugin *plugin = PluginManager::instance().getPlugin(i);
				TiXmlElement *pluginelement = new TiXmlElement(plugin->getName());
				plugin->writeXML(pluginelement);
				pluginselement->LinkEndChild(pluginelement);
			}
			element->LinkEndChild(pluginselement);
		}
	}
	
	void Simulation::setTraversalMode(TraversalMode mode)
	{
		//configurevis.setTraversalMode(mode);
		initvis.setTraversalMode(mode);
		updatevis.setTraversalMode(mode);
		//displayvis.setTraversalMode(mode);
	}
	
	void Simulation::setTimeStep(const double dt)
	{
		timestep = dt;
		if (initialized && timestep > maximum_timestep && maximum_timestep > 0) {
			dout(WARN) << "Specified update frequency (" << 1.0/dt << " Hz) below models minimum, raising it to " << 1.0/maximum_timestep << " Hz" << std::endl;
			timestep = maximum_timestep;
		}
		//rratio_num_steps = 1.0/timestep;
		updatevis.setTimeStep(dt);
		displayvis.setTimeStep(dt);
	}
	
	void Simulation::doStatistics(const bool value)
	{
		initvis.doStatistics(value);
		updatevis.doStatistics(value);
		displayvis.doStatistics(value);
	}
	
	void Simulation::display(const DisplayMode mode)
	{
		if (root.valid())
			displayvis.visit(*root, mode);
	}
	
	Simulation* Simulation::instance()
	{
		if (multiple_instances)
			return NULL;
		if (!instanceptr)
			instanceptr = new Simulation;
		return instanceptr;
	}
	
	void Simulation::resetInstance(Simulation *newinstanceptr)
	{
		instanceptr = newinstanceptr;
		multiple_instances = false;
	}
	
	Simulation *Simulation::instanceptr = NULL;
	bool Simulation::multiple_instances = false;
	
	
	
} // namespace sbx
