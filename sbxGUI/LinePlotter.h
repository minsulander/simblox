#ifndef SBXGUI_LINEPLOTTER_H
#define SBXGUI_LINEPLOTTER_H

#include "LinePlot2D.h"
#include <sbx/MultiModels.h>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

namespace sbxGUI {
	
	class LinePlot2DWindow : public Fl_Gl_Window {
	public:
		LinePlot2DWindow(int x, int y, int w, int h, char *title = "");
		virtual void init();
		virtual void draw();
		void setPlot(LinePlot2D *newplot) { plot = newplot; }
		void setClearColor(const Eigen::Vector4f& color) { clearcolor = color; }
	private:
		LinePlot2D *plot;
		Eigen::Vector4f clearcolor;
	};
	
	class LinePlotter : public sbx::MIModel<double> {
	public:
		LinePlotter(const std::string& name = "LinePlotter");
		LinePlotter(const LinePlotter& source);
		META_Object(sbxGUI, LinePlotter);
		virtual void parseXML(const TiXmlElement *element);
		virtual void writeXML(TiXmlElement *element);
		virtual void configure();
		virtual void init();
		virtual void update(const double dt);
		virtual void display(const sbx::DisplayMode mode);
		virtual void show();
		virtual void hide();
		virtual const char* className() { return "LinePlotter"; }
		virtual const char* libraryName() { return "sbxGUI"; }
		virtual const char* description() const { return "Window showing a line plot for each input port"; }
		virtual const bool isEndPoint() { return true; }
		
		Fl_Window *getWindow() { return window; }
	protected:
		void createWindow();
		virtual ~LinePlotter();
		
		LinePlot2DWindow *window;
		std::vector<DataSet*> datasets;
		LinePlot2D plot;
		bool start_shown;
		double t;
		unsigned int max_points;
	};
	
}

#endif
