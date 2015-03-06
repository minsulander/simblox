#include "LinePlotter.h"
#include <sbx/XMLParser.h>
#include <sbx/ModelFactory.h>

namespace sbxGUI {
	
	REGISTER_Object(sbxGUI, LinePlotter);
	
	float default_colors[] = {
		0.7,0,0,1,
		0,0.7,0,1,
		0,0,0.7,1,
		0.5,0.5,0,1,
		0,0.5,0.5,1,
		0.5,0,0.5,1
	};
	
	LinePlot2DWindow::LinePlot2DWindow(int x, int y, int w, int h, char *title) :
		Fl_Gl_Window(x,y,w,h,title)
	{
		mode(FL_RGB|FL_ALPHA|FL_DEPTH|FL_DOUBLE);
		Fl_Color col = color();
		unsigned char r,g,b;
		Fl::get_color(col,r,g,b);
		clearcolor = Eigen::Vector4f(r/255.0, g/255.0, b/255.0, 1);
	}
	
	void LinePlot2DWindow::init() {
		glViewport(0,0,w(),h());
		glClearColor(clearcolor[0], clearcolor[1], clearcolor[2], clearcolor[3]);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH);
	}
	
	void LinePlot2DWindow::draw() {
		if (!valid()) {
			valid(1);
			init();
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// view transformations
		glMatrixMode(GL_PROJECTION);
   		glLoadIdentity();
		glOrtho(-1,1,-1,1,-1,1);
   		glMatrixMode(GL_MODELVIEW);
   		glLoadIdentity();
		
		glColor3f(0,1,0);
		if (plot)
			plot->draw();
	}
	
	LinePlotter::LinePlotter(const std::string& name)
	:	sbx::MIModel<double>(name),
		window(NULL),
		start_shown(false),
		t(0),
		max_points(0)
	{
		registerParameter(&max_points, Parameter::UINTEGER, "max_points", "", "Maximum data points of each dataset");
		createWindow();
	}
	
	LinePlotter::LinePlotter(const LinePlotter& source)
	:	sbx::MIModel<double>(source),
		window(NULL),
		start_shown(source.start_shown),
		t(source.t),
		max_points(source.max_points)
	{
		copyParameter(source, "max_points", &max_points);
	}
	
	void LinePlotter::parseXML(const TiXmlElement *element)
	{
		Model::parseXML(element);
		createWindow();
		if (element->FirstChildElement("color")) {
			Eigen::Vector3d rgb = sbx::XMLParser::parseVector3(element,"color");
			window->color(fl_rgb_color((uchar)(rgb.x()*255), (uchar)(rgb.y()*255), (uchar)(rgb.z()*255)));
		}
		window->position(sbx::XMLParser::parseInt(element, "position", "", true, window->x(), "x"),
						 sbx::XMLParser::parseInt(element, "position", "", true, window->y(), "y"));
		window->size(sbx::XMLParser::parseInt(element, "size", "", true, window->w(), "w"),
					 sbx::XMLParser::parseInt(element, "size", "", true, window->h(), "h"));
		start_shown = sbx::XMLParser::parseBoolean(element, "visible", true, false);
	}
	
	void LinePlotter::writeXML(TiXmlElement *element)
	{
		sbx::XMLParser::setInt(element, "position", window->x(), "x");
		sbx::XMLParser::setInt(element, "position", window->y(), "y");
		sbx::XMLParser::setInt(element, "size", window->w(), "w");
		sbx::XMLParser::setInt(element, "size", window->h(), "h");
		if (window->visible())
			sbx::XMLParser::setBoolean(element, "visible", true);
		Model::writeXML(element);
	}
	
	void LinePlotter::configure()
	{
		if (start_shown)
			show();
	}
	
	void LinePlotter::init()
	{
		t = 0;
		for (unsigned int i = 0; i < datasets.size(); i++)
			datasets[i]->clear();
	}
	
	void LinePlotter::update(const double dt)
	{
		t += dt;
		for (unsigned int i = 0; i < inputs.size(); i++) {
			if (datasets.size() < i+1) {
				DataSet *data = new DataSet;
				if (inputs[i]->getOwner()->getPortName(inputs[i]).substr(0,2) != "in" || !inputs[i]->isConnected())
					data->label = getPortName(inputs[i]);
				else {
					std::stringstream ss;
					if (inputs[i]->getOtherEnd()->getOwner())
						ss << inputs[i]->getOtherEnd()->getOwner()->getName() << ".";
					ss << inputs[i]->getOtherEnd()->getOwner()->getPortName(inputs[i]->getOtherEnd());
					data->label = ss.str();
				}
				/*
				if (i > sizeof(default_colors)/(4*sizeof(float)))
					sbx::dout(sbx::WARN) << "LinePlotter is out of colors...\n";
				else {
				*/
					float *col = (default_colors+i*4);
					data->color = Eigen::Vector4f(col[0],col[1],col[2],col[3]);
				//}
				datasets.push_back(data);
				plot.add(datasets[datasets.size()-1]);
			}
			if (inputs[i]->isConnected()) {
				datasets[i]->append(t, inputs[i]->get());
			}
			if (max_points > 0 && datasets[i]->x.size() > max_points) {
				datasets[i]->x.erase(datasets[i]->x.begin());
				datasets[i]->y.erase(datasets[i]->y.begin());
			}
		}		
	}
	
	void LinePlotter::display(const sbx::DisplayMode mode)
	{
		if (!window || !window->visible())
			return;
		window->redraw();
	}
	
	void LinePlotter::show()
	{
		createWindow();
		window->show();
		window->copy_label(getName().c_str());
		display(sbx::DISPLAY_USER);
	}
	
	void LinePlotter::hide()
	{
		if (window)
			window->hide();
	}
	
	void LinePlotter::createWindow()
	{
		if (!window) {
			window = new LinePlot2DWindow(0,0,300,200);
			window->copy_label(getName().c_str());
			window->setPlot(&plot);
			plot.setAxisColor(Eigen::Vector4f(0,0,0,1));
			plot.setTextColor(Eigen::Vector4f(0,0,0,1));
			plot.setGridColor(Eigen::Vector4f(0,0,0,0.5));
			window->end();
			window->resizable(window);
		}
	}
	
	LinePlotter::~LinePlotter()
	{
		if (window)
			window->hide();
		window = NULL;
	}
	
} // namespace sbxGUI
