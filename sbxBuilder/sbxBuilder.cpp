#include "global.h"
#include "BlockCanvasWindow.h"
#include "SimulationEditor.h"
#include "ModelEditor.h"
#include "StatisticsWindow.h"
#include "Decorations.h"

#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Browser.H>
#include <FL/fl_ask.H>
#include <sbx/Model.h>
#include <sbx/Group.h>
#include <sbx/Ports.h>
#include <sbx/XMLParser.h>
#include <sbx/ModelFactory.h>
#include <sbx/PluginManager.h>
#include <sbx/Log.h>
#include <stdio.h>

sbx::Timer simtimer;
double last_time = 0, last_stats_time = 0;

using namespace sbxBuilder;

void update_sim(void *data) {
	if (!simulation_window)
		return;
	SimulationState state = simulation_window->getRunControl()->getState();
	if (state == RUNNING) {
		try {
		sbx::Simulation *sim = simulation_window->getSimulation();
		if (sim->isRealTime())
			sim->update(simtimer.time_s() - last_time);
		else {
			while (simtimer.time_s() - last_time < 0.1)
				sim->step();
		}
		last_time = simtimer.time_s();
		simulation_window->update();
		if (simtimer.time_s() - last_stats_time > 1.0) {
			if (stats_window && stats_window->shown())
				stats_window->update();
			last_stats_time = simtimer.time_s();
		}
		} catch (std::exception& e) {
			fl_message("Simulation error:\n%s", e.what());
			simulation_window->getRunControl()->setState(STOPPED_ERROR);
		}
	} else {
		simtimer.setStartTick();
		last_time = 0;
		last_stats_time = 0;
	}
	Fl::repeat_timeout(0.001, update_sim);
}

int event_handler(int event) {
	switch(event) {
		case FL_SHORTCUT:
			if (Fl::event_key() == FL_Escape) { // disable quitting on ESC
				return 1;
			} else if (Fl::event_key() == 110) { // Ctrl-N
				newFile();
				return 1;
			} else if (Fl::event_key() == 111) { // Ctrl-O
				loadFile();
				return 1;
			} else if (Fl::event_key() == 115) { // Ctrl-S
				saveFile();
				return 1;
			} else if (Fl::event_key() == 113) { // Ctrl-Q
				if (!saveChoice())
					return 1;
				exit(0);
			} else if (Fl::event_key() == 114) { // Ctrl-R
				simulation_window->getRunControl()->setState(RUNNING);
				return 1;
			} else if (Fl::event_key() == 116) { // Ctrl-T
				simulation_window->getRunControl()->setState(STOPPED);
				return 1;
			} else if (Fl::event_key() == 105) { // Ctrl-I
				simulation_window->getRunControl()->setState(INITIALIZED);
				return 1;
			} else if (Fl::event_key() == 112) { // Ctrl-P
				simulation_window->getRunControl()->setPaused(!simulation_window->getSimulation()->isPaused());
				return 1;
			}
			std::cout << "shortcut " << Fl::event_key() << "\n";
			break;
	}
	if (event != 0)
		std::cout << "event " << event << "\n";
	return 0;
}

int args_callback(int argc, char **argv, int& i)
{
	return 0;
}

int main(int argc, char **argv) {
	fl_register_images();
	sbx::XMLParser::instance().getPath().add(".");

	Fl::scheme("plastic");
	// Load decorations
	std::string decorations_filename = sbx::XMLParser::instance().getPath().find("decorations.xml");
	if (decorations_filename.length() > 0)
		Decorations::instance().load(decorations_filename);
	else
		sbx::dout(1) << "No decorations\n";
	
	// Load plugins
	//sbx::PluginManager::instance().getPath().add(".");
	sbx::PluginManager::instance().getPath().add("../sbxPlugins");
	if (sbx::FilePath::dirname(argv[0]).length() > 1)
		sbx::PluginManager::instance().getPath().add(sbx::FilePath::dirname(argv[0]) + "/../sbxPlugins");
	sbx::PluginManager::instance().loadAllPlugins();
	
	canvas_window = new BlockCanvasWindow(100, 100, 600, 400);
	canvas_window->end();
    simulation_window = new SimulationEditor(50, 50, 300, 100);
    simulation_window->end();
    model_window = new ModelEditor(150, 150, 300, 150);
    model_window->end();
    model_window->hide();
	stats_window = new StatisticsWindow(200, 200, 500, 300);
	stats_window->end();
	stats_window->hide();

	Fl::add_handler(event_handler);
	Fl::add_timeout(0.001, update_sim);
	
	int argi = 1;
	Fl::args(argc, argv, argi, args_callback);
    canvas_window->show(argc, argv);
    simulation_window->show();

	while (argi < argc) {
		filename = argv[argi];
		load(filename);
		argi++;
	}

	try {
		int ret = Fl::run();
		return ret;
	} catch (std::exception& e) {
		fl_message("Fatal Error:\n%s", e.what());
		return 1;
	}
}
