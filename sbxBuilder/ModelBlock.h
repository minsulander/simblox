#ifndef SBXBUILDER_MODELBLOCK_H
#define SBXBUILDER_MODELBLOCK_H

#include <FL/Fl_Group.H>
#include <sbx/Model.h>
#include <smrt/ref_ptr.h>
#include <string>
#include <map>

namespace sbxBuilder {
	
#define MODEL_COLOR_NORMAL		FL_GRAY
#define MODEL_COLOR_SELECTED	FL_WHITE
	
	class ModelBlock : public Fl_Group {
	public:
		ModelBlock(int X, int Y, sbx::Model *model);
		virtual ~ModelBlock();
		void update();
		sbx::Model *getModel() { return model.get(); }
		
		void parseXML(const TiXmlElement *element);
		void writeXML(TiXmlElement *element);
		
		/// Called when user clicks on the block
		void select();
		/// Called when previously selected and now unselected
		void unselect();
		/// Called when user double-clicks on the block
		void open();
		/// Called when user selects "properties" from the menu
		void edit();
		
		static ModelBlock *selected;
		typedef std::map< sbx::Model*, ModelBlock*> BlockMap;
		static BlockMap blocks;
		
	protected:
		void popupMenu();
		static void menuCB(Fl_Widget *w, void *data);
		void menuAction(const std::string& path);
		int handle(int e);
		virtual void draw();
		/// Setup box, colors, labels etc depending on the type of model
		void decorate();
		smrt::ref_ptr<sbx::Model> model;
		std::string description;
		Fl_Color normalColor;
	};
	
}

#endif
