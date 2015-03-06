#include "BlockCanvasWindow.h"
#include "ModelBlock.h"
#include "SimulationEditor.h"
#include "ModelEditor.h"
#include "StatisticsWindow.h"
#include "global.h"
#include <sbx/ModelFactory.h>
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Browser.H>
#include <string>
#include <vector>
#include <iostream>

namespace sbxBuilder {
	
	BlockCanvasWindow::BlockCanvasWindow(int X, int Y, int W, int H, sbx::Group *group)
	:	Fl_Double_Window(X, Y, W, H),
	bannerbox(NULL)
	{
		menubar = new Fl_Menu_Bar(0,0,W,20);
		menubar->add("File/New", "^n", menuCB, (void*)this);
		menubar->add("File/Open", "^o", menuCB, (void*)this);
		menubar->add("File/Save", "^s", menuCB, (void*)this);
		menubar->add("File/Save As...", "+^s", menuCB, (void*)this);
		menubar->add("File/Quit", "^q", menuCB, (void*)this);
		menubar->textsize(12);
		
		makeNewModelsMenu(menubar, menuCB, (void*)this); // see globals.cpp
		
		menubar->add("Simulation/Properties", 0, menuCB, (void*)this);
		menubar->add("Simulation/_Statistics", 0, menuCB, (void*)this);
		menubar->add("Simulation/Stop", "^t", menuCB, (void*)this);
		menubar->add("Simulation/Initialize", "^i", menuCB, (void*)this);
		menubar->add("Simulation/Run", "^r", menuCB, (void*)this);
		menubar->add("Simulation/Pause", "^p", menuCB, (void*)this);
		
		root_canvas = current_canvas = new BlockCanvas(0,20,W,H-35, group);
		resizable(root_canvas);
		label("Block Editor");
		
		if (!group) {
			bannerbox = new Fl_Box(50, 50, W-100, H-100, "SimBlox Builder");
			bannerbox->labelsize(36);
			bannerbox->labelcolor(FL_GRAY);
		}
		callback(closeCB, this);
	}
	
	void BlockCanvasWindow::updateTitle()
	{
		std::stringstream ss;
		ss << "Block Editor";
		if (filename.length() > 0) {
			char relfname[128];
			fl_filename_relative(relfname, 127, filename.c_str());
			ss << ": " << relfname;
		}
		if (current_canvas) {
			ss << ": " << current_canvas->getGroup()->getName();
			if (bannerbox) {
				remove(bannerbox);
				delete bannerbox;
				bannerbox = NULL;
			}
		}
		if (changes_made)
			ss << " (*)";
		copy_label(ss.str().c_str());
		
	}
	
	void BlockCanvasWindow::setCurrentCanvas(BlockCanvas *canvas)
	{
		current_canvas = canvas;
		updateTitle();
	}
	
	void BlockCanvasWindow::parseXML(const TiXmlElement *element)
	{
		root_canvas->parseXML(element);
		position(sbx::XMLParser::parseInt(element, "position", "", true, x(), "x"),
				 sbx::XMLParser::parseInt(element, "position", "", true, y(), "y"));
		size(sbx::XMLParser::parseInt(element, "size", "", true, w(), "w"),
			 sbx::XMLParser::parseInt(element, "size", "", true, h(), "h"));
	}
	
	void BlockCanvasWindow::writeXML(TiXmlElement *element)
	{
		root_canvas->writeXML(element);
		sbx::XMLParser::setInt(element, "position", x(), "x");
		sbx::XMLParser::setInt(element, "position", y(), "y");
		sbx::XMLParser::setInt(element, "size", w(), "w");
		sbx::XMLParser::setInt(element, "size", h(), "h");
	}
	
	void BlockCanvasWindow::menuCB(Fl_Widget *w, void *data)
	{
		BlockCanvasWindow *win = (BlockCanvasWindow*) data;
		Fl_Menu_Bar *bar = (Fl_Menu_Bar*) w;
		char path[80];
		bar->item_pathname(path, sizeof(path)-1);
		win->menuAction(path);
	}
	
	void BlockCanvasWindow::menuAction(const std::string& path)
	{
		if (path == "File/New") {
			if (!newFile())
				return;
		} else if (path == "File/Open") {
			if (!loadFile())
				return;
		} else if (path == "File/Save") {
			saveFile();
		} else if (path == "File/Save As...") {
			saveAs();
		} else if (path.substr(0,4) == "New/") {
			newModelAction(path);
		} else if (path == "File/Quit") {
			if (!saveChoice())
				return;
			exit(0);
			/*
			 hide();
			 simulation_window->hide();
			 model_window->hide();
			 */
		} else if (path == "Simulation/Properties") {
			simulation_window->show();
		} else if (path == "Simulation/Statistics") {
			if (!stats_window)
				stats_window = new StatisticsWindow(x()+100, y()+100, 500, 300);
			stats_window->show();
			stats_window->update();
		} else if (path == "Simulation/Stop")
			simulation_window->getRunControl()->setState(STOPPED);
		else if (path == "Simulation/Initialize")
			simulation_window->getRunControl()->setState(INITIALIZED);
		else if (path == "Simulation/Run")
			simulation_window->getRunControl()->setState(RUNNING);
		else if (path == "Simulation/Pause")
			simulation_window->getRunControl()->setPaused(!simulation_window->getSimulation()->isPaused());
	}
	
	void BlockCanvasWindow::closeCB(Fl_Widget *w, void *data)
	{
		BlockCanvasWindow *win = (BlockCanvasWindow*) data;
		if (win)
			win->closeAction();
	}
	
	void BlockCanvasWindow::closeAction()
	{
		if (changes_made && !saveChoice())
			return;
		hide();
	}
	
} // namespace sbxBuilder
