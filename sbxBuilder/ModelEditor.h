#ifndef SBXBUILDER_MODELEDITOR_H
#define SBXBUILDER_MODELEDITOR_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Box.H>
#include <sbx/Model.h>
#include <smrt/ref_ptr.h>

namespace sbxBuilder {

class ModelEditor : public Fl_Double_Window {
public:
	ModelEditor(int X, int Y, int W, int H);
	void setModel(sbx::Model *model);
	sbx::Model* getModel() { return model.get(); }
	
	void parseXML(const TiXmlElement *element);
	void writeXML(TiXmlElement *element);
	
protected:
	static void changeCB(Fl_Widget *w, void *data);
	void changeAction(Fl_Widget *w);

	smrt::ref_ptr<sbx::Model> model;
	Fl_Input *name;
	Fl_Input *freq;
	Fl_Output *classinfo;
	Fl_Group *head, *params;
};

}

#endif
