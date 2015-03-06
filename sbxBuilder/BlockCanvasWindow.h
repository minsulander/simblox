#ifndef SBXBUILDER_BLOCKCANVASWINDOW_H
#define SBXBUILDER_BLOCKCANVASWINDOW_H

#include "BlockCanvas.h"
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Box.H>
#include <sbx/Group.h>
#include <string>

namespace sbxBuilder {

class BlockCanvasWindow : public Fl_Double_Window
{
public:
	BlockCanvasWindow(int X, int Y, int W, int H, sbx::Group *group = NULL);
	BlockCanvas* getRootCanvas() { return root_canvas; }
	BlockCanvas* getCurrentCanvas() { return current_canvas; }
	void updateTitle();
	void setCurrentCanvas(BlockCanvas *canvas);
	
	void parseXML(const TiXmlElement *element);
	void writeXML(TiXmlElement *element);

protected:
	static void menuCB(Fl_Widget *w, void *data);
	void menuAction(const std::string& pathname);
	static void closeCB(Fl_Widget *w, void *data);
	void closeAction();
	
	BlockCanvas *root_canvas, *current_canvas;
	Fl_Menu_Bar *menubar;
	Fl_Box *bannerbox;
};

}

#endif
