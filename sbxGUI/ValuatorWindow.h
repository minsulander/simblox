#ifndef SBXGUI_VALUATORWINDOW_H
#define SBXGUI_VALUATORWINDOW_H

#include <Eigen/Core>
#include <sbx/MultiModels.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Value_Slider.H>

namespace sbxGUI {

class ValuatorWindow : public sbx::MIMOModel<double> {
public:
	ValuatorWindow(const std::string& name = "ValuatorWindow");
	META_Object(sbxGUI, ValuatorWindow);
	virtual void parseXML(const TiXmlElement *element);
	virtual void writeXML(TiXmlElement *element);
	virtual void configure();
	virtual void display(const sbx::DisplayMode mode);
	virtual void show();
	virtual void hide();
	virtual const char* className() { return "ValuatorWindow"; }
	virtual const char* libraryName() { return "sbxGUI"; }
	virtual const char* description() const { return "Window showing input/output valuators"; }
	virtual const bool isEndPoint() { return true; }
	
	Fl_Window *getWindow() { return window; }
protected:
	virtual ~ValuatorWindow();
	void resizeWindow();
	
private:
	Fl_Window *window;
	Fl_Pack *in_pack, *out_pack;
	std::vector<Fl_Value_Slider*> in_sliders, out_sliders;
	Eigen::VectorXd out_values;
	bool start_shown;
};

}

#endif
