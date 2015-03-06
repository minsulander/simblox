#ifndef PLUGIN_H
#define PLUGIN_H

#include "Export.h"
#include <exception>
#include <string>
#include "XMLParser.h"

namespace sbx
{

	class SIMBLOX_API Plugin
	{
	public:
		virtual ~Plugin() {}
		virtual const char* getName() = 0;
		virtual const char* getDescription() { return ""; }
		virtual const char* getAuthor() = 0;
		virtual const char* getLicense() { return "SimBlox"; }
		virtual const float getVersion() { return -1; }
		virtual void parseXML(const TiXmlElement *element) {}
		virtual void writeXML(TiXmlElement *element) {}
		
		virtual void registerModels() {}
		virtual void preInitialize() {}
		virtual void postInitialize() {}
		virtual void preUpdate(const double timestep) {}
		virtual void postUpdate(const double timestep) {}
		virtual void pauseUpdate(const double timestep) {}
	};

	class SIMBLOX_API PluginException : public std::exception {
	public:
		PluginException(const std::string& error, Plugin* plugin = NULL) { 
			this->plugin = plugin; this->error = "PluginException: " + error;
		}
		virtual ~PluginException() throw() {}
		virtual const char* what() const throw()
		{
			return error.c_str();
		}
		std::string error;
		Plugin *plugin;
	};
}

#define REGISTER_PLUGIN(name) \
	extern "C" { \
		void SIMBLOX_PLUGIN_API SimBlox_Plugin_Load() { \
			sbx::PluginManager::instance().registerPlugin(new name); \
		} \
	}

#endif
