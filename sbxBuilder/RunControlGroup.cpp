#include "RunControlGroup.h"
#include "StatisticsWindow.h"
#include "global.h"
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <iostream>

namespace sbxBuilder {
	
	RunControlGroup::RunControlGroup(int X, int Y, int W, int H, sbx::Simulation *sim)
	:	Fl_Group(X,Y,W,H),
	simulation(sim),
	state(UNDEFINED),
	rratio_counter(0)
	{
		box(FL_FLAT_BOX);
		
		stop = new Fl_Button(X, Y, W/8, H, "@square");
		stop->labelsize(10);
		stop->box(FL_THIN_UP_BOX);
		stop->callback(buttonCB, this);
		
		init = new Fl_Button(X+W/8, Y, W/8, H, "I");
		init->labelfont(9);
		init->labelsize(16);
		init->box(FL_THIN_UP_BOX);
		init->callback(buttonCB, this);
		
		run = new Fl_Button(X+2*W/8, Y, W/8, H, "@>");
		run->box(FL_THIN_UP_BOX);
		run->callback(buttonCB, this);
		
		pause = new Fl_Button(X+3*W/8, Y, W/8, H, "@||");
		pause->box(FL_THIN_UP_BOX);
		pause->callback(buttonCB, this);
		
		{
			Fl_Group *group = new Fl_Group(W/2,0, W/2, H);
			group->box(FL_FLAT_BOX);
			tout = new Fl_Value_Output(W/2+15, 0, W/4-15, H, "T");
			tout->box(FL_FLAT_BOX);
			tout->textsize(12);
			tout->labelsize(12);
			tout->step(1);
			rtout = new Fl_Value_Output(W/2+W/4+25, 0, W/4-25, H, "RT");
			rtout->box(FL_FLAT_BOX);
			rtout->textsize(12);
			rtout->labelsize(12);
			group->end();
		}
		deactivate();
	}
	
	
	void RunControlGroup::update()
	{
		if (!simulation)
			return;
		
		tout->value(simulation->getTime());
		
		if (state == RUNNING && !simulation->isPaused() && ++rratio_counter >= 10) {
			rratio_counter = 0;
			double rratio = simulation->getAverageRealTimeRatio();
			if (rratio < 1) {
				rtout->precision(2);
				rtout->textcolor(FL_RED);
				rtout->labelcolor(FL_RED);
				rtout->textfont(FL_HELVETICA_BOLD);
				rtout->labelfont(FL_HELVETICA_BOLD);
			} else if (rratio < 5) {
				rtout->precision(1);
				rtout->textcolor(FL_YELLOW);
				rtout->labelcolor(FL_YELLOW);
				rtout->textfont(FL_HELVETICA_BOLD);
				rtout->labelfont(FL_HELVETICA_BOLD);
			} else {
				rtout->precision(0);
				rtout->textcolor(FL_BLACK);
				rtout->labelcolor(FL_BLACK);
				rtout->textfont(FL_HELVETICA);
				rtout->labelfont(FL_HELVETICA);
			}
			rtout->value(rratio);
		}
		
		if (simulation->isPaused())
			pause->color(FL_YELLOW);
		else
			pause->color(FL_GRAY);
		
		if (state == RUNNING && simulation->isDone())
			setState(STOPPED_DONE);
		redraw();
	}
	
	void RunControlGroup::setSimulation(sbx::Simulation *newsimulation)
	{
		simulation = newsimulation;
		if (simulation.valid()) {
			setState(STOPPED);
		}
	}
	
	void RunControlGroup::setState(const SimulationState newstate)
	{
		if (newstate == state)
			return;
		if (newstate != UNDEFINED && state == UNDEFINED)
			activate();
		else if (newstate == UNDEFINED && state != UNDEFINED)
			deactivate();
		if (newstate == INITIALIZED || (newstate >= INITIALIZED && state <= STOPPED)) {
			state = INITIALIZING;
			init->color(FL_YELLOW);
			try {
				simulation->init();
				state = INITIALIZED;
			} catch (std::exception& e) {
				fl_message("Failed to initialize simulation:\n%s", e.what());
				setState(STOPPED_ERROR);
				return;
			}
		}
		if (stats_window && (newstate == INITIALIZED || newstate == STOPPED || newstate == STOPPED_ERROR || newstate == STOPPED_DONE))
			stats_window->update();
		state = newstate;
		
		stop->color(FL_GRAY);
		init->color(FL_GRAY);
		run->color(FL_GRAY);
		switch (state) {
			case STOPPED_ERROR: stop->color(FL_RED); break;
			case STOPPED_DONE: stop->color(FL_GREEN); break;
			case STOPPED: stop->color(FL_BLUE); break;
			case INITIALIZED: init->color(FL_CYAN); break;
			case RUNNING: run->color(FL_GREEN); break;
		}
		stop->redraw();
		init->redraw();
		run->redraw();
		
		update();
	}
	
	void RunControlGroup::setPaused(const bool paused)
	{
		simulation->setPaused(paused);
		update();
		if (paused && simulation->getContinuousDisplay())
			simulation->display(sbx::DISPLAY_CONTINUOUS);
	}
	
	void RunControlGroup::buttonCB(Fl_Widget *w, void *data)
	{
		RunControlGroup *rcgroup = (RunControlGroup*) data;
		rcgroup->buttonAction((Fl_Button*) w);
	}
	
	void RunControlGroup::buttonAction(Fl_Button *button)
	{
		if (!simulation)
			return;
		if (button == stop) {
			setState(STOPPED);
		} else if (button == init) {
			setState(INITIALIZED);
		} else if (button == run) {
			rratio_counter = 0;
			setState(RUNNING);
		} else if (button == pause) {
			setPaused(!simulation->isPaused());
		}
	}
	
} // namespace
