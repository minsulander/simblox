#include "UtilityModels.h"
#include "ModelFactory.h"
#include <iostream>
#include <iomanip>

namespace sbx
{
	
	REGISTER_Object(sbx, PortDumper);
	REGISTER_Object(sbx, TimerModel);
	
	PortDumper::PortDumper(const std::string& name) :
		MIModel<double>(name),
		count(0),
		interval(0),
		streamptr(&std::cout)
	{
		registerParameter(&interval,Parameter::DOUBLE,"interval","second","Time interval between dump outputs");
	}
	
	PortDumper::PortDumper(const PortDumper& source) :
		MIModel<double>(source),
		count(source.count),
		interval(source.interval),
		streamptr(source.streamptr)
	{
		copyParameter(source, "interval", &interval);
	}
	
	
	void PortDumper::init() {
		count = 0;
		t = 0;
	}
	
	void PortDumper::update(const double dt)
	{
		t += dt;
		
	}
	
	void PortDumper::display(const DisplayMode mode)
	{
		if (widths.size() < inputs.size())
			widths.resize(inputs.size());
		if (count == 0) {
			// Output header
			for (InPortList::iterator i = inputs.begin(); i != inputs.end(); i++) {
				InPort<double>* port = *i;
				std::stringstream ss;
				if (!port->getOtherEnd() || !port->getOtherEnd()->getOwner())
					ss << "???";
				else
					ss << port->getOtherEnd()->getOwner()->getName() << "."
					<< port->getOtherEnd()->getOwner()->getPortName(port->getOtherEnd());
				widths[i-inputs.begin()] = ss.str().size();
				*streamptr << ss.str() << " ";
			}
			*streamptr << "\n";
			// Output data
			for (InPortList::iterator i = inputs.begin(); i != inputs.end(); i++) {
				InPort<double>* port = *i;
				*streamptr << std::setw(widths[i-inputs.begin()]);
				if (port->isConnected())
					*streamptr << port->get();
				else
					*streamptr << "(n/c)";
				if (mode == DISPLAY_INITIAL)
					*streamptr << " (initial)";
				*streamptr << "\n";
			}
			count++;
		} else if (mode != DISPLAY_CONTINUOUS || interval-t < interval/1000.0) {
			// Output data
			for (InPortList::iterator i = inputs.begin(); i != inputs.end(); i++) {
				InPort<double>* port = *i;
				*streamptr << std::setw(widths[i-inputs.begin()]);
				if (port->isConnected())
					*streamptr << port->get();
				else
					*streamptr << "(n/c)";
				if (mode == DISPLAY_FINAL)
					*streamptr << " (final)";
				*streamptr << "\n";
			}
			t = 0;
			count++;
		} 
	}
	
	
} // namespace sbx
