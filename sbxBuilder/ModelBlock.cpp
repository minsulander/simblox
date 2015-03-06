#include "ModelBlock.h"
#include "PortBox.h"
#include "BlockCanvas.h"
#include "BlockCanvasWindow.h"
#include "ModelEditor.h"
#include "Decorations.h"
#include "global.h"
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

namespace sbxBuilder {
	
	extern bool changes_made;
	
	ModelBlock::ModelBlock(int X, int Y, sbx::Model *nmodel)
	: Fl_Group(X,Y,100,60), model(nmodel)
	{
		box(FL_THIN_UP_BOX);
		color(MODEL_COLOR_NORMAL);
		normalColor = MODEL_COLOR_NORMAL;
		update();
		blocks[model.get()] = this;
	}
	
	ModelBlock::~ModelBlock()
	{
		if (selected == this)
			selected = NULL;
		blocks[model.get()] = NULL;
	}
	
	void ModelBlock::update() {
		copy_label(model->getName().c_str());
		labelsize(PORTHEIGHT-2);
		align(FL_ALIGN_CENTER | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_TEXT_OVER_IMAGE);
		description = model->getName() + " " + std::string(model->libraryName()) + "::" + model->className() + "\n" + model->description();
		tooltip(description.c_str());
		
		unsigned int incounter = 0, outcounter = 0;
		// Figure out height by number of ports
		for (unsigned int i = 0; i < model->getNumPorts(); i++) {
			if (model->getPort(i)->isInput())
				incounter++;
			else
				outcounter++;
		}
		unsigned int portcount = (incounter > outcounter ? incounter : outcounter);
		size(w(), (portcount+1) * (PORTHEIGHT+3) + PORTHEIGHT/2);
		
		// Figure out width by port names / model name
		incounter = outcounter = 0;
		std::vector<unsigned int> lengths;
		lengths.resize(portcount);
		for (unsigned int i = 0; i < model->getNumPorts(); i++) {
			if (model->getPort(i)->isInput())
				lengths[incounter++] += model->getPortName(i).length();
			else
				lengths[outcounter++] += model->getPortName(i).length();
		}
		unsigned int len = 0;
		for (unsigned int i = 0; i < lengths.size(); i++) {
			if (lengths[i] > len)
				len = lengths[i];
		}
		if (model->getName().length() > len)
			len = model->getName().length();
		if (len < 3)
			len = 3;
		size( len*PORTWIDTH+6, h());
		
		// Add port boxes
		clear();
		incounter = outcounter = 0;
		for (unsigned int i = 0; i < model->getNumPorts(); i++) {
			sbx::Port *port = model->getPort(i);
			int x1,y1;
			if (port->isInput()) {
				x1 = x()+3;
				y1 = y()+(PORTHEIGHT+3)*(incounter+1)+3;
				incounter++;
			} else { 
				x1 = x()+w()-PORTWIDTH-3;
				y1 = y()+(PORTHEIGHT+3)*(outcounter+1)+3;
				outcounter++;
			}
			PortBox *pb = new PortBox(x1, y1, port, model.get());
			add(pb);
		}
		
		decorate();
	}
	
	void ModelBlock::parseXML(const TiXmlElement *element)
	{
		int nx = sbx::XMLParser::parseInt(element, "position", "", false, 0, "x");
		int ny = sbx::XMLParser::parseInt(element, "position", "", false, 0, "y");
		int nw = sbx::XMLParser::parseInt(element, "size", "", false, 0, "w");
		int nh = sbx::XMLParser::parseInt(element, "size", "", false, 0, "h");
		position(nx, ny);
		size(nw, nh);
	}
	
	void ModelBlock::writeXML(TiXmlElement *element)
	{
		element->SetAttribute("model", model->getPath());
		sbx::XMLParser::setInt(element, "position", x(), "x");
		sbx::XMLParser::setInt(element, "position", y(), "y");
		sbx::XMLParser::setInt(element, "size", w(), "w");
		sbx::XMLParser::setInt(element, "size", h(), "h");
		/*
		 for (unsigned int i = 0; i < children(); i++) {
		 PortBox *pb = dynamic_cast<PortBox*> (child(i));
		 if (!pb)
		 continue;
		 TiXmlElement *portElement = new TiXmlElement("port");
		 element->LinkEndChild(portElement);
		 pb->writeXML(portElement);
		 }
		 */
	}
	
	void ModelBlock::select()
	{
		if (selected == this)
			return;
		else if (selected)
			selected->unselect();
		selected = this;
		color(MODEL_COLOR_SELECTED);
		redraw();
	}
	
	void ModelBlock::unselect()
	{
		selected = NULL;
		color(normalColor);
		redraw();
	}
	
	void ModelBlock::open()
	{
		sbx::Group *group = dynamic_cast<sbx::Group*> (getModel());
		if (group && parent() && parent()->parent()) {
			BlockCanvas *canvas = BlockCanvas::canvases[group];
			sbx::dout(5) << "get canvas " << group << " = " << canvas << "\n";
			if (canvas) {
				sbx::dout(3) << "open existing canvas " << group << "\n";
				parent()->hide();
				canvas->resize(parent()->x(),parent()->y(),parent()->w(),parent()->h());
				parent()->parent()->add(canvas);
				parent()->parent()->resizable(canvas);
				canvas->update();
				canvas->show();
			} else {
				sbx::dout(3) << "open new canvas " << group << "\n";
				canvas = new BlockCanvas(parent()->x(),parent()->y(),parent()->w(),parent()->h(),group);
				parent()->parent()->add(canvas);
				parent()->hide();
				canvas->show();
				canvas->update();
			}
			model_window->setModel(group);
		} else {
			edit();
		}
	}
	
	void ModelBlock::edit()
	{
		model_window->setModel(getModel());
		model_window->show();
		model->show();
	}
	
	void ModelBlock::popupMenu()
	{
		Fl_Menu_Button *popup = new Fl_Menu_Button(Fl::event_x(), Fl::event_y(), 0, 0);
		popup->type(Fl_Menu_Button::POPUP3);
		popup->add("Properties", 0, menuCB, (void*)this);
		if (model.valid() && model->supportsDynamicInputs()) {
			popup->add("Add Input", 0, menuCB, (void*)this);
			popup->add("Remove Input", 0, menuCB, (void*)this);
		}
		if (model.valid() && model->supportsDynamicOutputs()) {
			popup->add("Add Output", 0, menuCB, (void*)this);
			popup->add("Remove Output", 0, menuCB, (void*)this);
		}
		popup->add("Duplicate", 0, menuCB, (void*)this);
		popup->textsize(12);
		window()->add(popup);
		popup->popup();
		window()->remove(popup);
		delete popup;
	}
	
	void ModelBlock::menuCB(Fl_Widget *w, void *data)
	{
		ModelBlock *block = (ModelBlock*) data;
		Fl_Menu_ *menu = (Fl_Menu_*) w;
		char path[80];
		menu->item_pathname(path, sizeof(path)-1);
		block->menuAction(path);
	}
	
	void ModelBlock::menuAction(const std::string& path)
	{
		std::cout << "action " << path << "\n";
		if (!model.valid())
			return;
		if (path == "/Properties") {
			edit();
		} else if (path == "/Add Input") {
			model->addInput();
		} else if (path == "/Remove Input") {
			model->removeInput();
		} else if (path == "/Add Output") {
			model->addOutput();
		} else if (path == "/Remove Output") {
			model->removeOutput();
		} else if (path == "/Duplicate") {
			if (!canvas_window || !canvas_window->getCurrentCanvas())
				return;
			BlockCanvas *current_canvas = canvas_window->getCurrentCanvas();
			smrt::ref_ptr<sbx::Model> newmodel = (sbx::Model*) model->clone();
			unsigned int name_index = 0;
			for (unsigned int i = 0; i < current_canvas->getGroup()->getNumChildren(); i++) {
				if (newmodel->getName() == current_canvas->getGroup()->getChild(i)->getName().substr(0,newmodel->getName().length()))
					name_index++;
			}
			if (name_index > 0) {
				std::stringstream namess;
				namess << newmodel->getName() << name_index;
				newmodel->setName(namess.str());
			}
			current_canvas->getGroup()->addChild(newmodel.get());
			ModelBlock *block = new ModelBlock(x()+name_index*w()/2, y()+name_index*h()/2, newmodel.get());
			ModelBlock::blocks[newmodel.get()] = block;
			current_canvas->add(block);
			/// \todo Copy block canvas positions (recursively)
			block->select();
			change();
			newmodel->show();
		}
		update();
		if (parent())
			parent()->redraw();
	}
	
	int ModelBlock::handle(int e) {
		if (!parent())
			return 0;
		static int offset[2] = { 0, 0 };
		int ret = Fl_Group::handle(e);
		switch ( e ) {
			case FL_PUSH:
				if (Fl::event_button() == FL_LEFT_MOUSE && Fl::belowmouse() == this) {
					if (Fl::belowmouse() == this) {
						offset[0] = x() - Fl::event_x();    // save where user clicked for dragging
						offset[1] = y() - Fl::event_y();
						parent()->add(this); // bring it up front
						select();
					}
					return 1;
				} else if (Fl::event_button() == FL_RIGHT_MOUSE && Fl::belowmouse() == this) {
					popupMenu();
					return 1;
				}
			case FL_RELEASE:
				return 1;
			case FL_DRAG:
				position(offset[0]+Fl::event_x(), offset[1]+Fl::event_y());     // handle dragging
				change();
				parent()->redraw();
				return 1;
		}
		return ret;
	}
	
	void ModelBlock::draw()
	{
		if (dynamic_cast<sbx::Group*> (model.get())) {
			fl_draw_box(FL_THIN_UP_BOX, x()+3, y()+3, w(), h(), color());
		}
		Fl_Group::draw();
	}
	
	void ModelBlock::decorate()
	{
		if (!model.valid())
			return;
		std::string fullname = std::string(model->libraryName()) + "::" + model->className();
		Decorations::instance().apply(fullname, this);
		normalColor = color();
	}
	
	ModelBlock *ModelBlock::selected = NULL;
	ModelBlock::BlockMap ModelBlock::blocks;
	
} // namespace sbxBuilder
