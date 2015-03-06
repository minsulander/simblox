#include "PortBox.h"
#include "BlockCanvas.h"
#include "BlockCanvasWindow.h"
#include "global.h"
#include <sbx/XMLParser.h>
#include <sbx/ModelVisitor.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <sstream>

namespace sbxBuilder {
	
	PortBox::PortBox(int X, int Y, sbx::Port *nport, sbx::Model *nmodel)
	:	Fl_Box(X, Y, PORTWIDTH, PORTHEIGHT),
	port(nport),
	exported(false),
	model(nmodel)
	{
		box(FL_UP_BOX);
		labelsize(PORTHEIGHT-2);
		
		boxes[port] = this;
		update();
	}
	
	PortBox::~PortBox()
	{
		boxes[port] = NULL;
	}
	
	void PortBox::update() {
		exported = false;
		std::string name =  model->getPortName(port);
		copy_label(name.c_str());
		if (port->isInput())
			align(FL_ALIGN_RIGHT);
		else
			align(FL_ALIGN_LEFT);
		std::stringstream ss;
		ss << name;
		if (port->getUnit().length() > 0)
			ss << " [" << port->getUnit() << "]";
		ss << (port->isInput() ? " (in)" : " (out)");
		if (model->getPortDescription(port).length() > 0)
			ss << "\n" << model->getPortDescription(port);
		description = ss.str();
		tooltip(description.c_str());
		
		
		sbx::Model *owner = port->getOwner();
		if (model == owner && owner->getParent()) {
			for (unsigned int i = 0; i < owner->getParent()->getNumPorts(); i++)
				if (owner->getParent()->getPort(i) == port) {
					exported = true;
				}
		}

		if (parent() && parent()->parent() && dynamic_cast<BlockCanvas*> (parent()->parent())) {
			BlockCanvas* canvas = (BlockCanvas*) parent()->parent();
			if (canvas->getGroup()->ownsPort(port))
				exported = true;
		}
		setNormalColor();
	}
	
	bool PortBox::isInput() { return port->isInput(); }
	
	sbx::Port *PortBox::getPort() { return port; }
	
	sbx::Model *PortBox::getModel() { return model.get(); }
	
	void PortBox::setNormalColor(bool showselected)
	{
		if (showselected && selected == this)
			color(PORT_COLOR_SELECTED);
		else if (port->isValid())
			color(PORT_COLOR_NORMAL);
		else
			color(PORT_COLOR_INVALID);
	}
	
	void PortBox::writeXML(TiXmlElement *element)
	{
		//sbx::XMLParser::setString(element, "name", model->getPortName(port));
	}
	
	void PortBox::popupMenu()
	{
		Fl_Menu_Button *popup = new Fl_Menu_Button(Fl::event_x(), Fl::event_y(), 0, 0);
		popup->type(Fl_Menu_Button::POPUP3);
		popup->add("Connect", 0, menuCB, (void*)this);
		if (port->isConnected())
			popup->add("Disconnect", 0, menuCB, (void*)this);
		if (!exported)
			popup->add("Export", 0, menuCB, (void*)this);
		else
			popup->add("Unexport", 0, menuCB, (void*)this);
		popup->textsize(12);
		window()->add(popup);
		popup->popup();
		window()->remove(popup);
		delete popup;
	}
	
	void PortBox::menuCB(Fl_Widget *w, void *data)
	{
		PortBox *box = (PortBox*) data;
		Fl_Menu_ *menu = (Fl_Menu_*) w;
		char path[80];
		menu->item_pathname(path, sizeof(path)-1);
		box->menuAction(path);
	}
	
	void PortBox::menuAction(const std::string& path)
	{
		std::cout << "action " << path << "\n";
		if (path == "/Connect") {
			const char *portname = fl_input("Connect to port name:");
			if (portname) {
				sbx::FindVisitor finder;
				sbx::Port *otherend = finder.findPort(*model->getRoot(), portname);
				if (!otherend)
					fl_message("Unknown port name");
				else {
					try {
						port->connect(otherend);
						update();
						parent()->parent()->redraw();
					} catch (sbx::PortException& e) {
						fl_beep(FL_BEEP_ERROR);
						fl_message("The port connection cannot be made:\n%s", e.what());
					}
				}
			}
		} else if (path == "/Disconnect") {
			disconnect();		
		} else if (path == "/Export" && model.valid() && model->getParent()) {
			const char *name = fl_input("Enter the exported name:", model->getPortName(port).c_str());
			if (name)
				model->getParent()->exportChildPort(port, name);
			update();
			parent()->parent()->redraw();
		} else if (path == "/Unexport" && model.valid() && model->getParent()) {
			model->getParent()->unexportChildPort(port);
			update();
			parent()->parent()->redraw();
		}
	}
	
	void PortBox::disconnect()
	{
		std::vector<PortBox*> others;
		for (unsigned int i = 0; i < port->getNumConnections(); i++)
			others.push_back(boxes[port->getOtherEnd(i)]);
		port->disconnect();
		setNormalColor();
		for (unsigned int i = 0; i < others.size(); i++) {
			others[i]->setNormalColor();
			others[i]->redraw();
		}
		parent()->parent()->redraw();
	}
	
	void PortBox::draw()
	{
		Fl_Box::draw();
		drawLine();
	}
	
	void PortBox::drawLine() {
		if (exported) {
			int x1, y1, x2, y2;
			int xt, align;
			if (port->isInput()) {
				x2 = x()+w()/2;
				y1 = y2 = y()+h()/2;
				x1 = x2-PORTWIDTH;
				align = FL_ALIGN_RIGHT;
				xt = x1;
			} else {
				x1 = x()+w()/2;
				y1 = y2 = y()+h()/2;
				x2 = x1+PORTWIDTH;
				align = FL_ALIGN_LEFT;
				xt = x2;
			}
			// Gray for no exported connections, black for exported connections
			fl_color(FL_GRAY);
			for (unsigned int i = 0; i < port->getNumConnections(); i++) {
				if (port->getOtherEnd(i)->getOwner()->getParent() != port->getOwner()->getParent())
					fl_color(FL_BLACK);
			}
			fl_line(x1, y1, x2, y2);
			sbx::Group *group = ((BlockCanvas*)parent()->parent())->getGroup();
			std::string exportName = group->getPortName(port);
			fl_draw(exportName.c_str(), xt, y1-PORTHEIGHT/2, 0, PORTHEIGHT, (Fl_Align)align);
		}
		
		for (unsigned int i = 0; i < port->getNumConnections(); i++) {
			PortBox *otherend = boxes[port->getOtherEnd(i)];
			if (!otherend || otherend->parent()->parent() != parent()->parent()) {
				if (!otherend || !exported && otherend->getModel()->getParent() != model.get()) {
					// far out external connection...
					int x1, y1, x2, y2;
					int xt, align;
					if (port->isInput()) {
						x2 = x()+w()/2;
						y1 = y2 = y()+h()/2;
						x1 = x2-PORTWIDTH;
						align = FL_ALIGN_RIGHT;
						xt = x1;
					} else {
						x1 = x()+w()/2;
						y1 = y2 = y()+h()/2;
						x2 = x1+PORTWIDTH;
						align = FL_ALIGN_LEFT;
						xt = x2;
					}
					if (port->isConnected())
						fl_color(FL_BLACK);
					else
						fl_color(FL_GRAY);
					fl_line(x1, y1, x2, y2);
					sbx::Model *other = port->getOtherEnd()->getOwner();
					std::string name = other->getPath() + "." + other->getPortName(port->getOtherEnd());
					fl_draw(name.c_str(), xt, y1-PORTHEIGHT/2, 0, PORTHEIGHT, (Fl_Align)align);
				}
			} else {
				int x1 = x()+w()/2;
				int y1 = y()+h()/2;
				int x2 = otherend->x()+otherend->w()/2;
				int y2 = otherend->y()+otherend->h()/2;
				fl_color(FL_BLACK);
				fl_line(x1, y1, x2, y2);
			}
		}
		if (selected == this) {
			int x1 = x()+w()/2;
			int y1 = y()+h()/2;
			fl_color(FL_WHITE);
			fl_line(x1, y1, BlockCanvas::mousex, BlockCanvas::mousey);
		}
	}
	
	/*
	 void PortBox::drawLine() {
	 if ((this->isInput() && this->isConnected()) || selected == this) {
	 int x1, y1, x2, y2;
	 PortBox *otherend = NULL;
	 if (selected == this) {
	 x1 = this->x();
	 if (!this->isInput())
	 x1 += this->w();
	 y1 = this->y()+this->h()/2;
	 x2 = BlockCanvas::mousex;
	 y2 = BlockCanvas::mousey;
	 } else {
	 otherend = this->getOtherEnd();
	 x1 = this->x();
	 y1 = this->y()+this->h()/2;
	 x2 = otherend->x()+otherend->w();
	 y2 = otherend->y()+otherend->h()/2;
	 }
	 
	 int corner = findClearCorner(x1+(x2-x1)/2, y1, y2);
	 
	 fl_color(FL_BLACK);
	 fl_line(x1, y1, corner, y1);
	 fl_line(corner, y1, corner, y2);
	 fl_line(corner, y2, x2, y2);
	 }
	 }
	 
	 int PortBox::findClearCorner(int wanted, int y1, int y2) {
	 int corner = wanted;
	 int direction = 0;
	 bool clear = false;
	 while (!clear) {
	 clear = true;
	 for (CornerMap::iterator it = corners.begin(); it != corners.end(); it++) {
	 if (it->first == this)
	 continue;
	 CornerSegment& segment = it->second;
	 if (     (y2 > y1 && (segment.y1 > y1 || segment.y2 > y1) && (segment.y1 < y2 || segment.y2 < y2))
	 || (y1 > y2 && (segment.y1 > y2 || segment.y2 > y2) && (segment.y1 < y1 || segment.y2 < y1))  ) {
	 if (abs(corner - segment.x) < PORTHEIGHT) {
	 clear = false;
	 if (!direction) {
	 if (corner > segment.x)
	 direction = 1;
	 else
	 direction = -1;
	 }
	 corner += direction;
	 }
	 }
	 }
	 }
	 if (this->isInput()) {
	 corners[this].x = corner;
	 corners[this].y1 = y1;
	 corners[this].y2 = y2;
	 }
	 return corner;
	 }
	 */
	
	int PortBox::handle(int e) {
		int ret = Fl_Box::handle(e);
		switch (e) {
			case FL_ENTER:
				if (selected) {
					if (selected->getPort()->canConnect(getPort()))
						color(PORT_COLOR_OK);
					else
						color(PORT_COLOR_NOTOK);
					redraw();
				}
				return 1;
			case FL_LEAVE:
				setNormalColor();
				redraw();
				return 1;
			case FL_PUSH:
				if (Fl::event_button() == FL_LEFT_MOUSE) {
					if (selected) {
						try {
							port->connect(selected->getPort());
							change();
						} catch (sbx::PortException& e) {
							fl_beep(FL_BEEP_ERROR);
							fl_message("The port connection cannot be made:\n%s", e.what());
						}
						selected->setNormalColor(false);
						selected = NULL;
						setNormalColor();
						parent()->parent()->redraw();
					} else if (Fl::event_state() & FL_SHIFT) {
						disconnect();
						change();
					} else {
						selected = this;
						color(PORT_COLOR_SELECTED);
						redraw();
					}
					return 1;
				} else if (Fl::event_button() == FL_RIGHT_MOUSE) {
					popupMenu();
					return 1;
				}
		}
		return ret;
	}
	
	PortBox *PortBox::selected = NULL;
	PortBox::BoxMap PortBox::boxes;
	PortBox::CornerMap PortBox::corners;
	
}
