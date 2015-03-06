#ifndef SBXBUILDER_SIMULATIONEDITOR_H
#define SBXBUILDER_SIMULATIONEDITOR_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <sbx/Simulation.h>
#include "RunControlGroup.h"

namespace sbxBuilder {
	
	class SimulationEditor : public Fl_Double_Window {
	public:
		SimulationEditor(int X, int Y, int W, int H, sbx::Simulation *simulation = NULL);
		void newSimulation(sbx::Group *group);
		void setSimulation(sbx::Simulation *simulation);
		sbx::Simulation* getSimulation() { return simulation.get(); }
		RunControlGroup* getRunControl() { return runcontrol; }
		void update();
		bool isExpanded();
		void expand(const bool flag);
		
		void parseXML(const TiXmlElement *element);
		void writeXML(TiXmlElement *element);
		
	protected:
		static void buttonCB(Fl_Widget *w, void *data);
		void buttonAction(Fl_Button *b);
		static void changeCB(Fl_Widget *w, void *data);
		void changeAction(Fl_Widget *w);
		
		RunControlGroup *runcontrol;
		Fl_Button *edit;
		Fl_Group *editgrp;
		Fl_Input *endtime, *freq;
		Fl_Check_Button *realtime;
		Fl_Check_Button *continuous;
		Fl_Choice *traversal;
		smrt::ref_ptr<sbx::Simulation> simulation;
	};
	
}

#endif
