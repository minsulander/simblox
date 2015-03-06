#include "StatisticsWindow.h"
#include "SimulationEditor.h"
#include "global.h"
#include <sbx/ModelVisitor.h>
#include <sbx/XMLParser.h>

namespace sbxBuilder {
	
	int widths[] = { 150, 50, 50, 50, 50, 50, 50, 50, 0 };

	class PrintStatsCallback : public sbx::VisitorCallback {
	public:
		PrintStatsCallback(sbx::Simulation *sim, Fl_Browser *brows) : simulation(sim), browser(brows)
		{
			initstats = sim->getInitVisitor().getStatistics();
			updatestats = sim->getUpdateVisitor().getStatistics();
			displaystats = sim->getDisplayVisitor().getStatistics();
			indentlevel = 0;
		}
		virtual void apply(sbx::Model& model)
		{
			std::stringstream ss;
			for (int i = 0; i < indentlevel; i++)
				ss << "  ";
			ss << model.getName();
			ss << "\t" << (model.getUpdateFrequency() != 0 ? model.getUpdateFrequency() : simulation->getFrequency());
			if (initstats[&model] != 0) {
				ss << "\t"<< initstats[&model]*1000;
				ss << "\t" << (int)round(initstats[&model]/initstats[simulation->getRoot()]*100);
			} else
				ss << "\t\t";
			if (updatestats[&model] != 0) {
				ss << "\t" << updatestats[&model]*1000;
				ss << "\t" << (int)round(updatestats[&model]/updatestats[simulation->getRoot()]*100);
			} else
				ss << "\t\t";
			if (displaystats[&model] != 0) {
				ss << "\t" << displaystats[&model]*1000;
				ss << "\t" << (int)round(displaystats[&model]/displaystats[simulation->getRoot()]*100);
			} else
				ss << "\t\t";			
			browser->add(ss.str().c_str());
		}
		virtual void apply(sbx::Group& group)
		{
			indentlevel--;
		}
		virtual void preTraverse(sbx::Group& group)
		{
			apply(*((sbx::Model*)&group));
			indentlevel++;
		}
	protected:
		sbx::Simulation *simulation;
		sbx::VisitorTimeMap initstats, updatestats, displaystats;
		Fl_Browser *browser;
		int indentlevel;
	};

	StatisticsWindow::StatisticsWindow(int X, int Y, int W, int H)
	: Fl_Double_Window(X,Y,W,H, "Statistics")
	{
		browser = new Fl_Browser(0,0,W,H-20);
		browser->column_widths(widths);
		browser->textsize(12);
		resizable(browser);
		end();
	}
	
	void StatisticsWindow::parseXML(const TiXmlElement *element)
	{
		position(sbx::XMLParser::parseInt(element, "position", "", true, x(), "x"),
				 sbx::XMLParser::parseInt(element, "position", "", true, y(), "y"));
		size(sbx::XMLParser::parseInt(element, "size", "", true, w(), "w"),
			 sbx::XMLParser::parseInt(element, "size", "", true, h(), "h"));
		if (sbx::XMLParser::parseBoolean(element, "visible", true, false))
			show();
	}
	
	void StatisticsWindow::writeXML(TiXmlElement *element)
	{
		sbx::XMLParser::setInt(element, "position", x(), "x");
		sbx::XMLParser::setInt(element, "position", y(), "y");
		sbx::XMLParser::setInt(element, "size", w(), "w");
		sbx::XMLParser::setInt(element, "size", h(), "h");
		if (visible())
			sbx::XMLParser::setBoolean(element, "visible", true);
	}
	
	void StatisticsWindow::update()
	{
		if (!simulation_window || !simulation_window->getSimulation())
			return;
		
		sbx::Simulation *sim = simulation_window->getSimulation();

		sim->doStatistics(true);
		browser->clear();
		browser->add("@bModel\t@bFreq\t@bInit\t@b%\t@bUpdate\t@b%\t@bDisplay\t@b%");
		
		sbx::CallbackVisitor statsvis(new PrintStatsCallback(sim, browser));
		statsvis.setTraversalMode(sbx::SEQUENTIAL);
		statsvis.visit(*sim->getRoot());
		
		std::stringstream ss;
		ss << "Total\t" << sim->getFrequency() << "\t"
			<< sim->getInitVisitor().getTotalTime() << "\t\t"
			<< sim->getUpdateVisitor().getTotalTime() << "\t\t"
			<< sim->getDisplayVisitor().getTotalTime();
		browser->add(ss.str().c_str());
		browser->add("@-");
		ss.str("");
		ss << "Total simulation time " << sim->getTime();
		browser->add(ss.str().c_str());
		ss.str(""); ss << "Total real time " << sim->getRealTimeSinceStart();
		browser->add(ss.str().c_str());
		ss.str(""); ss << "Average realtime ratio " << sim->getAverageRealTimeRatio();
		browser->add(ss.str().c_str());
	}

} // namespace
