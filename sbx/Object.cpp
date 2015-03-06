#include "Object.h"

/** @class Object
 This base object class is inspired by OpenSceneGraph. Thanks OSG team for a great framework!
 */

namespace sbx {
	
	Object::Object(const std::string& newname) : name(newname) {}
	Object::Object(const Object& source) : name(source.name) {}
	
}
