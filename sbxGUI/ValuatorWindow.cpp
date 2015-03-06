#include "ValuatorWindow.h"

#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <sbx/Log.h>
#include <sbx/XMLParser.h>
#include <sbx/ModelFactory.h>

using namespace sbx;

namespace sbxGUI {
	
	REGISTER_Object(sbxGUI, ValuatorWindow);
	
#define SLIDER_WIDTH 200
#define SLIDER_HEIGHT 25
#define LABEL_HEIGHT 15
#define SLIDER_TYPE FL_HOR_SLIDER
	
	ValuatorWindow::ValuatorWindow(const std::string& name)
	:	sbx::MIMOModel<double>(name),
		window(NULL),
		start_shown(false),
		out_values(1,1)
	{
		window = new Fl_Window(0,0,2*SLIDER_WIDTH,SLIDER_HEIGHT+LABEL_HEIGHT);
		window->copy_label(getName().c_str());
		in_pack = new Fl_Pack(0, 0, SLIDER_WIDTH-5, window->h(), "Inputs");
		in_pack->spacing(LABEL_HEIGHT);
		in_pack->end();
		out_pack = new Fl_Pack(SLIDER_WIDTH+5, 0, SLIDER_WIDTH-5, window->h(), "Outputs");
		out_pack->spacing(LABEL_HEIGHT);
		out_pack->end();
		window->end();
		window->resizable(window);
	}
	
	void ValuatorWindow::parseXML(const TiXmlElement *element)
	{
		Model::parseXML(element);
		if (element->FirstChildElement("color")) {
			Eigen::Vector3d rgb = XMLParser::parseVector3(element,"color");
			window->color(fl_rgb_color((uchar)(rgb.x()*255), (uchar)(rgb.y()*255), (uchar)(rgb.z()*255)));
		}
		window->position(sbx::XMLParser::parseInt(element, "position", "", true, window->x(), "x"),
						 sbx::XMLParser::parseInt(element, "position", "", true, window->y(), "y"));
		window->size(sbx::XMLParser::parseInt(element, "size", "", true, window->w(), "w"),
					 sbx::XMLParser::parseInt(element, "size", "", true, window->h(), "h"));
		start_shown = sbx::XMLParser::parseBoolean(element, "visible", true, false);
		sbx::XMLParser::parseVector(out_values, element, "out_values");
	}
	
	void ValuatorWindow::writeXML(TiXmlElement *element)
	{
		sbx::XMLParser::setInt(element, "position", window->x(), "x");
		sbx::XMLParser::setInt(element, "position", window->y(), "y");
		sbx::XMLParser::setInt(element, "size", window->w(), "w");
		sbx::XMLParser::setInt(element, "size", window->h(), "h");
		if (window->visible())
			sbx::XMLParser::setBoolean(element, "visible", true);
		out_values.resize(1,out_sliders.size());
		for (size_t i = 0; i < out_sliders.size(); i++) {
			out_values[i] = out_sliders[i]->value();
		}
		sbx::XMLParser::setVector(element, "out_values", out_values);
		Model::writeXML(element);
	}
	
	void ValuatorWindow::configure()
	{
		if (start_shown)
			show();
	}
	
	void ValuatorWindow::display(const DisplayMode mode)
	{
		if (!window->visible())
			return;
		for (unsigned int i = 0; i < inputs.size(); i++) {
			if (in_sliders.size() < i+1) {
				in_sliders.push_back(new Fl_Value_Slider(0, 0, in_pack->w(), SLIDER_HEIGHT));
				in_sliders[i]->type(SLIDER_TYPE);
				if (inputs[i]->getOwner()->getPortName(inputs[i]).substr(0,2) != "in" || !inputs[i]->isConnected())
					in_sliders[i]->label(inputs[i]->getOwner()->getPortName(inputs[i]).c_str());
				else {
					std::stringstream ss;
					if (inputs[i]->getOtherEnd()->getOwner())
						ss << inputs[i]->getOtherEnd()->getOwner()->getName() << ".";
					ss << inputs[i]->getOtherEnd()->getOwner()->getPortName(inputs[i]->getOtherEnd());
					in_sliders[i]->copy_label(ss.str().c_str());
				}
				in_sliders[i]->set_output();
				in_pack->add(in_sliders[i]);
				resizeWindow();
				dout(3) << "ValuatorWindow add input " << in_sliders[i]->label() << "\n";
			}
			if (inputs[i]->isConnected()) {
				in_sliders[i]->activate();
				double value = inputs[i]->get();
				if (value < in_sliders[i]->minimum())
					in_sliders[i]->minimum(value);
				if (value > in_sliders[i]->maximum())
					in_sliders[i]->maximum(value);
				in_sliders[i]->value(value);
			} else
				in_sliders[i]->deactivate();
		}
		for (unsigned int i = 0; i < outputs.size(); i++) {
			if (out_sliders.size() < i+1) {
				out_sliders.push_back(new Fl_Value_Slider(0, 0, out_pack->w(), SLIDER_HEIGHT));
				out_sliders[i]->type(SLIDER_TYPE);
				out_sliders[i]->label(outputs[i]->getOwner()->getPortName(outputs[i]).c_str());
				if (out_values.size() > i)
					out_sliders[i]->value(out_values[i]);
				out_pack->add(out_sliders[i]);
				resizeWindow();
				dout(3) << "ValuatorWindow add output " << out_sliders[i]->label() << "\n";
			}
			if (outputs[i]->isConnected()) {
				out_sliders[i]->activate();
				outputs[i]->set(out_sliders[i]->value());
			} else
				out_sliders[i]->deactivate();
		}
	}
	
	void ValuatorWindow::show()
	{
		if (window) {
			window->show();
			window->copy_label(getName().c_str());
		}
		resizeWindow();
		display(sbx::DISPLAY_USER);
	}
	
	void ValuatorWindow::hide()
	{
		if (window)
			window->hide();
	}
	
	ValuatorWindow::~ValuatorWindow()
	{
		if (window)
			window->hide();
	}
	
	void ValuatorWindow::resizeWindow()
	{
		if (!window)
			return;
		int w, h;
		w = SLIDER_WIDTH;
		if (in_sliders.size() > 0 && out_sliders.size() > 0)
			w *= 2;
		if (in_sliders.size() > out_sliders.size())
			h = in_sliders.size() * (SLIDER_HEIGHT + LABEL_HEIGHT);
		else
			h = out_sliders.size() * (SLIDER_HEIGHT + LABEL_HEIGHT);
		if (window->w() > w)
			w = window->w();
		if (window->h() > h)
			h = window->h();
		window->size(w, h);
		dout(5) << "ValuatorWindow resize " << w << " x " << h << "\n";
	}
	
} // namespace sbxGUI
