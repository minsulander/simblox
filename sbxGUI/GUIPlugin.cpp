#include "GUIPlugin.h"
#include <sbx/PluginManager.h>
#include <sbx/ModelFactory.h>
#include <sbx/Log.h>
#include <iomanip>
#include "ValuatorWindow.h"
#include "LinePlotter.h"

namespace sbxGUI {

GUIPlugin::GUIPlugin()
:	Plugin()
{
}

void GUIPlugin::parseXML(const TiXmlElement *element)
{
	/*
	std::string fltk_scheme = sbx::XMLParser::parseString(element,"scheme",true,"plastic");
	sbx::dout(2) << "  scheme " << fltk_scheme << "\n";
	Fl::scheme(fltk_scheme.c_str());
	 */
}

void GUIPlugin::registerModels()
{
	/*
	REGISTER_MODEL(sbxGUI::ValuatorWindow);
	REGISTER_MODEL(sbxGUI::LinePlotter);
	 */
}

void GUIPlugin::postUpdate(const double dt)
{
	/// \todo Fltk updates need to be done in a way that co-exists with the main loop in sbxBuilder
	//Fl::wait(0);
}

void GUIPlugin::pauseUpdate(const double dt)
{
	//Fl::wait(0);
}

REGISTER_PLUGIN(sbxGUI::GUIPlugin);

} // namespace sbxGUI
