/*
 *  BasicModels.cpp
 *  SimBlox
 *
 *  Created by Martin Insulander on 2007-12-15.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "BasicModels.h"
#include "Log.h"
#include "ModelFactory.h"
#include <math.h>

namespace sbx
{
	
	REGISTER_Object(sbx, SignalGenerator);
	
	SignalGenerator::SignalGenerator(const std::string& name)
	:	Model(name)
	{
		waveform = CONSTANT;
		waveformstr = "constant";
		amplitude = frequency = phase = bias = 0.0;
		registerPort(signal,"out","","Signal output");
		registerParameter(&waveformstr,Parameter::STRING,"waveform","","Waveform type (sine/cosine/noise/ramp etc...)");
		registerParameter(&amplitude,Parameter::DOUBLE,"amplitude","","Signal amplitude");
		registerParameter(&frequency,Parameter::DOUBLE,"frequency","radian/sec","Signal frequency");
		registerParameter(&phase,Parameter::DOUBLE,"phase","radian","Signal phase");
		registerParameter(&bias,Parameter::DOUBLE,"bias","","Signal bias");
	}
	
	SignalGenerator::SignalGenerator(const SignalGenerator& source) :
	Model(source),
	waveform(source.waveform),
	waveformstr(source.waveformstr),
	amplitude(source.amplitude),
	frequency(source.frequency),
	phase(source.phase),
	bias(source.bias),
	t(source.t)
	{
		signal = *source.signal;
		copyPort(source, "out", signal);
		copyParameter(source, "waveform", &waveformstr);
		copyParameter(source, "amplitude", &amplitude);
		copyParameter(source, "frequency", &frequency);
		copyParameter(source, "phase", &phase);
		copyParameter(source, "bias", &bias);
	}
	
	
	void SignalGenerator::init()
	{
		t = 0.0;
		switch (waveform) {
			case CONSTANT:
			case COSINE:
				signal = amplitude;
				break;
			case SINE:
				signal = 0; 
				break;
			default:
				signal = 0.0;
				setError("Unknown waveform");
				/// \todo more waveforms
		}
	}
	
	void SignalGenerator::update(const double dt)
	{
		t += dt;
		switch (waveform) {
			case CONSTANT: signal = amplitude; break;
			case SINE: signal = amplitude*sin(frequency*t+phase)+bias; break;
			case COSINE: signal = amplitude*cos(frequency*t+phase)+bias; break;
			default:
				signal = 0.0;
				setError("Unknown waveform");
		}
	}
	
	void SignalGenerator::configure()
	{
		if (waveformstr == "sine")
			waveform = SINE;
		else if (waveformstr == "cosine")
			waveform = COSINE;
		else if (waveformstr == "constant")
			waveform = CONSTANT;
		else
			throw ModelException("Invalid waveform '" + waveformstr + "' specified");
		dout(4) << "  waveform #" << waveform << "\n";
	}
	
} // namespace OSDF
