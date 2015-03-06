#ifndef SBX_OPERATORMODELS_H
#define SBX_OPERATORMODELS_H

#include "Model.h"
#include "Ports.h"

namespace op {
	
	class SIMBLOX_API Constant : public sbx::Model
	{
	public:
		Constant(const std::string& name = "Constant") : sbx::Model(name)
			{ registerPort(out,"out"); registerParameter(&value, Parameter::DOUBLE, "value"); }
		Constant(const Constant& source) : sbx::Model(source)
			{ copyPort(source, "out", out); copyParameter(source, "value", &value); }
		
		META_Model(op, Constant, "Constant output");
		
		virtual void init();

		void set(const double newvalue) { value = newvalue; out = value; }
		double get() const { return value; }
		
	private:
		double value;
		sbx::OutPort<double> out;
	};

	class SIMBLOX_API Add : public sbx::Model
	{
	public:
		Add(const std::string& name = "Add") : sbx::Model(name)
			{ registerPort(a,"a"); registerPort(b,"b"); registerPort(c,"c"); }
		Add(const Add& source) : sbx::Model(source)
			{ copyPort(source, "a", a); copyPort(source, "b", b); copyPort(source, "c", c); }
		
		META_Model(op, Add, "Addition operator, a + b = c");
		
		virtual void init();
		virtual void update(const double dt);
		
	private:
		sbx::InPort<double> a,b;
		sbx::OutPort<double> c;
	};
	
	class SIMBLOX_API Subtract : public sbx::Model
	{
	public:
		Subtract(const std::string& name = "Subtract") : sbx::Model(name)
		{ registerPort(a,"a"); registerPort(b,"b"); registerPort(c,"c"); }
		Subtract(const Subtract& source) : sbx::Model(source)
		{ copyPort(source, "a", a); copyPort(source, "b", b); copyPort(source, "c", c); }
		
		META_Model(op, Subtract, "Subtraction operator, a - b = c");
		
		virtual void init();
		virtual void update(const double dt);
		
	private:
		sbx::InPort<double> a,b;
		sbx::OutPort<double> c;
	};
	
	class SIMBLOX_API Multiply : public sbx::Model
	{
	public:
		Multiply(const std::string& name = "Multiply") : sbx::Model(name)
		{ registerPort(a,"a"); registerPort(b,"b"); registerPort(c,"c"); }
		Multiply(const Multiply& source) : sbx::Model(source)
		{ copyPort(source, "a", a); copyPort(source, "b", b); copyPort(source, "c", c); }
		
		META_Model(op, Multiply, "Multiplication operator, a * b = c");
		
		virtual void init();
		virtual void update(const double dt);
		
	private:
		sbx::InPort<double> a,b;
		sbx::OutPort<double> c;
	};
	
	class SIMBLOX_API Divide : public sbx::Model
	{
	public:
		Divide(const std::string& name = "Divide") : sbx::Model(name)
		{ registerPort(a,"a"); registerPort(b,"b"); registerPort(c,"c"); }
		Divide(const Divide& source) : sbx::Model(source)
		{ copyPort(source, "a", a); copyPort(source, "b", b); copyPort(source, "c", c); }
		
		META_Model(op, Divide, "Division operator, a / b = c");
		
		virtual void init();
		virtual void update(const double dt);
		
	private:
		sbx::InPort<double> a,b;
		sbx::OutPort<double> c;
	};
	
}

#endif
