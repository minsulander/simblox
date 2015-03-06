/*
 *  ModelFactory.cpp
 *  HPanel
 *
 *  Created by Martin Insulander on 2007-11-11.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ModelFactory.h"
#include "PluginManager.h"

namespace sbx
{

Model *ModelFactory::create(const std::string& name)
{
	std::string type = name;
	if (type.find("_") != std::string::npos)
		type = type.substr(0,type.find("_")) + "::" + type.substr(type.find("_")+1);
	else if (type.find("/") != std::string::npos)
		type = type.substr(0,type.find("/")) + "::" + type.substr(type.find("/")+1);
	//if (type.substr(0,5) == "sbx::")
	//	type = type.substr(5);
	if (!hasRegistered(type)) {
		// Figure out which plugin and load it on need (unless all plugins are loaded)
		PluginManager& plugman = PluginManager::instance();
		if (!plugman.hasLoadedAll()) {
			if (type.find("::") != std::string::npos) {
				std::string plugin = type.substr(0,type.find("::"));
				if (!plugman.hasLoaded(plugin))
					plugman.loadPlugin(plugin);
			}
		}
	}
	return Factory<std::string, Model>::create(type);
}

ModelFactory& ModelFactory::instance()
{
	if (!instanceptr)
		instanceptr = new ModelFactory;
	return *instanceptr;
}

ModelFactory *ModelFactory::instanceptr = NULL;

}
