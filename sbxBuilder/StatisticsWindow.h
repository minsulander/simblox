#ifndef SBXBUILDER_STATISTICSWINDOW_H
#define SBXBUILDER_STATISTICSWINDOW_H


#include "BlockCanvas.h"
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Browser.H>

namespace sbxBuilder {
	
	class StatisticsWindow : public Fl_Double_Window {
	public:
		StatisticsWindow(int X, int Y, int W, int H);
		void update();
		
		void parseXML(const TiXmlElement *element);
		void writeXML(TiXmlElement *element);
	private:
		Fl_Browser *browser;
	};
	
}

#endif
