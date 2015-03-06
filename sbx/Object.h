#ifndef SBX_OBJECT_H
#define SBX_OBJECT_H

#include "Export.h"
#include "smrt/Referenced.h"
#include <string>

/** META_Object macro define the standard clone, isSameKindAs and className methods.
 * Use when subclassing from Object to make it more convenient to define 
 * the standard pure virtual clone, isSameKindAs and className methods 
 * which are required for all Object subclasses.*/
#define META_Object(library,name) \
virtual sbx::Object* cloneType() const { return new name (); } \
virtual sbx::Object* clone() const { return new name (*this); } \
virtual bool isSameKindAs(const sbx::Object* obj) const { return dynamic_cast<const name *>(obj)!=NULL; } \
virtual const char* libraryName() const { return #library; }\
virtual const char* className() const { return #name; }

namespace sbx {
	
	/// Pure virtual base class for SimBlox objects
	class SIMBLOX_API Object : public smrt::Referenced {
	public:
		Object(const std::string& newname = "Object");
		Object(const Object& other);
		
        /** Clone the type of an object, with Object* return type.
		 Must be defined by derived classes.*/
        virtual Object* cloneType() const = 0;
		
        /** Clone an object, with Object* return type.
		 Must be defined by derived classes.*/
        virtual Object* clone() const = 0;
		
        virtual bool isSameKindAs(const Object*) const { return true; }
		
        /** return the name of the object's library. Must be defined
		 by derived classes. The OpenSceneGraph convention is that the
		 namespace of a library is the same as the library name.*/
        virtual const char* libraryName() const = 0;
		
        /** return the name of the object's class type. Must be defined
		 by derived classes.*/
        virtual const char* className() const = 0;
		inline const std::string& getName() const { return name; }
		virtual void setName(const std::string& newname) { name = newname; }
	private:
		std::string name;
	};
	
}

#endif
