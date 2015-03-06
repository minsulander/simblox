#include "PluginManager.h"
#include "Log.h"
#include <iostream>
#include <fstream>

#if defined(WIN32) && !defined(__CYGWIN__)
#include <io.h>
#include <windows.h>
#include <winbase.h>
#else
#include <dlfcn.h>
#endif

namespace sbx
{
	
	void PluginManager::loadPlugin(const std::string& name)
	{
		if (name.length() == 0)
			throw PluginException("Empty plugin name");
		if (name == "sbx")
			return; // built-in
		std::string fname;
		// Check if it's a complete filename
		std::ifstream ifs(name.c_str());
		if (ifs.good()) {
			ifs.close();
			fname = name;
		} else {
			std::string osname;
#if defined(WIN32)
			osname += name + ".dll";
#elif defined(__APPLE__)
			osname += std::string("lib") + name + ".dylib";
#else
			osname += std::string("lib") + name + ".so";
#endif
			fname = path.find(osname);
			if (fname.length() == 0) {
				std::stringstream ss;
				ss << osname << ": File not found in plugin path " << path;
				throw PluginException(ss.str());
			}
		}
		
		dout(3) << "Loading plugin library " << fname << "\n";
#if defined(WIN32) && !defined(__CYGWIN__)
		void *handle = LoadLibrary( fname.c_str() );
		if (!handle)
			throw PluginException(fname + ": Failed to load plugin");
#else
		void *handle = dlopen( fname.c_str(), RTLD_NOW | RTLD_GLOBAL);
		if (!handle)
			throw PluginException(fname + ": " + dlerror());
#endif
		void (*loadfunc)();
#if defined(WIN32) && !defined(__CYGWIN__)
		loadfunc = (void (*)())GetProcAddress((HMODULE)handle,"SimBlox_Plugin_Load");
#else
		loadfunc = (void (*)()) dlsym(handle,"SimBlox_Plugin_Load");
#endif
		if (!loadfunc)
			throw PluginException(fname + ": Failed to load plugin. Entry point 'SimBlox_Plugin_Load' not found.");
		(*loadfunc)();
		libraries[name] = handle;
	}
	
	void PluginManager::loadAllPlugins()
	{
		std::string ext;
#if defined(WIN32)
		ext = ".dll";
#elif defined(__APPLE__)
		ext = ".dylib";
#else
		ext = ".so";
#endif
		dout(2) << "Loading " << ext << " plugins from path " << path << "\n";
		FileList list = path.getFilesByExtension(ext);
		for (FileList::iterator i = list.begin(); i != list.end(); i++) {
			try {
				loadPlugin(*i);
			} catch (PluginException& e) {
				dout(3) << *i << ": " << e.what() << "\n";
			}
		}
		loaded_all = true;
	}
	
	void PluginManager::registerPlugin(Plugin *plugin)
	{
		dout(2) << "Registering plugin " << plugin->getName() << "\n";
		dout(3) << "  Version " << plugin->getVersion() << "\n";
		dout(3) << "  Description: " << plugin->getDescription() << "\n";
		dout(3) << "  Author: " << plugin->getAuthor() << "\n";
		plugin->registerModels();
		plugins.push_back(plugin);
	}
	
	void PluginManager::unload()
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			delete (*i);
		}
		plugins.clear();
		for (LibraryList::iterator i = libraries.begin(); i != libraries.end(); i++) {
#if defined(WIN32) && !defined(__CYGWIN__)
			FreeLibrary((HMODULE)i->second);
#else
			dlclose(i->second);
#endif
		}
		libraries.clear();
	}
	
	Plugin *PluginManager::getPlugin(const std::string& name)
	{
		for (int i = 0; i < getNumPlugins(); i++) {
			if (getPlugin(i) && name == getPlugin(i)->getName())
				return getPlugin(i);
		}
		return NULL;
	}
	
	bool PluginManager::hasLoaded(const std::string& name)
	{
		/// \todo Should "sbx" be a "plugin" (registered with registerDefaultModels()) just like all the rest?
		return (libraries.find(name) != libraries.end() || name == "sbx");
	}
	
	bool PluginManager::hasLoadedAll()
	{
		return loaded_all;
	}
	
	void PluginManager::preInitialize()
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			(*i)->preInitialize();
		}
	}
	
	void PluginManager::postInitialize()
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			(*i)->postInitialize();
		}
	}
	
	void PluginManager::preUpdate(const double dt)
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			(*i)->preUpdate(dt);
		}
	}
	
	void PluginManager::postUpdate(const double dt)
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			(*i)->postUpdate(dt);
		}
	}
	
	void PluginManager::pauseUpdate(const double dt)
	{
		for (PluginList::iterator i = plugins.begin(); i != plugins.end(); i++) {
			(*i)->pauseUpdate(dt);
		}
	}
	
	PluginManager::PluginManager()
	{
		path.addEnvironmentVariable("SIMBLOX_PLUGIN_PATH");
		path.addEnvironmentVariable("SIMBLOX_HOME", "/sbxPlugins");
		loaded_all = false;
	}
	
	PluginManager& PluginManager::instance()
	{
		if (!instanceptr)
			instanceptr = new PluginManager;
		return *instanceptr;
	}
	
	PluginManager *PluginManager::instanceptr = NULL;
	
} // namespace sbx
