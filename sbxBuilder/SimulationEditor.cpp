#include "SimulationEditor.h"
#include "global.h"
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <units/units.h>

namespace sbxBuilder {
	
	SimulationEditor::SimulationEditor(int X, int Y, int W, int H, sbx::Simulation *simulation)
	:	Fl_Double_Window(X, Y, W, H, "Simulation")
	{
		runcontrol = new RunControlGroup(0, 0, W-25, 25);
		runcontrol->end();
		edit = new Fl_Button(W-25, 0, 25, 25, "@8DnArrow");
		edit->callback(buttonCB, (void*) this);
		editgrp = new Fl_Group(0, 25, W, H-25);
		
		endtime = new Fl_Input(65, 30, 50, 20, "End time");
		endtime->callback(changeCB, (void*) this);
		freq = new Fl_Input(65, 50, 50, 20, "Freq");
		freq->callback(changeCB, (void*) this);
		realtime = new Fl_Check_Button(120, 30, 20, 20, "Realtime");
		realtime->callback(changeCB, (void*) this);
		continuous = new Fl_Check_Button(120, 50, 20, 20, "Continuous display");
		continuous->callback(changeCB, (void*) this);
		traversal = new Fl_Choice(65, 70, 100, 20, "Traversal");
		traversal->add("Sequential");
		traversal->add("Dependent");
		traversal->add("Parallel");
		traversal->callback(changeCB, (void*) this);
		
		editgrp->box(FL_FLAT_BOX);
		editgrp->end();
		end();
		
		setSimulation(simulation);
	}
	
	void SimulationEditor::newSimulation(sbx::Group *group)
	{
		setSimulation(new sbx::Simulation(group));
	}
	
	void SimulationEditor::setSimulation(sbx::Simulation *newsimulation)
	{
		simulation = newsimulation;
		runcontrol->setSimulation(simulation.get());
		if (!simulation)
			return;
		simulation->configure();
		std::stringstream ss;
		ss << simulation->getEndTime() << " s";
		endtime->value(ss.str().c_str());
		ss.str("");
		ss << simulation->getFrequency() << " Hz";
		freq->value(ss.str().c_str());
		realtime->value(simulation->isRealTime());
		continuous->value(simulation->getContinuousDisplay());
		traversal->value(simulation->getTraversalMode());
	}
	
	void SimulationEditor::update()
	{
		if (!simulation)
			return;
		runcontrol->update();
	}
	
	bool SimulationEditor::isExpanded()
	{
		return (h() > runcontrol->h());
	}
	
	void SimulationEditor::expand(const bool flag)
	{
		if (flag) {
			editgrp->show();
			size(w(), runcontrol->h() + editgrp->h());
			edit->label("@8DnArrow");
		} else {
			editgrp->hide();
			size(w(), runcontrol->h());
			edit->label("@2DnArrow");
		}
	}
	
	void SimulationEditor::parseXML(const TiXmlElement *element)
	{
		expand(sbx::XMLParser::parseBoolean(element, "expanded", true, true));
		position(sbx::XMLParser::parseInt(element, "position", "", true, x(), "x"),
				 sbx::XMLParser::parseInt(element, "position", "", true, y(), "y"));
	}
	
	void SimulationEditor::writeXML(TiXmlElement *element)
	{
		sbx::XMLParser::setBoolean(element, "expanded", isExpanded());
		sbx::XMLParser::setInt(element, "position", x(), "x");
		sbx::XMLParser::setInt(element, "position", y(), "y");
	}
	
	void SimulationEditor::buttonCB(Fl_Widget *w, void *data)
	{
		SimulationEditor *simed = (SimulationEditor*) data;
		simed->buttonAction((Fl_Button*) w);
	}
	
	void SimulationEditor::buttonAction(Fl_Button *button)
	{
		if (button == edit)
			expand(!isExpanded());
	}
	
	void SimulationEditor::changeCB(Fl_Widget *w, void *data)
	{
		SimulationEditor *simed = (SimulationEditor*) data;
		simed->changeAction(w);
	}
	
	void SimulationEditor::changeAction(Fl_Widget *w)
	{
		if (w == endtime) {
			double val;
			std::string unit;
			std::stringstream ss(endtime->value());
			if (ss >> val) {
				if (ss >> unit)
					val = units::convert(val, unit, "seconds");
				simulation->setEndTime(val);
				sbx::dout(3) << "set endtime = " << val << "\n";
			}
		}
		if (w == freq) {
			double val;
			std::string unit;
			std::stringstream ss(freq->value());
			if (ss >> val) {
				if (ss >> unit)
					val = units::convert(val, unit, "Hz");
				simulation->setFrequency((int)val);
				sbx::dout(3) << "set frequency = " << val << "\n";
			}
		}
		if (w == realtime)
			simulation->setRealTime(realtime->value());
		if (w == continuous)
			simulation->setContinuousDisplay(continuous->value());
		if (w == traversal)
			simulation->setTraversalMode((sbx::TraversalMode)traversal->value());
		change();
	}
	
} // namespace
