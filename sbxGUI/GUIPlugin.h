#ifndef GUIPLUGIN_H
#define GUIPLUGIN_H

#include <sbx/Export.h>
#include <sbx/Plugin.h>
#include <string>

namespace sbxGUI {

	class SIMBLOX_PLUGIN_API GUIPlugin : public sbx::Plugin {
	public:
		GUIPlugin();
		
		virtual const char* getName() { return "sbxGUI"; }
		virtual const char* getDescription() { return "Fltk-based GUI for simulation control etc."; }
		virtual const char* getAuthor() { return "Martin Insulander"; }
		virtual const char* getLicense() { return "SimBlox"; }
		virtual const float getVersion() { return 0.1; }
		virtual void parseXML(const TiXmlElement *element);
		
		virtual void registerModels();
		virtual void postUpdate(const double dt);
		virtual void pauseUpdate(const double dt);
	};
}

#endif
