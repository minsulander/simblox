#ifndef SBXBUILDER_RUNCONTROLGROUP_H
#define SBXBUILDER_RUNCONTROLGROUP_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Value_Output.H>
#include <sbx/Simulation.h>

namespace sbxBuilder {
	
	enum SimulationState {
		UNDEFINED,
		STOPPED_ERROR,
		STOPPED_DONE,
		STOPPED,
		INITIALIZING,
		INITIALIZED,
		RUNNING
	};
	
	class RunControlGroup : public Fl_Group {
	public:
		RunControlGroup(int X, int Y, int W, int H, sbx::Simulation *simulation = NULL);
		void update();
		void setSimulation(sbx::Simulation *newsimulation);
		SimulationState getState() { return state; }
		void setState(const SimulationState state);
		void setPaused(const bool paused);
		
	protected:
		static void buttonCB(Fl_Widget *w, void *data);
		void buttonAction(Fl_Button *b);
		
		Fl_Button *stop, *init, *run, *pause;
		Fl_Value_Output *tout, *rtout;
		
		smrt::ref_ptr<sbx::Simulation> simulation;
		SimulationState state;
		unsigned int rratio_counter;
	};
	
}

#endif
