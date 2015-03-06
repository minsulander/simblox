#ifndef GROUP_H_
#define GROUP_H_

#include "Model.h"
#include "smrt/ref_ptr.h"
#include <vector>
#include <map>

namespace sbx
{

class SIMBLOX_API Group : public sbx::Model
{
public:
	Group(const std::string& name = "Group");
	Group(const Group& source);
	META_Object(sbx,Group);

	virtual const char* className() { return "Group"; }
	virtual const char* libraryName() { return "sbx"; }
	virtual const char* description() const { return "A 'model' containing other models"; }
	virtual void parseXML(const TiXmlElement* element);
	virtual void writeXML(TiXmlElement* element);
	virtual void init() {}
	virtual void update(const double dt) {}
	virtual void accept(ModelVisitor& visitor);
	virtual void traverse(ModelVisitor& visitor);
	virtual Group* asGroup() { return this; }
	
	/// Add a child model to this group (<b>note:</b> removes it from its previous owner, if applicable)
	void addChild(Model *child);
	/// Get the number of children this group has
	unsigned int getNumChildren() { return children.size(); }
	/// Get child by index
	Model* getChild(const unsigned int index) { return children[index].get(); }
	/// Remove a child by index
	inline void removeChild(const int index) { removeChildren(index, 1); }
	/// Remove a sequence of children
	void removeChildren(const unsigned int index, const unsigned int num);
	/// Remove a child by pointer
	void removeChild(Model *child);
	/// Remove all children
	void clear();
	/// Add a child's port to this group's port map
	void exportChildPort(Port *port, const std::string& name);
	/// Remove a child's port from this group's port map
	void unexportChildPort(Port *port);
	
protected:

	virtual ~Group();

	typedef std::vector< smrt::ref_ptr<sbx::Model> > ChildList;
	ChildList children;
};

}

#endif /*GROUP_H_*/
