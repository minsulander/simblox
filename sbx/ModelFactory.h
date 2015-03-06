#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include "Factory.h"
#include "Model.h"
#include <string>
#include <map>
#include <exception>

namespace sbx
{
	
	class SIMBLOX_API ModelFactory : public Factory<std::string, Model>
	{
	public:
		Model* create(const std::string& type);
		static ModelFactory& instance();
	protected:
		ModelFactory() {}; // pure singleton class
		static ModelFactory *instanceptr;
	};
	
	template <class ObjectT>
	class ObjectAdder {
	public:
		ObjectAdder(const std::string& identifier) { 
			sbx::ModelFactory::instance().registerObject(identifier, new Creator<ObjectT, sbx::Model>);
		}
	};
	
#define REGISTER_Object(library, name) \
sbx::ObjectAdder<name> adder##library##name(#library "::" #name)
#define REGISTER_Object_Alias(library, name, identifier) \
sbx::ObjectAdder<name> adder##identifier(#identifier)
	
}

#endif
