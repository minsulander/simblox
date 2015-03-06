#include "BlockCanvas.h"
#include "PortBox.h"
#include "ModelBlock.h"
#include "ModelEditor.h"
#include "BlockCanvasWindow.h"
#include "global.h"
#include <sbx/XMLParser.h>
#include <sbx/ModelVisitor.h>
#include <sbx/ModelFactory.h>
#include <sbx/Log.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Button.H>
#include <string>

namespace sbxBuilder {
	
	/// \todo Use refpointers!
	
	BlockCanvas::BlockCanvas(int X, int Y, int W, int H, sbx::Group *ngroup)
	:	Fl_Scroll(X, Y, W, H),
	group(ngroup)
	{
		end();
		box(FL_FLAT_BOX);
		color(Fl_Color(46));
		if (group.valid()) {
			canvases[group.get()] = this;
			update();
		}
	}
	
	BlockCanvas::~BlockCanvas()
	{
		sbx::dout(5) << "~BlockCanvas() " << this << "\n";
		canvases[group.get()] = NULL;
	}
	
	void BlockCanvas::update()
	{
		if (!group.valid())
			return;
		begin();
		int curx = x()+10, cury = y()+10, maxheight = 0;
		for (unsigned int i = 0; i < group->getNumChildren(); i++) {
			ModelBlock *block = ModelBlock::blocks[group->getChild(i)];
			if (!block) {
				block = new ModelBlock(curx, cury, group->getChild(i));
				block->end();
				if (curx + block->w() > w()) {
					cury += maxheight+10;
					curx = x()+10;
					maxheight = 0;
				} else {
					curx += block->w()+10;
				}
				if (block->h() > maxheight)
					maxheight = block->h();
			}
			block->update();
		}
		end();
		redraw();
		if (window()) {
			BlockCanvasWindow *bcwin = dynamic_cast<BlockCanvasWindow*> (window());
			if (bcwin) {
				bcwin->setCurrentCanvas(this);
			}
		}
	}
	
	void BlockCanvas::parseXML(const TiXmlElement *element)
	{
		canvases[group.get()] = this;
		sbx::dout(5) << "set canvas " << group.get() << " = " << this << "\n";
		sbx::FindVisitor finder;
		for (const TiXmlElement* elem=element->FirstChildElement(); elem; elem=elem->NextSiblingElement()) {
			if (std::string(elem->Value()) == "block") {
				std::string modelname = sbx::XMLParser::parseStringAttribute(elem, "model");
				sbx::Model *model = finder.findModel(*group.get(), modelname);
				if (!model)
					throw sbx::ParseException(std::string("Unknown model '") + modelname + "'");
				
				const TiXmlElement *canvasElement = elem->FirstChildElement("canvas");
				if (canvasElement) {
					std::string groupname = sbx::XMLParser::parseStringAttribute(canvasElement, "group");
					sbx::Group *cgroup = dynamic_cast<sbx::Group*> (finder.findModel(*group.get(), groupname));
					sbx::dout(4) << "subcanvas for " << groupname << " " << cgroup << "\n";
					if (!cgroup)
						throw sbx::ParseException(std::string("Unknown group '") + groupname + "'");
					BlockCanvas *canvas = new BlockCanvas(x(), y(), w(), h());
					canvas->group = cgroup;
					canvas->parseXML(canvasElement);
					canvas->hide();
					add(canvas);
				}
				
				ModelBlock *block = new ModelBlock(0, 0, model);
				add(block);
				block->parseXML(elem);
			}
		}
		
		update();
		
	}
	
	void BlockCanvas::writeXML(TiXmlElement *element)
	{
		if (!group.valid())
			return;
		element->SetAttribute("group", group->getPath());
		for (unsigned int i = 0; i < children(); i++) {
			ModelBlock *block = dynamic_cast<ModelBlock*> (child(i));
			if (!block)
				continue;
			TiXmlElement *blockElement = new TiXmlElement("block");
			block->writeXML(blockElement);
			if (canvases[(sbx::Group*)block->getModel()]) {
				TiXmlElement *canvasElement = new TiXmlElement("canvas");
				canvases[(sbx::Group*)block->getModel()]->writeXML(canvasElement);
				blockElement->LinkEndChild(canvasElement);
			}
			element->LinkEndChild(blockElement);
		}
	}
	
	void BlockCanvas::setGroup(sbx::Group *newgroup)
	{
		clear();
		group = newgroup;
		//update();
	}
	
	void BlockCanvas::newGroup()
	{
		clear();
		group = new sbx::Group;
		group->setName("root");
		update();
	}
	
	void BlockCanvas::popupMenu()
	{
		popup_x = Fl::event_x();
		popup_y = Fl::event_y();
		Fl_Menu_Button *popup = new Fl_Menu_Button(popup_x, popup_y, 0, 0);
		popup->type(Fl_Menu_Button::POPUP3);
		if (group->getParent())
			popup->add("Go To Parent", 0, menuCB, (void*)this);
		makeNewModelsMenu(popup, menuCB, (void*)this); // see globals.cpp
		popup->textsize(12);
		window()->add(popup);
		popup->popup();
		window()->remove(popup);
		delete popup;
	}
	
	void BlockCanvas::menuCB(Fl_Widget *w, void *data)
	{
		BlockCanvas *canvas = (BlockCanvas*) data;
		Fl_Menu_ *menu = (Fl_Menu_*) w;
		char path[80];
		menu->item_pathname(path, sizeof(path)-1);
		canvas->menuAction(path);
	}
	
	void BlockCanvas::menuAction(const std::string& path)
	{
		if (path == "/Go To Parent")
			goToParent();
		else if (path.substr(0,4) == "New/")
			newModelAction(path, popup_x, popup_y);
		else
			sbx::dout(3) << "BlockCanvas menu: " << path << "\n";
	}
	
	int BlockCanvas::mousex = 0;
	int BlockCanvas::mousey = 0;
	
	int BlockCanvas::handle(int e)
	{
		int ret = Fl_Scroll::handle(e);
		switch (e) {
			case FL_PUSH:
				if (Fl::belowmouse() == this) {
					if (PortBox::selected) {
						PortBox::selected->setNormalColor(false);
						PortBox::selected = NULL;
						redraw();
					}
					if (ModelBlock::selected) {
						ModelBlock::selected->unselect();
					}
					if (Fl::event_button() == FL_RIGHT_MOUSE) {
						popupMenu();
					} else if (Fl::event_clicks() >= 1 && group->getParent()) {
						goToParent();
					}
					return 1;
				} else if (Fl::event_clicks() >= 1) {
					ModelBlock *block = dynamic_cast<ModelBlock*> (Fl::belowmouse());
					if (block) {
						block->open();
						return 1;
					}
				}
				break;
			case FL_MOVE:
				if (PortBox::selected) {
					mousex = Fl::event_x();
					mousey = Fl::event_y();
					redraw();
					return 1;
				}
				break;
			case FL_KEYUP:
				switch(Fl::event_key()) {
					case FL_BackSpace:
					case FL_Delete:
						if (ModelBlock::selected) {
							ModelBlock *block = ModelBlock::selected;
							sbx::Model *model = block->getModel();
							if (model_window && model_window->getModel() == model)
								model_window->setModel(NULL);
							sbx::dout(3) << "remove " << model->getName() << " refcount " << model->referenceCount() << "\n";
							block->unselect();
							group->removeChild(model);
							remove(block);
							delete block;
							update();
							redraw();
							change();
						}
						return 1;
					case FL_Enter:
					case FL_KP_Enter:
						if (ModelBlock::selected)
							ModelBlock::selected->open();
						return 1;
				}
				break;
		}
		return ret;
	}
	
	void BlockCanvas::goToParent()
	{
		sbx::Group* pgroup = dynamic_cast<sbx::Group*> (group->getParent());
		if (pgroup) {
			if (canvases[pgroup]) {
				hide();
				canvases[pgroup]->update();
				canvases[pgroup]->show();
			} else {
				BlockCanvas *canvas = new BlockCanvas(x(),y(),w(),h(),pgroup);
				parent()->add(canvas);
				hide();
				canvas->show();
			}	
		}
	}
	
	BlockCanvas::CanvasMap BlockCanvas::canvases;
	
} // namespace sbxBuilder
