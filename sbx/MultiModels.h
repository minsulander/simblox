#ifndef MULTIMODELS_H_
#define MULTIMODELS_H_

#include "Export.h"
#include "Model.h"
#include "Ports.h"

namespace sbx {
	
	template <typename T>
	class MIModel : public Model
	{
	public:
		MIModel<T>(const std::string& name = "MIModel") : Model(name) {}
		MIModel<T>(const MIModel<T>& source) : Model(source)
		{
			for (size_t i = 0; i < source.inputs.size(); i++) {
				Port *port = addInput();
				copyPort(source, source.getPortName(i), *port);
			}
		}
		typedef std::vector< InPort<T>* > InPortList;
		virtual Port* addInput()
		{ 
			InPort<T>* p = new InPort<T>;
			std::stringstream ss;
			ss << "in" << (inputs.size()+1);
			registerPort(*p,ss.str(),"",ss.str());
			inputs.push_back(p);
			return p;
		}
		virtual void removeInput()
		{
			for (size_t i = inputs.size()-1; i > 0; i--) {
				if (!inputs[i]->isConnected()) {
					unregisterPort(*inputs[i]);
					inputs.erase(inputs.begin()+i);
					return;
				}
			}
		}
		virtual bool supportsDynamicInputs() { return true; }
	protected:
		virtual ~MIModel() 
		{
			for (int i = 0; i < inputs.size(); i++)
				delete inputs[i];
		}
		InPortList inputs;
	};
	
	template <typename T>
	class MOModel : public Model
	{
	public:
		MOModel<T>(const std::string& name = "MOModel") : Model(name) {}
		MOModel<T>(const MOModel<T>& source) : Model(source)
		{
			for (size_t i = 0; i < source.outputs.size(); i++) {
				Port *port = addOutput();
				copyPort(source, source.getPortName(i), *port);
			}
		}
		typedef std::vector< OutPort<T>* > OutPortList;
		virtual Port* addOutput()
		{ 
			OutPort<T>* p = new OutPort<T>;
			std::stringstream ss;
			ss << "out" << (outputs.size()+1);
			registerPort(*p,ss.str(),"",ss.str());
			outputs.push_back(p);
			return p;
		}
		virtual void removeOutput()
		{
			for (size_t i = outputs.size()-1; i > 0; i--) {
				if (!outputs[i]->isConnected()) {
					unregisterPort(*outputs[i]);
					outputs.erase(outputs.begin()+i);
					return;
				}
			}
		}
		virtual bool supportsDynamicOutputs() { return true; }
	protected:
		virtual ~MOModel() 
		{
			for (int i = 0; i < outputs.size(); i++)
				delete outputs[i];
		}
		OutPortList outputs;
	};
	
	template <typename T>
	class MIMOModel : public Model
	{
	public:
		MIMOModel<T>(const std::string& name = "MIMOModel") : Model(name) {}
		MIMOModel<T>(const MIMOModel<T>& source) : Model(source)
		{
			for (size_t i = 0; i < source.inputs.size(); i++) {
				Port *port = addInput();
				copyPort(source, source.getPortName(source.inputs[i]), *port);
			}
			for (size_t i = 0; i < source.outputs.size(); i++) {
				Port *port = addOutput();
				copyPort(source, source.getPortName(source.outputs[i]), *port);
			}
		}
		typedef std::vector< InPort<T>* > InPortList;
		typedef std::vector< OutPort<T>* > OutPortList;
		
		virtual Port* addInput()
		{ 
			InPort<T>* p = new InPort<T>;
			std::stringstream ss;
			ss << "in" << (inputs.size()+1);
			registerPort(*p,ss.str(),"",ss.str());
			inputs.push_back(p);
			return p;
		}
		virtual Port* addOutput()
		{ 
			OutPort<T>* p = new OutPort<T>;
			std::stringstream ss;
			ss << "out" << (outputs.size()+1);
			registerPort(*p,ss.str(),"",ss.str());
			outputs.push_back(p);
			return p;
		}
		virtual void removeInput()
		{
			for (size_t i = inputs.size()-1; i > 0; i--) {
				if (!inputs[i]->isConnected()) {
					unregisterPort(*inputs[i]);
					inputs.erase(inputs.begin()+i);
					return;
				}
			}
		}
		virtual void removeOutput()
		{
			for (size_t i = outputs.size()-1; i > 0; i--) {
				if (!outputs[i]->isConnected()) {
					unregisterPort(*outputs[i]);
					outputs.erase(outputs.begin()+i);
					return;
				}
			}
		}
		virtual bool supportsDynamicInputs() { return true; }
		virtual bool supportsDynamicOutputs() { return true; }
	protected:
		virtual ~MIMOModel() 
		{
			for (int i = 0; i < inputs.size(); i++)
				delete inputs[i];
			for (int i = 0; i < outputs.size(); i++)
				delete outputs[i];
		}
		InPortList inputs;
		OutPortList outputs;
	};
	
}

#endif /*MULTIMODELS_H_*/
