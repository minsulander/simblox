#include "Group.h"
#include "Ports.h"
#include "Log.h"
#include "ModelFactory.h"
#include "ModelVisitor.h"
#include "XMLParser.h"

namespace sbx
{
	
	REGISTER_Object(sbx, Group);
	
	Group::Group(const std::string& name) : Model(name) { }
	
	Group::Group(const Group& source) : Model(source)
	{
		for(ChildList::const_iterator itr=source.children.begin();
			itr!=source.children.end();
			++itr)
		{
			Model* child = (Model*) (*itr)->clone();
			if (child) addChild(child);
		}
	}
	
	Group::~Group()
	{
		clear();
	}
	
	void Group::parseXML(const TiXmlElement* element)
	{
		Model::parseXML(element);
		std::string file = XMLParser::parseStringAttribute(element, "file", true, "");
		if (file.length() > 0) {
			Group *group = XMLParser::instance().loadModels(file);
			while (group->getNumChildren() > 0) {
				addChild(group->getChild(0));
				group->removeChild(0);
			}
			delete group;
		}
		for (const TiXmlElement* elem=element->FirstChildElement(); elem; elem=elem->NextSiblingElement()) {
			if (std::string(elem->Value()) == "include") {
				std::string file = XMLParser::parseStringAttribute(elem, "file", true, "");
				if (file.length() > 0) {
					smrt::ref_ptr<Group> group = XMLParser::instance().loadModels(file);
					std::cout << "include " << group->getNumChildren() << " children\n";
					while (group->getNumChildren() > 0)
						addChild(group->getChild(0));
					// children are removed from their previous owner
				}
			} else if (std::string(elem->Value()) == "port") {
				// Exported child port reference
				std::string pname;
				Port* refport;
				if (elem->Attribute("name"))
					pname = elem->Attribute("name");
				else
					throw ParseException(std::string("Missing port name in Group '") + pname + "'", elem);
				if (elem->Attribute("ref")) {
					FindVisitor fv;
					refport = fv.findPort(*this,elem->Attribute("ref"));
					if (!refport)
						throw ParseException(std::string("Unknown port reference for '") + pname + "' in Group '" + getName() + "'", elem);
				} else
					throw ParseException(std::string("Missing port reference for '") + pname + "' in Group '" + getName() + "'", elem);
				exportChildPort(refport, pname);
			} else if (std::string(elem->Value()) == "connect") {
				// Port connection(s)
				if (elem->Attribute("first") && elem->Attribute("second")) {
					std::string s1 = elem->Attribute("first");
					std::string s2 = elem->Attribute("second");
					FindVisitor fv;
					Port* port1 = fv.findPort(*this, s1);
					Port* port2 = fv.findPort(*this, s2);
					if (port1 && port2) {
						dout(2) << " connect " << s1 << " to " << s2 << "\n";
						port1->connect(port2);
					} else if (!port2)
						throw PortException("Unknown port '" + s2 + "'");
					else
						throw PortException("Unknown port '" + s1 + "'");
				} else {
					std::string str = elem->GetText();
					int i;
					while ((i = str.find_first_of(",;:-><")) > 0)
						str.replace(i,1," ");
					std::stringstream ss(str);
					std::string s1, s2;
					while (ss >> s1 >> s2) {
						if (s1.substr(0,1) == "#" || s1.substr(0,2) == "//")
							continue; // comments
						FindVisitor fv;
						Port* port1 = fv.findPort(*this,s1);
						Port* port2 = fv.findPort(*this,s2);
						if (port1 && port2) {
							dout(2) << " connect " << s1 << " to " << s2 << "\n";
							port1->connect(port2);
						} else if (!port2)
							throw PortException("Unknown port '" + s2 + "'");
						else
							throw PortException("Unknown port '" + s1 + "'");
					}
				}
			} else {
				// Child model
				dout(3) << " creating " << elem->Value() << "\n";
				sbx::Model* model = sbx::ModelFactory::instance().create(elem->Value());
				dout(2) << " model " << elem->Value() << "\n";
				if (model) {
					model->parseXML(elem);
					addChild(model);
				} else
					dout(ERROR) << "Unknown model '" << elem->Value() << "'\n";
			}
		}
	}
	
	void Group::writeXML(TiXmlElement *element)
	{
		Model::writeXML(element);
		// Children
		for (ChildList::iterator i = children.begin(); i != children.end(); i++) {
			Model *child = i->get();
			TiXmlElement *childElement = new TiXmlElement(child->libraryName() + std::string("_") + child->className());
			/// \todo figure out when to add libraryName() to element name
			child->writeXML(childElement);
			element->LinkEndChild(childElement);
		}
		// Exported child ports
		for (unsigned int p = 0; p < getNumPorts(); p++) {
			Port *port = getPort(p);
			TiXmlElement *portElement = new TiXmlElement("port");
			portElement->SetAttribute("name", getPortName(p));
			portElement->SetAttribute("ref", port->getOwner()->getPath(this) + "." +
									  port->getOwner()->getPortName(port));
			element->LinkEndChild(portElement);
		}
		// Port connections
		std::map< Port*, std::vector<Port*> > connectionsMade;
		for (ChildList::iterator i = children.begin(); i != children.end(); i++) {
			Model *child = i->get();
			for (unsigned int p = 0; p < child->getNumPorts(); p++) {
				Port *port = child->getPort(p);
				
				for (unsigned int c = 0; c < port->getNumConnections(); c++) {
					std::string port1, port2;
					if (ownsPort(port))
						port1 = getPortName(port);
					else
						port1 = child->getName() + "." + child->getPortName(p);
					Port *otherport = port->getOtherEnd(c);
					
					// Check that this connection hasn't already been specified
					if (connectionsMade.find(port) != connectionsMade.end()) {
						bool done = false;
						for (std::vector<Port*>::iterator i = connectionsMade[port].begin(); i != connectionsMade[port].end(); i++) {
							if (*i == otherport) {
								done = true;
								break;
							}
						}
						if (done)
							continue;
					}
					Model *othermodel = otherport->getOwner();
					// Skip external connection from exported port
					if (ownsPort(port) && othermodel->getParent() == getParent())
						continue;
					if (othermodel->getParent() && othermodel->getParent()->ownsPort(otherport))
						othermodel = othermodel->getParent();
					port2 = othermodel->getPath(this) + "." + othermodel->getPortName(otherport);
					TiXmlElement *connElement = new TiXmlElement("connect");
					connElement->SetAttribute("first",port1);
					connElement->SetAttribute("second",port2);
					element->LinkEndChild(connElement);
					
					connectionsMade[port].push_back(otherport);
					connectionsMade[otherport].push_back(port);
				}
			}
		}
	}
	
	void Group::accept(ModelVisitor& visitor)
	{
		visitor.apply(*this);
	}
	
	void Group::traverse(ModelVisitor& visitor)
	{
		for (ChildList::iterator i = children.begin(); i != children.end(); i++) {
			(*i)->accept(visitor);
		}
	}
	
	/** \note If the model already has a parent group, it will be removed from that group.
	 \throw ModelException if \a child == NULL
	 */
	void Group::addChild(Model* child)
	{
		if (!child)
			throw ModelException("Attempt to add NULL child to group", this);
		for (ChildList::iterator i = children.begin(); i != children.end(); i++) {
			if ((*i)->getName() == child->getName())
				throw ModelException(std::string("Attempt to add child with non-unique name '") + child->getName() + "'", this);
		}
		children.push_back(child);
		if (child->getParent() && child->getParent() != this)
			child->getParent()->removeChild(child);
		child->setParent(this);
	}
	
	void Group::removeChildren(const unsigned int index, const unsigned int num)
	{
		children.erase(children.begin()+index, children.begin()+index+num);
	}
	
	void Group::removeChild(Model* child)
	{
		for (ChildList::iterator i = children.begin(); i != children.end(); i++) {
			if (*i == child) {
				children.erase(i);
				return;
			}
		}
	}
	
	void Group::clear()
	{
		children.clear();
	}
	
	/// \throw ModelException if \a port does not belong to a child model
	void Group::exportChildPort(Port *port, const std::string& name)
	{
		//if (port->getOwner()->getParent() != this)
		//	throw ModelException("Attempt to export port from non-child", this);
		ports[name] = port;
		port_descriptions[port] = port->getOwner()->getPortDescription(port);
		dout(4) << "   exported port " << port->getOwner()->getName() << "."
		<< port->getOwner()->getPortName(port)
		<< " as " << name << "\n";
	}
	
	void Group::unexportChildPort(Port *port)
	{
		for (PortList::iterator it = ports.begin(); it != ports.end(); it++) {
			if (it->second == port) {
				ports.erase(it);
				return;
			}
		}
	}
	
}
