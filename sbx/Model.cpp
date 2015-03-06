#include "Model.h"
#include "Ports.h"
#include "Group.h"
#include "BlackBox.h"
#include "XMLParser.h"
#include "Log.h"
#include "ModelVisitor.h"
#include <numerix/misc.h>

namespace sbx
{
	
	Model::Model(const std::string& name) :
		Object(name),
		parent(NULL),
		warnflag(false),
		errflag(false)
	{
		// Register blackbox variables
		BlackBox::instance().beginGroup(this,"Model");
		BlackBox::instance().registerBool("errflag",&errflag);
		BlackBox::instance().registerBool("warnflag",&warnflag);
		BlackBox::instance().endGroup();
		update_frequency = getMinimumUpdateFrequency();
	}
	
	Model::Model(const Model& source) :
		Object(source),
		parent(NULL),
		warnflag(source.warnflag),
		warnstr(source.warnstr),
		errflag(source.errflag),
		errstr(source.errstr),
		update_frequency(source.update_frequency)
	{
	}
	
	Model::~Model()
	{
		//std::cout << "delete " << getName() << "\n";
	}
	
	/** This method can be used to display data, typically by endpoint models, e.g. a plotter or
	 logger. For realtime simulations it will be called after each update(), while for 
	 non-realtime simulations it may be called periodically, or at the end of the simulation 
	 run to show results.
	 \see isEndPoint()
	 */
	void Model::display(const DisplayMode mode) {}
	
	void Model::parseXML(const TiXmlElement* element)
	{
		int ivalue;
		double value;
		std::string str;
		if (element->Attribute("name"))
			setName(element->Attribute("name"));
		dout(3) << "  name '" << getName() << "'\n";
		if (element->Attribute("frequency")) {
			setUpdateFrequency(atoi(element->Attribute("frequency")));
			dout(3) << "  frequency " << update_frequency << "\n";
		}
		if (supportsDynamicInputs()) {
			int inputs = XMLParser::parseInt(element, "numinputs", "", true, 0);
			for (int i = 0; i < inputs; i++)
				addInput();
		}
		if (supportsDynamicOutputs()) {
			int outputs = XMLParser::parseInt(element, "numoutputs", "", true, 0);
			for (int i = 0; i < outputs; i++)
				addOutput();
		}
		
		for (ParameterList::iterator i = parameters.begin(); i != parameters.end(); i++) {
			Parameter& param = i->second;
			try {
				switch (param.type) {
					case Parameter::BOOLEAN:
						ivalue = XMLParser::parseBoolean(element,param.name);
						*((bool*)param.ptr) = ivalue;
						dout(3) << "  param " << param.name << " = " << (ivalue?"true":"false") << " " << param.unit << "\n";
						break;
					case Parameter::INTEGER:
						ivalue = XMLParser::parseInt(element,param.name,param.unit);
						*((int*)param.ptr) = ivalue;
						dout(3) << "  param " << param.name << " = " << ivalue << " " << param.unit << "\n";
						break;
					case Parameter::UINTEGER:
						ivalue = XMLParser::parseInt(element,param.name,param.unit);
						*((unsigned int*)param.ptr) = ivalue;
						dout(3) << "  param " << param.name << " = " << ivalue << " " << param.unit << "\n";
						break;
					case Parameter::FLOAT:
						value = XMLParser::parseDouble(element,param.name,param.unit);
						*((float*)param.ptr) = value;
						dout(3) << "  param " << param.name << " = " << value << " " << param.unit << "\n";
						break;
					case Parameter::DOUBLE:
						value = XMLParser::parseDouble(element,param.name,param.unit);
						*((double*)param.ptr) = value;
						dout(3) << "  param " << param.name << " = " << value << " " << param.unit << "\n";
						break;
					case Parameter::STRING:
						str = XMLParser::parseString(element,param.name);
						*((std::string*)param.ptr) = str;
						dout(3) << "  param " << param.name << " = '" << str << "'\n";
						break;
				}
				param.parsed = true;
			} catch (ParseNoElementException& e) {
				param.parsed = false;
			}
		}
	}
	
	void Model::writeXML(TiXmlElement *element)
	{
		element->SetAttribute("name",getName());
		if (update_frequency)
			element->SetAttribute("frequency", update_frequency);
		if (supportsDynamicInputs())
			XMLParser::setInt(element, "numinputs", getNumInputs());
		if (supportsDynamicOutputs())
			XMLParser::setInt(element, "numoutputs", getNumOutputs());
		for (ParameterList::iterator i = parameters.begin(); i != parameters.end(); i++) {
			Parameter& param = i->second;
			switch (param.type) {
				case Parameter::BOOLEAN:
					XMLParser::setBoolean(element, param.name, *((bool*)param.ptr));
					break;
				case Parameter::INTEGER:
					XMLParser::setInt(element, param.name, *((int*)param.ptr));
					break;
				case Parameter::UINTEGER:
					XMLParser::setInt(element, param.name, *((unsigned int*)param.ptr));
					break;
				case Parameter::FLOAT:
					XMLParser::setDouble(element, param.name, *((int*)param.ptr));
					break;
				case Parameter::DOUBLE:
					XMLParser::setDouble(element, param.name, *((double*)param.ptr));
					break;
				case Parameter::STRING:
					XMLParser::setString(element, param.name, *((std::string*)param.ptr));
					break;
			}
			if (param.unit.length() > 0)
				XMLParser::setString(element, param.name, param.unit, "unit");
		}
	}
	
	Port* Model::addInput()
	{
		return NULL;
		//throw PortException("Model '" + getName() + "' does not allow allocation of new input ports");
	}
	
	Port* Model::addOutput()
	{
		return NULL;
		//throw PortException("Model '" + getName() + "' does not allow allocation of new output ports");
	}
	
	void Model::removeInput() { }
	void Model::removeOutput() { }
	bool Model::supportsDynamicInputs() { return false; }
	bool Model::supportsDynamicOutputs() { return false; }
	
	ModelOrderList& Model::getDataProviders()
	{
		if (traversalProviders.size() > 0)
			return traversalProviders;
		std::map<Model*,bool> handled;
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port && port->isInput() && port->isConnected()) {
				InputPort* inp = (InputPort*) port;
				Model* provider = inp->getOtherEnd()->getOwner();
				if (provider && !handled[provider]) {
					traversalProviders.push_back(provider);
					handled[provider] = true;
				}
			}
		}
		return traversalProviders;
	}
	
	ModelOrderList& Model::getDataDependants()
	{
		if (traversalDependants.size() > 0)
			return traversalDependants;
		std::map<Model*,bool> handled;
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port && port->isOutput() && port->isConnected()) {
				OutputPort* outp = (OutputPort*) port;
				for (int j = 0; j < outp->getNumConnections(); j++) {
					Model* dependant = outp->getOtherEnd(j)->getOwner();
					if (dependant && !handled[dependant]) {
						traversalDependants.push_back(dependant);
						handled[dependant] = true;
					}
				}
			}
		}
		return traversalDependants;
	}
	
	const bool Model::dependsOn(Model* other)
	{
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isInput() && port->isConnected()) {
				InputPort* inp = (InputPort*) port;
				if (inp->getOtherEnd()->getOwner() == other)
					return true;
			}
		}
		return false;		
	}
	
	const bool Model::dependsLooselyOn(Model* other)
	{
		bool depends = false;
		bool all_loose = true;
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isInput() && port->isConnected()) {
				InputPort* inp = (InputPort*) port;
				if (inp->getOtherEnd()->getOwner() == other) {
					depends = true;
					if (!inp->isLoose())
						all_loose = false;
				}
			}
		}
		return (depends && all_loose);		
	}
	
	const bool Model::dependsStronglyOn(Model* other)
	{
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isInput() && port->isConnected()) {
				InputPort* inp = (InputPort*) port;
				if (inp->getOtherEnd()->getOwner() == other && !inp->isLoose())
					return true;
			}
		}
		return false;		
	}
	
	const bool Model::providesFor(Model* other)
	{
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isOutput() && port->isConnected()) {
				OutputPort* outp = (OutputPort*) port;
				for (int j = 0; j < outp->getNumConnections(); j++) {
					if (outp->getOtherEnd(j)->getOwner() == other)
						return true;
				}
			}
		}
		return false;
	}
	
	const bool Model::hasDataProviders()
	{
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isInput() && port->isConnected())
				return true;
		}
		return false;
	}
	
	const bool Model::hasDataDependants()
	{
		for (PortList::iterator i = ports.begin(); i != ports.end(); i++) {
			Port* port = i->second;
			if (port->isOutput() && port->isConnected())
				return true;
		}
		return false;
	}
	
	const bool Model::hasEndPointDependants()
	{
		ModelOrderList& dependants = getDataDependants();
		for (ModelOrderList::iterator i = dependants.begin(); i != dependants.end(); i++) {
			if ((*i)->isEndPoint())
				return true;
			if ((*i)->hasEndPointDependants())
				return true;
		}
		return false;
	}
	
	void Model::accept(ModelVisitor& visitor)
	{
		if (visitor.getTraversalMode() != DEPENDENT || isEndPoint() || hasEndPointDependants())
			visitor.apply(*this);
		else if (visitor.getVisitCount() == 0)
			dout(5) << "skip " << getName() << "\n";
	}
	
	void Model::traverse(ModelVisitor& visitor)
	{
		if (visitor.getTraversalMode() == DEPENDENT) {
			ModelOrderList& deps = getDataProviders();
			for (ModelOrderList::iterator i = deps.begin(); i < deps.end(); i++) {
				if (*i != this) // cyclic dependency - TODO warn?
					(*i)->accept(visitor);
			}
		}
	}
	
	void Model::setName(const std::string& newname)
	{
		if (parent) {
			for (unsigned int i = 0; i < parent->getNumChildren(); i++) {
				if (parent->getChild(i) != this && parent->getChild(i)->getName() == newname)
					throw ModelException("Attempt to set name to sibling with equal name", this);
			}
		}
		Object::setName(newname);
	}
	
	void Model::setError(const std::string& error)
	{
		if (error.length() > 0) {
			errflag = true;
			errstr = error;
		} else {
			errflag = false;
			errstr = "";
		}
	}
	
	void Model::setWarning(const std::string& warning)
	{
		if (warning.length() > 0) {
			warnflag = true;
			warnstr = warning;
		} else {
			warnflag = false;
			warnstr = "";
		}
	}
	
	void Model::setUpdateFrequency(const int freq)
	{
		if (freq < getMinimumUpdateFrequency()) {
			std::stringstream ss;
			ss << "Specified update frequency (" << freq << " Hz) is lower than model minimum (" << getMinimumUpdateFrequency() << " Hz)";
			throw ModelException(ss.str(), this);
		}
		update_frequency = freq;
	}
	
	Port* Model::getPort(const std::string& name)
	{
		PortList::iterator it = ports.find(name);
		if (it != ports.end())
			return it->second;
		return NULL;
	}
	
	const Port* Model::getPort(const std::string& name) const
	{ 
		PortList::const_iterator it = ports.find(name);
		if (it != ports.end())
			return it->second;
		return NULL;
	}
	
	Port* Model::getPort(const unsigned int index)
	{
		if (index >= ports.size())
			return NULL;
		Port* port;
		PortList::iterator i = ports.begin();;
		for (unsigned int count = 0; count < index; count++)
			i++;
		port = i->second;
		return port;
	}
	
	/// \throw ModelException if \a port is not owned by this model
	unsigned int Model::getPortIndex(const Port *port)
	{
		PortList::iterator i = ports.begin();;
		for (unsigned int count = 0; count < ports.size(); count++) {
			if (i->second == port)
				return count;
			i++;
		}
		throw ModelException("Specified port is not owned by this model", this);
	}
	
	unsigned int Model::getNumInputs() const
	{
		unsigned int count = 0;
		for (PortList::const_iterator i = ports.begin(); i != ports.end(); i++)
			if (i->second->isInput())
				count++;
		return count;
	}
	
	unsigned int Model::getNumOutputs() const
	{
		unsigned int count = 0;
		for (PortList::const_iterator i = ports.begin(); i != ports.end(); i++)
			if (i->second->isOutput())
				count++;
		return count;
	}
	
	/// \throw ModelException if \a index is out of bounds
	std::string Model::getPortName(const unsigned int index) const
	{
		if (index >= ports.size())
			throw ModelException("Invalid port index", this);
		PortList::const_iterator i = ports.begin();;
		for (unsigned int count = 0; count < index; count++)
			i++;
		return i->first;
	}
	
	/// \throw ModelException if \a port is not owned by this model
	std::string Model::getPortName(const Port *port) const
	{
		for (PortList::const_iterator i = ports.begin(); i != ports.end(); i++)
			if (i->second == port)
				return i->first;
		throw ModelException("Specified port is not owned by this model", this);
	}
	
	/// \throw ModelException if \a port is not owned by this model
	std::string Model::getPortDescription(const Port *port) const
	{
		PortDescriptionList::const_iterator it = port_descriptions.find(port);
		if (it == port_descriptions.end())
			throw ModelException("Specified port is not owned by this model", this);
		return it->second;
	}
	
	bool Model::ownsPort(const Port *port) const
	{
		for (PortList::const_iterator i = ports.begin(); i != ports.end(); i++)
			if (i->second == port)
				return true;
		return false;
	}
	
	void Model::onPortConnect(Port *port, Port *otherend)
	{
		traversalProviders.clear();
		traversalDependants.clear();
	}
	
	void Model::onPortDisconnect(Port *port, Port *otherend)
	{
		traversalProviders.clear();
		traversalDependants.clear();
	}
	
	void Model::setParent(Group *newparent)
	{
		parent = newparent;
	}
	
	Group* Model::getRoot()
	{
		if (!parent)
			return asGroup();
		Group *group = parent;
		while (group->getParent())
			group = group->getParent();
		return group;
	}
	
	/// \see FindVisitor::findModelByPath()
	std::string Model::getPath(const Group *relative) const
	{
		std::string path = getName();
		const Group *group = getParent();
		if (!group)
			path.insert(0, "/");
		while (group && group != relative) {
			if (group->getParent())
				path.insert(0, group->getName() + "/");
			else
				path.insert(0, "/");
			group = group->getParent();
		}
		return path;
	}
	
	std::string Model::getParameterName(const unsigned int index) const
	{
		if (index >= parameters.size())
			throw ModelException("Invalid parameter index");
		ParameterList::const_iterator i = parameters.begin();
		for (unsigned int count = 0; count < index; count++)
			i++;
		return i->first;
	}
	
	const Model::Parameter& Model::getParameterSpec(const std::string& name) const
	{
		ParameterList::const_iterator it = parameters.find(name);
		if (it == parameters.end())
			throw ModelException("Unknown parameter '" + name + "'");
		return it->second;
	}
	
	const Model::Parameter& Model::getParameterSpec(const unsigned int index) const
	{
		return getParameterSpec(getParameterName(index));
	}
	
	void Model::getParameter(const std::string& name, bool& b)
	{
		if (parameters[name].type != Parameter::BOOLEAN)
			throw ModelException(std::string("Parameter '") + name + "' is not a boolean", this);
		b = *((bool*)parameters[name].ptr);
	}
	
	void Model::getParameter(const std::string& name, int& i, const std::string& unit)
	{
		if (parameters[name].type != Parameter::INTEGER)
			throw ModelException(std::string("Parameter '") + name + "' is not a int", this);
		i = *((int*)parameters[name].ptr);
		if (unit.length() > 0)
			i = (int)round(units::convert(i,parameters[name].unit,unit));
	}
	
	void Model::getParameter(const std::string& name, unsigned int& i, const std::string& unit)
	{
		if (parameters[name].type != Parameter::UINTEGER)
			throw ModelException(std::string("Parameter '") + name + "' is not an unsigned int", this);
		i = *((unsigned int*)parameters[name].ptr);
		if (unit.length() > 0)
			i = (unsigned int)round(units::convert(i,parameters[name].unit,unit));
	}
	
	void Model::getParameter(const std::string& name, float& f, const std::string& unit)
	{
		if (parameters[name].type != Parameter::FLOAT)
			throw ModelException(std::string("Parameter '") + name + "' is not a float", this);
		f = *((float*)parameters[name].ptr);
		if (unit.length() > 0)
			f = (float)units::convert(f,parameters[name].unit,unit);
	}
	
	void Model::getParameter(const std::string& name, double& d, const std::string& unit)
	{
		if (parameters[name].type != Parameter::DOUBLE)
			throw ModelException(std::string("Parameter '") + name + "' is not a double", this);
		d = *((double*)parameters[name].ptr);
		if (unit.length() > 0)
			d = units::convert(d,parameters[name].unit,unit);
	}
	
	void Model::getParameter(const std::string& name, std::string& s)
	{
		if (parameters[name].type != Parameter::STRING)
			throw ModelException(std::string("Parameter '") + name + "' is not a string", this);
		s = *((std::string*)parameters[name].ptr);
	}
	
	std::string Model::getParameter(const std::string& name)
	{
		std::stringstream ss;
		switch (parameters[name].type) {
			case Parameter::BOOLEAN: {
				bool b;
				getParameter(name,b);
				ss << b;
				break;
			}
			case Parameter::INTEGER: {
				int i;
				getParameter(name,i);
				ss << i;
				break;
			}
			case Parameter::UINTEGER: {
				unsigned int i;
				getParameter(name,i);
				ss << i;
				break;
			}
			case Parameter::FLOAT: {
				float f;
				getParameter(name,f);
				ss << f;
				break;
			}
			case Parameter::DOUBLE: {
				double d;
				getParameter(name,d);
				ss << d;
				break;
			}
			case Parameter::STRING: {
				std::string s;
				getParameter(name,s);
				return s;
				break;
			}
		}
		return ss.str();
	}
	
	void Model::setParameter(const std::string& name, const double value, const std::string& unit)
	{
		double d = value;
		if (parameters[name].unit.length() > 0 && unit.length() > 0) {
			d = units::convert(value, unit, parameters[name].unit);		
		}
		switch (parameters[name].type) {
			case Parameter::BOOLEAN:
				*((bool*)parameters[name].ptr) = (bool) d;
				break;
			case Parameter::INTEGER:
				*((int*)parameters[name].ptr) = (int) round(d);
				break;
			case Parameter::UINTEGER:
				*((unsigned int*)parameters[name].ptr) = (unsigned int) round(d);
				break;
			case Parameter::FLOAT:
				*((float*)parameters[name].ptr) = (float) d;
				break;
			case Parameter::DOUBLE:
				*((double*)parameters[name].ptr) = d;
				break;
			case Parameter::STRING:
				std::stringstream ss;
				ss << d;
				*((std::string*)parameters[name].ptr) = ss.str();
				break;
		}
	}
	
	void Model::setParameter(const std::string& name, const std::string& value, const std::string& unit)
	{
		switch (parameters[name].type) {
			case Parameter::BOOLEAN:
			case Parameter::INTEGER:
			case Parameter::UINTEGER:
			case Parameter::FLOAT:
			case Parameter::DOUBLE:
				setParameter(name, atof(value.c_str()), unit);
				break;
			case Parameter::STRING:
				*((std::string*)parameters[name].ptr) = value;
				break;
		}
	}
	
	void Model::registerPort(Port& port, const std::string& name, const std::string& unit, const std::string& description)
	{ 
		port.setOwner(this); 
		port.setUnit(unit);
		ports[name] = &port;
		port_descriptions[&port] = description;
	}
	
	void Model::unregisterPort(Port& port)
	{
		for (PortList::iterator it = ports.begin(); it != ports.end(); it++) {
			if (it->second == &port) {
				ports.erase(it);
				return;
			}
		}
	}
	
	void Model::copyPort(const Model& source, const std::string& name, Port& port)
	{
		const Port *srcport = source.getPort(name);
		if (!srcport)
			throw ModelException("Invalid copy port");
		port.setOwner(this);
		port.setUnit(srcport->getUnit());
		ports[name] = &port;
		port_descriptions[&port] = source.getPortDescription(srcport);
	}
	
	void Model::registerParameter(void* ptr, Parameter::Type type, const std::string& name, const std::string& unit, const std::string& description)
	{
		Parameter param;
		param.ptr = ptr;
		param.type = type;
		param.name = name;
		param.unit = unit;
		param.description = description;
		parameters[name] = param;
	}
	
	void Model::copyParameter(const Model& source, const std::string& name, void* ptr)
	{
		Parameter param = source.getParameterSpec(name);
		param.ptr = ptr;
		parameters[name] = param;
	}
	
} // namespace sbx

/** \class sbx::Model
 This class is the base of all models, or \e blocks within a simulation. All models share some
 fundamental features, the most important of which are:
 
 <ul>
 <li>Initialization of internal model states through init()
 <li>Updating states, stepping the simulation through update()
 <li>Data input/output through ports
 <li>Parsing of parameters through parseXML()
 </ul>
 
 When adding a new model, the user should derive from this class and implement the pure virtual
 methods. A model can optionally also impose some requirements on the rest of the system
 (UpdateVisitor and Simulation) by implementing getMinimumUpdateFrequency() and isEndPoint(), or
 by implementing its own accept() and/or traverse() methods.
 
 A minimal valid implementation of a model might look like:
 \code
 class MyModel : public sbx::Model
 {
 public:
 MyModel() : sbx::Model() {}
 virtual ~MyModel() {}
 virtual const char* className() { return "MyModel"; }
 virtual const char* libraryName() { return ""; }
 virtual void init() {
 }
 virtual void update(const double dt) {
 }
 };
 
 \endcode
 \see Port, Parameter, ModelVisitor, Simulation
 
 \class sbx::Model::Parameter
 Models have a standard way of parsing, getting and setting parameters. The data is stored in this
 struct. Parameters can be \e registered through Model::registerParameter() and will then be parsed
 through a subsequent call to Model::parseXML(). Parameters can also be get/set at runtime using
 variations of Model::getParameter() and Model::setParameter().
 
 \todo const-correctness
 \todo documentation of all methods
 */
