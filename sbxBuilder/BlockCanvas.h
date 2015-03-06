#ifndef SBXBUILDER_BLOCKCANVAS_H
#define SBXBUILDER_BLOCKCANVAS_H

#include <FL/Fl_Scroll.H>
#include <sbx/Group.h>
#include <map>

namespace sbxBuilder {

class BlockCanvas : public Fl_Scroll {
public:
	BlockCanvas(int X, int Y, int W, int H, sbx::Group *group = NULL);
	virtual ~BlockCanvas();
	void update();
	
	void parseXML(const TiXmlElement *element);
	void writeXML(TiXmlElement *element);
	
	sbx::Group* getGroup() { return group.get(); }
	void setGroup(sbx::Group *group);
	void newGroup();

	static int mousex, mousey;
	
	typedef std::map<sbx::Group*, BlockCanvas*> CanvasMap;
	static CanvasMap canvases;
	
protected:
	int handle(int e);
	void popupMenu();
	static void menuCB(Fl_Widget *w, void *data);
	void menuAction(const std::string& path);

	void goToParent();

private:
	smrt::ref_ptr<sbx::Group> group;
	int popup_x, popup_y;
};

}

#endif
