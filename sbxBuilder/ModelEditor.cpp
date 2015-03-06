#include "ModelEditor.h"
#include "SimulationEditor.h"
#include "global.h"
#include "ModelBlock.h"
#include "BlockCanvasWindow.h"
#include <sbx/ModelVisitor.h>
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <units/units.h>
#include <FL/fl_ask.H>
#include <sstream>

namespace sbxBuilder {

ModelEditor::ModelEditor(int X, int Y, int W, int H)
: Fl_Double_Window(X, Y, W, H, "Model Editor")
{
	head = new Fl_Group(0, 0, W, 65);
	head->box(FL_FLAT_BOX);
	classinfo = new Fl_Output(50, 5, W-50, 15, "Class");
	classinfo->box(FL_FLAT_BOX);
	classinfo->color(FL_GRAY);
	classinfo->textsize(12);
	classinfo->labelsize(12);
	classinfo->set_output();
	name = new Fl_Input(W/3, 20, W*2/3, 20, "Name");
	name->callback(changeCB, (void*) this);
	freq = new Fl_Input(W/3, 40, W*2/3, 20, "Freq");
	freq->callback(changeCB, (void*) this);
	head->end();
		
	params = new Fl_Group(0, head->h(), W, H - head->h());
	params->box(FL_FLAT_BOX);
	params->end();
}

void ModelEditor::setModel(sbx::Model *newmodel)
{
	model = newmodel;
	if (!model) {
		hide();
		return;
	}
	std::stringstream ss;

	ss << model->libraryName() << "::" << model->className();	
	if (model->getMinimumUpdateFrequency())
		ss << " min " << model->getMinimumUpdateFrequency() << " Hz";
	if (model->isEndPoint())
		ss << " endpoint";
	classinfo->value(ss.str().c_str());
	
	name->value(model->getName().c_str());
	
	if (model->getUpdateFrequency() == 0) {
		ss.str("");
		ss << simulation_window->getSimulation()->getFrequency() << " Hz (sim)";
		freq->value(ss.str().c_str());
	} else {
		ss.str("");
		ss << model->getUpdateFrequency() << " Hz";
		freq->value(ss.str().c_str());
	}
	
	params->clear();
	params->begin();
	params->resize(0, head->h(), w(), model->getNumParameters()*20);
	for (unsigned int i = 0; i < model->getNumParameters(); i++) {
		const sbx::Model::Parameter& param = model->getParameterSpec(i);
		Fl_Input *paramin = new Fl_Input(w()/3, params->y()+i*20, 2*w()/3, 20, param.name.c_str());
		paramin->callback(changeCB, (void*) this);
		if (param.unit.length() > 0)
			paramin->value( (model->getParameter(param.name) + " " + param.unit).c_str() );
		else
			paramin->value(model->getParameter(param.name).c_str());
		paramin->tooltip(param.description.c_str());
	}
	params->end();
	size(w(), head->h()+params->h());
}

void ModelEditor::parseXML(const TiXmlElement *element)
{
	position(sbx::XMLParser::parseInt(element, "position", "", true, x(), "x"),
			 sbx::XMLParser::parseInt(element, "position", "", true, y(), "y"));
	if (sbx::XMLParser::parseBoolean(element, "visible", true, false))
		show();
	else
		hide();
	std::string modelstr = sbx::XMLParser::parseString(element, "model", true, "");
	if (modelstr.length() > 0) {
		sbx::FindVisitor finder;
		setModel(finder.findModel(*canvas_window->getRootCanvas()->getGroup(), modelstr));
	}
}

void ModelEditor::writeXML(TiXmlElement *element)
{
	sbx::XMLParser::setInt(element, "position", x(), "x");
	sbx::XMLParser::setInt(element, "position", y(), "y");
	sbx::XMLParser::setBoolean(element, "visible", visible());
	if (model.valid())
		sbx::XMLParser::setString(element, "model", model->getPath(canvas_window->getRootCanvas()->getGroup()));
}

void ModelEditor::changeCB(Fl_Widget *w, void *data)
{
	ModelEditor *modeled = (ModelEditor*) data;
	modeled->changeAction(w);
}

void ModelEditor::changeAction(Fl_Widget *w)
{
	if (!model)
		return;
	try {
		if (w == name)
			model->setName(name->value());
		
		if (w == freq) {
			std::stringstream ss(freq->value());
			std::string unit;
			double val;
			if (ss >> val) {
				if (ss >> unit)
					val = units::convert(val, unit, "Hz");
				model->setUpdateFrequency((int)val);
			}
		}
		ModelBlock *block = ModelBlock::blocks[model.get()];
		unsigned int i = params->find(w);
		if (i < params->children()) {
			const sbx::Model::Parameter& param = model->getParameterSpec(i);
			Fl_Input *paramin = (Fl_Input*) params->child(i);
			std::string value, unit;
			if (param.unit.length() > 0) {
				std::stringstream ss(paramin->value());
				ss >> value >> unit;
			} else
				value = paramin->value();
			try {
				model->setParameter(param.name, value, unit);
				sbx::dout(1) << "set " << param.name << " = " << value << " " << unit << "\n";
			} catch (sbx::ModelException& e) {
				fl_message("Invalid parameter setting\n%s", e.what());
			}
			model->configure();
		}
		if (block) {
			block->update();
			block->parent()->redraw();
		}
		change();
	} catch (sbx::ModelException& e) {
		fl_message("Failed to change values:\n%s", e.what());
		setModel(model.get());
	}
}

} // namespace
