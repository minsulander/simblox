
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sbx/ModelFactory.h>
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <sbx/Group.h>
#include <sbx/ModelVisitor.h>
#include <sbx/Simulation.h>
#include <sbx/PluginManager.h>
#include <sbx/Version.h>
#include <sbx/Ports.h>
#include <getopt.h>
#include <time.h>

using namespace std;
using namespace sbx;

void version()
{
	cout << "SimBlox v" << SIMBLOX_VERSION << "\n";
	cout << "  " << SIMBLOX_COPYRIGHT_NOTICE << "\n";
	cout << "  " << SIMBLOX_LICENSE_NOTICE << "\n";
}

void usage()
{
	cout << "usage: simblox [options] <datafile>\n"
		 << "  available options (all optional) are:\n"
		 << "    -h  -help              show this information and exit\n"
		 << "    -v  -version           show version information and exit\n"
		 << "    -d  -debug <level>     set debug level\n"
		 << "    -p  -datapath <path>   set path for data files\n"
		 << "         (also available through SIMBLOX_DATA_PATH environment variable)\n"
		 << "    -P  -pluginpath <path> set path for plugins\n"
		 << "         (also available through SIMBLOX_PLUGIN_PATH environment variable)\n"
		 << "    -A  -loadall           load all plugins at startup\n"
		 << "    -l  -list              list available models and exit \n"
		 << "    -s  -stats             print performance statistics when finished\n"
		 << "    -L  -logfile <file>    log to a file instead of stdout/stderr\n";
	exit(1);
}

XMLParser& parser = XMLParser::instance();
PluginManager& plugman = PluginManager::instance();

bool 	list = false,
		dostats = false,
		loadall = false;
const char 	*datapath = NULL, 
			*pluginpath = NULL,
			*logfile = NULL;

struct option long_options[] = {
	{"help", 0, 0, 'h'},
	{"version", 0, 0, 'v'},
	{"debug", 1, 0, 'd'},
	{"datapath", 1, 0, 'p'},
	{"pluginpath", 1, 0, 'P'},
	{"loadall", 0, 0, 'A'},
	{"list", 0, 0, 'l'},
	{"stats", 0, 0, 's'},
	{"logfile", 0, 0, 'L'},
	{0, 0, 0, 0}
};


int parseArguments(int *argc, char **argv)
{
	int option_index = 1;
    while (1) {
    	int i;
        int c = getopt_long_only(*argc,argv,"hd:p:P:AlsL:",long_options,&option_index);
        if (c == -1)
            break;
        switch (c) {
        	case 'h':
        		usage();
        		exit(0);
        		break;
        	case 'v':
        		version();
        		exit(0);
        		break;
			case 'd':
				if (optarg) {
					i = atoi(optarg);
					setDebugLevel(i);
				} else {
					dout(ERROR) << " need an argument for -d or -debug" << endl;
					usage();
					exit(1);
				}
				break;
			case 'p':
				datapath = optarg;
				break;
			case 'P':
				pluginpath = optarg;
				break;
			case 'A':
				loadall = true;
				break;
			case 'l':
				list = true;
				break;
			case 's':
				dostats = true;
				break;
			case 'L':
				logfile = optarg;
				break;
			default:
				usage();
				exit(1);
				break;
		}
	}
	return option_index;
}

void listModels()
{
	// List available models
	dout(1) << "Available factory models:\n";
	vector<string> objects = ModelFactory::instance().getIdentifierList();
	for (vector<string>::iterator i = objects.begin(); i != objects.end(); i++) {
		dout(1) << "  ";
		dout(0) << *i;
		if (getDebugLevel() >= 1) {
			smrt::ref_ptr<Model> model = ModelFactory::instance().create(*i);
			dout(2) << " (" << model->libraryName() << "::" << model->className() << ")";
			dout(1) << ": " << model->description();
			dout(0) << "\n";
			if (model->getMinimumUpdateFrequency() != 0)
				dout(2) << "    minimum update frequency " << model->getMinimumUpdateFrequency() << " Hz\n";
			for (int i = 0; i < model->getNumParameters(); i++) {
				std::string name = model->getParameterName(i);
				Model::Parameter param = model->getParameterSpec(name);
				dout(1) << "    parameter '" << name << "'";
				switch (param.type) {
					case Model::Parameter::BOOLEAN: dout(1) << " boolean"; break;
					case Model::Parameter::INTEGER: dout(1) << " integer"; break;
					case Model::Parameter::FLOAT: dout(1) << " float"; break;
					case Model::Parameter::DOUBLE: dout(1) << " double"; break;
					case Model::Parameter::STRING: dout(1) << " string"; break;
					default: dout(1) << "UNKNOWN ";
				}
				if (param.unit.length() > 0)
					dout(1) << " [" << param.unit << "]";
				if (param.description.length() > 0)
					dout(1) << ": " << param.description;
				dout(1) << "\n";
			}
			for (int i = 0; i < model->getNumPorts(); i++) {
				Port *port = model->getPort(i);
				dout(1) << "    " << (port->isInput() ? "input" : "output") << " '" 
						<< model->getPortName(port) << "'";
				if (port->getUnit().length() > 0)
					dout(1) << " [" << port->getUnit() << "]";
				if (model->getPortDescription(port).length() > 0)
					dout(1) << ": " << model->getPortDescription(port);
				dout(1) << "\n";
			}
			if (model->addInput() != NULL)
				dout(1) << "    multiple inputs\n";
			if (model->addOutput() != NULL)
				dout(1) << "    multiple outputs\n";
		} else
			dout(0) << "\n";
	}
}

class PrintStatsCallback : public VisitorCallback {
public:
	PrintStatsCallback(Simulation *sim) : simulation(sim)
	{
		initstats = sim->getInitVisitor().getStatistics();
		updatestats = sim->getUpdateVisitor().getStatistics();
		indentlevel = 0;
		printf("==== Statistics               Init   \t\tFreq\tUpdate [ms] ====\n");
	}
	virtual void apply(Model& model)
	{
		for (int i = 0; i < indentlevel; i++)
			printf("  ");
		printf(model.getName().c_str());
		for (int i = indentlevel*2+model.getName().length(); i < 30; i++)
			printf(" ");
		printf("%6.2f\t%d%%\t%d\t%6.2f\t%d%%\n",
				initstats[&model]*1000,
				(int)round(initstats[&model]/initstats[simulation->getRoot()]*100),
				(model.getUpdateFrequency() != 0 ? model.getUpdateFrequency() : simulation->getFrequency()),
				updatestats[&model]*1000,
				(int)round(updatestats[&model]/updatestats[simulation->getRoot()]*100));
	}
	virtual void apply(Group& group)
	{
		indentlevel--;
	}
	virtual void preTraverse(Group& group)
	{
		apply(*((Model*)&group));
		indentlevel++;
	}
protected:
	Simulation *simulation;
	VisitorTimeMap initstats, updatestats;
	int indentlevel;
};

int main(int argc, char* argv[])
{
	int exitcode = 0;
	try {
		parseArguments(&argc, argv);

		if (optind >= argc && !list) {
			usage();
			exit(1);
		}
		if (logfile)
			setLogFile(logfile);
		// Output date if logging to a file/stream
		if (getLogStream()) {
			time_t thetime = ::time(NULL);
			dout(1) << "Start " << ::ctime(&thetime) << "\n";
		}
		dout(1) << "SimBlox v" << SIMBLOX_VERSION << "\n";
		
		parser.getPath().add(".");
		if (argv[optind] && FilePath::dirname(argv[optind]).length() > 1)
			parser.getPath().add(FilePath::dirname(argv[optind]));
		if (datapath)
			parser.getPath().add(datapath);
		if (FilePath::dirname(argv[0]).length() > 1)
			parser.getPath().add(FilePath::dirname(argv[0]) + "/../data");
		dout(3) << "Data path: " << parser.getPath() << "\n";

		if (pluginpath)
			plugman.getPath().add(pluginpath);
		if (FilePath::dirname(argv[0]).length() > 1)
			plugman.getPath().add(FilePath::dirname(argv[0]) + "/../sbxPlugins");
		plugman.getPath().add(".");
		dout(3) << "Plugin path: " << plugman.getPath() << "\n";
		
		if (loadall)
			plugman.loadAllPlugins();
		
		if (list) {
			listModels();
			exit(0);
		}
		
		Group *root = parser.loadModels(argv[optind]);
		Simulation *sim = parser.loadSimulation(argv[optind]);
		sim->setRoot(root);
		if (dostats)
			sim->doStatistics();
			
		sim->run();
		
		if (dostats) {
			CallbackVisitor statsvis(new PrintStatsCallback(sim));
			statsvis.setTraversalMode(SEQUENTIAL);
			statsvis.visit(*sim->getRoot());
			dout(INFO) << "Total simulation time " << sim->getTime() << "\n";
			dout(INFO) << "Total real time " << sim->getRealTimeSinceStart() << "\n";
			dout(INFO) << "Average realtime ratio " << sim->getAverageRealTimeRatio() << "\n";
		}
		delete sim;
		
		//sim.setRoot(NULL);
		//root = NULL; // frees the memory (since it's a smart pointer)
		// Note: if any models from the plugin libraries have been instantiated, these need
		// to be deleted before we unload(), so perhaps it's safer to just not do this..
		plugman.unload();
	} catch (exception& e) {
		dout(ERROR) << "Caught " << e.what() << endl;
		exitcode = 3;
	} catch (...) {
		dout(ERROR) << "Caught something" << endl;
		exitcode = 4;
	}
	// Output date if logging to a file/stream
	if (getLogStream()) {
		time_t thetime = ::time(NULL);
		dout(1) << "End " << ::ctime(&thetime) << "\n";
	}
	closeLog();
	exit(exitcode);
		
}
