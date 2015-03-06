#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "Export.h"
#include "Plugin.h"
#include "FilePath.h"
#include <string>
#include <vector>
#include <map>

namespace sbx
{
	class SIMBLOX_API PluginManager {
	public:
		void init();
		
		FilePath& getPath() { return path; }
		
		void loadPlugin(const std::string& name);
		void loadAllPlugins();
		void registerPlugin(Plugin *theplugin);
		void unload();
		
		const unsigned int getNumPlugins() { return plugins.size(); }
		Plugin *getPlugin(const unsigned int index) { return plugins[index]; }
		Plugin *getPlugin(const std::string& name);
		bool hasLoaded(const std::string& name);
		bool hasLoadedAll();
		
		void preInitialize();
		void postInitialize();
		void preUpdate(const double dt);
		void postUpdate(const double dt);
		void pauseUpdate(const double dt);
	
		static PluginManager& instance();
		
	protected:
		PluginManager();

		typedef std::vector<Plugin*> PluginList;
		PluginList plugins;
		typedef std::map<std::string, void*> LibraryList;
		LibraryList libraries;
		FilePath path;
		bool loaded_all;

		static PluginManager *instanceptr;
	};
}

#endif
