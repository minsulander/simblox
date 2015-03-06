#include "global.h"
#include "BlockCanvasWindow.h"
#include "SimulationEditor.h"
#include "ModelEditor.h"
#include "ModelBlock.h"
#include "StatisticsWindow.h"
#include <sbx/XMLParser.h>
#include <sbx/ModelFactory.h>
#include <sbx/Log.h>
#include <FL/Fl_File_Chooser.H>

namespace sbxBuilder {
	
	bool newFile()
	{
		if (!canvas_window || !canvas_window->getRootCanvas())
			return false;
		if (changes_made && !saveChoice())
			return false;
		if (model_window)
			model_window->setModel(NULL);
		canvas_window->getRootCanvas()->newGroup();
		simulation_window->newSimulation(canvas_window->getRootCanvas()->getGroup());
		filename = "";
		changes_made = false;
		canvas_window->updateTitle();
		return true;
	}
	
	bool loadFile()
	{
		if (!canvas_window || !canvas_window->getRootCanvas())
			return false;
		if (changes_made && !saveChoice())
			return false;
		const char *file = fl_file_chooser("Open SimBlox Builder file", "*.xml", 0, 0);
		if (file) {
			if (model_window)
				model_window->setModel(NULL);
			load(file);
			filename = file;
			changes_made = false;
			canvas_window->updateTitle();
			return true;
		}
		return false;
	}
	
	bool saveFile()
	{
		if (!canvas_window || !canvas_window->getRootCanvas())
			return false;
		if (filename.length() == 0) {
			const char *file = fl_file_chooser("Save SimBlox Builder file", "*.xml", 0, 0);
			if (file)
				filename = file;
			else
				return false;
		}
		save(filename);
		changes_made = false;
		canvas_window->updateTitle();
		return true;
	}
	
	bool saveAs()
	{
		std::string oldfilename = filename;
		filename = "";
		if (!saveFile()) {
			filename = oldfilename;
			return false;
		}
		return true;
	}
	
	bool saveChoice()
	{
		if (changes_made) {
			std::string question = "Do you want to save changes";
			if (filename.length() > 0)
				question += " to '" + filename + "'";
			question += "?";
			int choice = fl_choice(question.c_str(), "Cancel", "Save", "Don't Save");
			if (choice == 0)
				return false;
			else if (choice == 1 && !saveFile())
				return false;
		}
		return true;
	}
	
	void load(const std::string& infilename)
	{
		std::string fname = sbx::XMLParser::instance().getPath().find(infilename);
		if (fname.length() == 0)
			fname = infilename;
		canvas_window->getRootCanvas()->setGroup(sbx::XMLParser::instance().loadModels(fname));
		sbx::Simulation *simulation = sbx::XMLParser::instance().loadSimulation(fname);
		simulation->setRoot(canvas_window->getRootCanvas()->getGroup());
		simulation_window->setSimulation(simulation);
		
		TiXmlDocument doc(fname);
		bool ok = doc.LoadFile();
		if (!ok)
			throw sbx::ParseException(doc.ErrorDesc(), &doc);
		sbx::dout(1) << "Parsing canvas from " << fname << "\n";
		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem->Value() || std::string(pElem->Value()) != "SimBlox")
			throw sbx::ParseNoElementException("No top-level SimBlox element", &doc);
		
		TiXmlElement *canvasElement = pElem->FirstChildElement("BlockCanvas");
		if (canvasElement)
			canvas_window->parseXML(canvasElement);
		
		TiXmlElement *simElement = pElem->FirstChildElement("SimulationEditor");
		if (simElement)
			simulation_window->parseXML(simElement);
		
		TiXmlElement *modElement = pElem->FirstChildElement("ModelEditor");
		if (modElement)
			model_window->parseXML(modElement);
		
		TiXmlElement *statsElement = pElem->FirstChildElement("StatisticsWindow");
		if (statsElement)
			stats_window->parseXML(statsElement);
		
		canvas_window->getRootCanvas()->update();
		filename = fname;
	}
	
	void save(const std::string& filename)
	{
		TiXmlDocument doc(filename);
		TiXmlHandle hDoc(&doc);
		TiXmlElement *root = new TiXmlElement("SimBlox");
		doc.LinkEndChild(root);
		if (canvas_window->getRootCanvas()->getGroup()) {
			TiXmlElement *modelsElement = new TiXmlElement("Models");
			canvas_window->getRootCanvas()->getGroup()->writeXML(modelsElement);
			root->LinkEndChild(modelsElement);
		}
		
		if (simulation_window->getSimulation()) {
			TiXmlElement *simulationElement = new TiXmlElement("Simulation");
			simulation_window->getSimulation()->writeXML(simulationElement);
			root->LinkEndChild(simulationElement);
		}
		
		TiXmlElement *canvasElement = new TiXmlElement("BlockCanvas");
		canvas_window->writeXML(canvasElement);
		root->LinkEndChild(canvasElement);
		
		TiXmlElement *simElement = new TiXmlElement("SimulationEditor");
		simulation_window->writeXML(simElement);
		root->LinkEndChild(simElement);
		
		TiXmlElement *modElement = new TiXmlElement("ModelEditor");
		model_window->writeXML(modElement);
		root->LinkEndChild(modElement);
		
		if (stats_window) {
			TiXmlElement *statsElement = new TiXmlElement("StatisticsWindow");
			stats_window->writeXML(statsElement);
			root->LinkEndChild(statsElement);
		}
		
		doc.SaveFile(filename);
	}
	
	void change()
	{
		changes_made = true;
		canvas_window->updateTitle();
	}
	
	void makeNewModelsMenu(Fl_Menu_ *menu, Fl_Callback *callback, void* data)
	{
		// List available models
		std::vector<std::string> objects = sbx::ModelFactory::instance().getIdentifierList();
		for (std::vector<std::string>::iterator i = objects.begin(); i != objects.end(); i++) {
			smrt::ref_ptr<sbx::Model> model = sbx::ModelFactory::instance().create(*i);
			if (!model)
				throw sbx::ModelException("Factory failed to create model of type '" + *i + "'");
			std::string str = std::string("New/") + model->libraryName() + "/" + model->className();
			menu->add(str.c_str(), 0, callback, data);
			sbx::dout(1) << str << "\n";
		}
	}
	
	bool newModelAction(const std::string& path, int x, int y)
	{
		if (!canvas_window->getRootCanvas()->getGroup() && !newFile())
			return false;
		BlockCanvas *current_canvas = canvas_window->getCurrentCanvas();
		smrt::ref_ptr<sbx::Model> model = sbx::ModelFactory::instance().create(path.substr(4));
		if (model.valid() && current_canvas && current_canvas->getGroup()) {
			unsigned int name_index = 0;
			for (unsigned int i = 0; i < current_canvas->getGroup()->getNumChildren(); i++) {
				if (model->getName() == current_canvas->getGroup()->getChild(i)->getName().substr(0,model->getName().length()))
					name_index++;
			}
			if (name_index > 0) {
				std::stringstream namess;
				namess << model->getName() << name_index;
				model->setName(namess.str());
			}
			current_canvas->getGroup()->addChild(model.get());
			current_canvas->update();
			ModelBlock *block = ModelBlock::blocks[model.get()];
			if (block) {
				if (x || y)
					block->position(x,y);
				block->select();
				change();
			}
			model->show();
			model_window->setModel(model.get());
			/// \todo find free space in the canvas for the new model block
			return true;
		}
		return false;
	}
	
	BlockCanvasWindow *canvas_window = NULL;
	SimulationEditor *simulation_window = NULL;
	ModelEditor *model_window = NULL;
	StatisticsWindow *stats_window = NULL;
	std::string filename = "";
	bool changes_made = false;
	
} // namespace
