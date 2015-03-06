/*
 *  BasicModels.h
 *  SimBlox
 *
 *  Created by Martin Insulander on 2007-12-15.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef BASICMODELS_H
#define BASICMODELS_H

#include "Model.h"
#include "Ports.h"
#include <string>

namespace sbx
{
	
class SIMBLOX_API SignalGenerator : public Model
{
public:
	SignalGenerator(const std::string& name = "SignalGenerator");
	SignalGenerator(const SignalGenerator& source);
	META_Object(sbx, SignalGenerator);
	virtual void init();
	virtual void update(const double dt);
	virtual void configure();
	virtual const char* className() { return "SignalGenerator"; }
	virtual const char* libraryName() { return "sbx"; }
	virtual const char* description() const { return "General purpose signal generator"; }
	
	enum Waveform {
		CONSTANT,
		SINE,
		COSINE
	};
protected:
	Waveform waveform;
	std::string waveformstr;
	double amplitude, frequency, phase, bias;
	double t;
	OutPort<double> signal;
};

}

#endif
