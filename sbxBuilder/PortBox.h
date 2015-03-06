#ifndef SBXBUILDER_PORTBOX_H
#define SBXBUILDER_PORTBOX_H

#include <sbx/Ports.h>
#include <smrt/ref_ptr.h>
#include <FL/Fl_Box.H>
#include <map>
#include <vector>
#include <string>

class TiXmlElement;

namespace sbxBuilder {
	
#define PORTWIDTH 14
#define PORTHEIGHT 14
#define PORT_COLOR_NORMAL	FL_GRAY
#define PORT_COLOR_SELECTED	FL_WHITE
#define PORT_COLOR_OK		FL_GREEN
#define PORT_COLOR_NOTOK	FL_RED
#define PORT_COLOR_INVALID	FL_BLACK
	
	class PortBox : public Fl_Box {
	public:
		PortBox(int X, int Y, sbx::Port *port, sbx::Model *model);
		virtual ~PortBox();
		void update();
		
		bool isInput();
		sbx::Port *getPort();
		sbx::Model *getModel();
		void setNormalColor(bool showselected = true);
		void writeXML(TiXmlElement *element);
		
		static PortBox *selected;	
		typedef std::map<sbx::Port*, PortBox*> BoxMap;
		static BoxMap boxes;
		
	protected:
		void popupMenu();
		static void menuCB(Fl_Widget *w, void *data);
		void menuAction(const std::string& path);
		void disconnect();
		virtual void draw();
		void drawLine();
		int findClearCorner(int wanted, int y1, int y2);
		int handle(int e);
		
		sbx::Port *port;
		smrt::ref_ptr<sbx::Model> model;
		std::string description;
		bool exported;
		
		struct CornerSegment {
			int x, y1, y2;
		};
		typedef std::map<PortBox*, CornerSegment> CornerMap;
		static CornerMap corners;
		
	};
	
}

#endif


