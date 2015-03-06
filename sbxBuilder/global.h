#ifndef SBXBUILDER_GLOBAL_H
#define SBXBUILDER_GLOBAL_H

#include <string>
#include <FL/Fl_Menu_.H>

namespace sbxBuilder {
	
	class BlockCanvasWindow;
	class SimulationEditor;
	class ModelEditor;
	class StatisticsWindow;
	
	extern bool newFile();
	extern bool loadFile();
	extern bool saveFile();
	extern bool saveAs();
	extern bool saveChoice();
	extern void load(const std::string& filename);
	extern void save(const std::string& filename);
	extern void change();
	
	extern void makeNewModelsMenu(Fl_Menu_ *menu, Fl_Callback *callback, void* data);
	extern bool newModelAction(const std::string& path, int x = 0, int y = 0);
	
	extern BlockCanvasWindow *canvas_window;
	extern SimulationEditor *simulation_window;
	extern ModelEditor *model_window;
	extern StatisticsWindow *stats_window;
	extern std::string filename;
	extern bool changes_made;
	
}

#endif
