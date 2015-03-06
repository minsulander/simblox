/*
 *  UtilityModels.h
 *  SimBlox
 *
 *  Created by Martin Insulander on 2007-12-16.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef UTILITYMODELS_H
#define UTILITYMODELS_H

#include "Model.h"
#include "MultiModels.h"

#include <iostream>

namespace sbx
{
	
class SIMBLOX_API PortDumper : public MIModel<double>
{
public:
	PortDumper(const std::string& name = "PortDumper");
	PortDumper(const PortDumper& source);
	META_Object(sbx, PortDumper);
	virtual void init();
	virtual void update(const double dt);
	virtual void display(const DisplayMode mode);
	virtual const char* className() { return "PortDumper"; }
	virtual const char* libraryName() { return "sbx"; }
	virtual const char* description() const { return "Simple utility logger"; }
	virtual const bool isEndPoint() { return true; }

	void setStream(std::ostream& stream) { streamptr = &stream; }
	
protected:
	std::ostream* streamptr;
	std::vector<int> widths;
	unsigned long count;
	double interval;
	double t;
};

class SIMBLOX_API TimerModel : public Model
{
public:
	TimerModel(const std::string& name = "Timer") : Model(name) { registerPort(t,"t","second","Simulation time"); }
	TimerModel(const TimerModel& source) : Model(source) { copyPort(source, "t", t); }
	META_Object(sbx, TimerModel);
	virtual void init() { t = 0; }
	virtual void update(const double dt) { t = *t + dt; }
	virtual const char* className() { return "TimerModel"; }
	virtual const char* libraryName() { return "sbx"; }
	virtual const char* description() const { return "Utility model providing time since simulation start"; }
protected:
	OutUnitPort<double> t;
};

}

#endif
