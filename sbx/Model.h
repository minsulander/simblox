#ifndef MODEL_H
#define MODEL_H

#include "Object.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <smrt/observer_ptr.h>

#define META_Model(library, name, desc) \
META_Object(library, name); \
virtual const char* description() const { return desc; }

class TiXmlElement;

namespace sbx
{
	
	class Model;
	class ModelVisitor;
	class Group;
	class Port;
	
	enum DisplayMode { DISPLAY_INITIAL, DISPLAY_CONTINUOUS, DISPLAY_FINAL, DISPLAY_USER };
	
	typedef std::vector< smrt::observer_ptr<Model> > ModelOrderList;

	/// Base class for all models in a simulation.
	class SIMBLOX_API Model : public Object
	{
	public:
		
		struct Parameter {
			enum Type { BOOLEAN, UINTEGER, INTEGER, FLOAT, DOUBLE, STRING };
			Type type;
			std::string name, unit, description;
			void *ptr;
			bool parsed;
		};
		
		Model(const std::string& name = "Model");
		Model(const Model& source);
		
		/// Get a short description of this model
		virtual const char* description() const = 0;
		/// Initialize model prior to simulation start
		virtual void init() {}
		/// Update this model with one simulation step
		virtual void update(const double dt) {}
		/// Display results of the simulation, if applicable
		virtual void display(const DisplayMode mode = DISPLAY_USER);
		/// Show the window used by display() for presentation, if applicable
		virtual void show() {}
		/// Hide the window used by display() for presentation, if applicable
		virtual void hide() {}
		
		/// Parse XML document element
		virtual void parseXML(const TiXmlElement* element);
		/// Write to an XML document element
		virtual void writeXML(TiXmlElement* element);
		/// Called after parsing
		virtual void configure() {}
		/// Get minimum model update frequency
		virtual const int getMinimumUpdateFrequency() { return 0; }
		
		/// Add a new input port, if supported (default behavior: return NULL)
		virtual Port* addInput();
		/// Add a new output port, if supported (default behavior: return NULL)
		virtual Port* addOutput();
		/// Remove the first available unconnected input port, if supported
		virtual void removeInput();
		/// Remove the first available unconnected output port, if supported
		virtual void removeOutput();
		/// Returns true if this model supports adding/removing inputs
		virtual bool supportsDynamicInputs();
		/// Returns true if this model supports adding/removing outputs
		virtual bool supportsDynamicOutputs();
		
		/// Get a list of other models that need to updated before this one.
		/// This essentially returns a list of models connected to this one through its input ports.
		ModelOrderList& getDataProviders();
		/// Get a list of other models that need to be updated after this one.
		/// Essentially returns a list of models connected to this one through its output ports.
		ModelOrderList& getDataDependants();
		/// Returns true if this model needs data from the other
		const bool dependsOn(Model* other);
		/// Returns true if this model depends on the other, but the connecting inputs ports are defined 
		/// as "loose", i.e. they can use data from the previous timestep
		const bool dependsLooselyOn(Model* other);
		/// Returns true if this model depends on the other, and the connecting input ports are not "loose"
		const bool dependsStronglyOn(Model* other);
		/// Returns true if this model provides data for the other
		const bool providesFor(Model* other);
		/// Returns true if this model depends on any other models (i.e. has any input ports connected)
		const bool hasDataProviders();
		/// Returns true if this model provides for any other models (i.e. has any output ports connected)
		const bool hasDataDependants();
		/// Returns true if this model provides for other models, and somewhere down that line is a data endpoint
		const bool hasEndPointDependants();
		
		/// An endpoint model is considered to always have data dependants (e.g. it displays something to the user),
		/// so it doesn't get skipped in the update traversal even though it has no output ports
		virtual const bool isEndPoint() { return false; }
		
		/// Accept a model visitor, can be overloaded to modify visitor pattern behavior
		virtual void accept(ModelVisitor& visitor);
		/// Traverse the model tree
		virtual void traverse(ModelVisitor& visitor);
		
		/// "Cast" this model to a group, for convenience
		virtual Group* asGroup() { return NULL; }
		
		// Getters and setters
		
		virtual void setName(const std::string& newname);
		
		void setError(const std::string& error = "");
		void setWarning(const std::string& warning = "");
		bool getWarnFlag() const { return warnflag; }
		bool getErrFlag() const { return errflag; }
		bool isOK() const { return (!warnflag && !errflag); } ///< Returns true if errFlag and warnFlag are both false
		const std::string& getWarnStr() const { return warnstr; }
		const std::string& getErrStr() const { return errstr; }
		
		const int getUpdateFrequency() { return update_frequency; }
		void setUpdateFrequency(const int freq);
		
		Port* getPort(const std::string& name);
		const Port* getPort(const std::string& name) const;
		Port* getPort(const unsigned int index);
		unsigned int getPortIndex(const Port *port);
		/// Get the total number of ports this model has
		unsigned int getNumPorts() const { return ports.size(); }
		/// Get the number of ports that are inputs
		unsigned int getNumInputs() const;
		/// Get the number of ports that are outputs
		unsigned int getNumOutputs() const;
		/// Get the name of a port
		std::string getPortName(const unsigned int index) const;
		/// Get the name of a port
		std::string getPortName(const Port *port) const;
		/// Get a description of the intented use for a port	
		std::string getPortDescription(const Port* port) const;
		inline std::string getPortDescription(const std::string& name) const { return getPortDescription(getPort(name)); }
		bool ownsPort(const Port *port) const;
		virtual void onPortConnect(Port *port, Port *otherend);
		virtual void onPortDisconnect(Port *port, Port *otherend);
		
		Group* getParent() { return parent; }
		const Group* getParent() const { return parent; }
		void setParent(Group* parent);
		/// Get root (the parent of all parents) to this model
		Group* getRoot();
		/// Get a path name which can be used to refer to this model in a hierarchy
		std::string getPath(const Group* relative = NULL) const;
		
		// Parameter getters and setters
		unsigned int getNumParameters() const { return parameters.size(); }
		std::string getParameterName(const unsigned int index) const;
		const Parameter& getParameterSpec(const std::string& name) const;
		const Parameter& getParameterSpec(const unsigned int index) const;
		void getParameter(const std::string& name, bool& b);
		void getParameter(const std::string& name, int& i, const std::string& unit = "");
		void getParameter(const std::string& name, unsigned int& i, const std::string& unit = "");
		void getParameter(const std::string& name, float& f, const std::string& unit = "");
		void getParameter(const std::string& name, double& d, const std::string& unit = "");
		void getParameter(const std::string& name, std::string& s);
		std::string getParameter(const std::string& name);
		void setParameter(const std::string& name, const double value, const std::string& unit = "");
		void setParameter(const std::string& name, const std::string& value, const std::string& unit = "");
		
	protected:
		virtual ~Model(); // protected destructor - using smart pointer
		
		void registerPort(Port& port, const std::string& name, const std::string& unit = "", const std::string& description = "");
		void unregisterPort(Port& port);
		void copyPort(const Model& source, const std::string& name, Port& port);
		void registerParameter(void* ptr, Parameter::Type type, const std::string& name, const std::string& unit = "", const std::string& description = "");
		void copyParameter(const Model& source, const std::string& name, void *ptr);
	private:
		bool warnflag, errflag;
		std::string warnstr, errstr;
		int update_frequency;
		typedef std::map<std::string, Port*> PortList;
		PortList ports;
		typedef std::map<const Port*, std::string> PortDescriptionList;
		PortDescriptionList port_descriptions;
		typedef std::map<std::string, Parameter> ParameterList;
		ParameterList parameters;
		Group* parent;
		ModelOrderList traversalDependants, traversalProviders;
		
		friend class Group;
	};
	
	class SIMBLOX_API ModelException : public std::exception {
	public:
		ModelException(const std::string& message = "", const Model* model = NULL) { 
			std::stringstream ss;
			ss << "ModelException";
			if (model)
				ss << " in model '" << model->getName() << "'";
			ss << ": " << message;
			this->message = ss.str();
			this->model = model;
		}
		~ModelException() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
		std::string message;
		const Model* model;
	};
	
	
}

#endif
