#include "XMLParser.h"
#include "units/units.h"
#include "ModelFactory.h"
#include "Group.h"
#include "Simulation.h"
#include "Log.h"

#include <sstream>

namespace sbx
{
	
	void XMLParser::init()
	{
		std::string unitsfile = path.find("units.dat");
		if (unitsfile.length() > 0)
			units::initialize(unitsfile);
		initialized = true;
	}
	
	Group* XMLParser::loadModels(const std::string& filename)
	{
		if (!initialized)
			init();
		std::string fname = path.find(filename);
		if (fname.length() == 0)
			throw ParseException("File not found: " + filename);
		filestack.push(fname);
		TiXmlDocument doc(fname);
		bool ok = doc.LoadFile();
		if (!ok)
			throw ParseException(doc.ErrorDesc(), &doc);
		dout(1) << "Parsing models from " << filename << "\n";
		TiXmlHandle hDoc(&doc);
		const TiXmlElement* pElem;
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem->Value() || std::string(pElem->Value()) != "SimBlox")
			throw ParseNoElementException("No top-level SimBlox element", &doc);
		pElem = pElem->FirstChildElement("Models");
		if (!pElem)
			throw ParseNoElementException("No 'Models' element", &doc);
		Group *group = new Group;
		group->parseXML(pElem);
		filestack.pop();
		return group;
	}
	
	Simulation* XMLParser::loadSimulation(const std::string& filename)
	{
		if (!initialized)
			init();
		std::string fname = path.find(filename);
		if (fname.length() == 0)
			throw ParseException("File not found: " + filename);
		filestack.push(fname);
		TiXmlDocument doc(fname);
		bool ok = doc.LoadFile();
		if (!ok)
			throw ParseException(doc.ErrorDesc(), &doc);
		dout(1) << "Parsing simulation from " << filename << "\n";
		TiXmlHandle hDoc(&doc);
		const TiXmlElement* pElem;
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem->Value() || std::string(pElem->Value()) != "SimBlox")
			throw ParseException("No top-level SimBlox element", &doc);
		pElem = pElem->FirstChildElement("Simulation");
		if (!pElem)
			throw ParseNoElementException("No 'Simulation' element", &doc);
		if (pElem->NextSiblingElement("Simulation"))
			throw ParseException("Multiple 'Simulation' elements", &doc);
		Simulation *sim = new Simulation;
		sim->parseXML(pElem);
		filestack.pop();
		return sim;
	}
	
	const char* XMLParser::getCurrentFile()
	{
		if (filestack.empty())
			return "";
		return filestack.top().c_str();
	}
	
	const char* XMLParser::parseStringAttribute(const TiXmlElement* xe, const std::string& name, bool usedef, const std::string& def)
	{
		if (xe->Attribute(name.c_str()))
			return xe->Attribute(name.c_str());
		else if (usedef)
			return def.c_str();
		else
			throw ParseNoElementException("Attribute '" + name + "' missing in element '" + xe->Value() + "'", xe);
	}
	
	const char* XMLParser::parseString(const TiXmlElement* parent, const std::string& name, bool usedef, const std::string& def, const std::string& attr)
	{
		const TiXmlElement* xe = parent->FirstChildElement(name.c_str());
		if (!xe) {
			if (usedef)
				return def.c_str();
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		if (attr == "text")
			return xe->GetText();
		else if (xe->Attribute(attr.c_str()))
			return xe->Attribute(attr.c_str());
		else if (usedef)
			return def.c_str();
		else
			throw ParseNoElementException("Element '" + name + "' does not have attribute '" + attr + "'", xe);
	}
	
	const int XMLParser::parseIntAttribute(const TiXmlElement* xe, const std::string& attr, bool usedef, const int def)
	{
		int val;
		int ret = xe->QueryIntAttribute(attr.c_str(),&val);
		if (ret == TIXML_WRONG_TYPE)
			throw ParseException("wrong type of value for attribute '" + attr + "' in '" + xe->Value() + "'", xe);
		else if (ret != TIXML_SUCCESS) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("no value for attribute '" + attr + "' in '" + xe->Value() + "'", xe);
		}
		return val;		
	}
	
	const int XMLParser::parseInt(const TiXmlElement* parent, const std::string& name, const std::string& unit, bool usedef, const int def, const std::string& attr) 
	{
		const TiXmlElement* xe = parent->FirstChildElement(name.c_str());
		if (!xe) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		int val;
		if (xe->Attribute(attr.c_str()))
			val = parseIntAttribute(xe,attr,usedef,def);
		else if ((attr == "text" || attr == "value") && xe->GetText())
			val = atoi(xe->GetText());
		else
			throw ParseException("missing '" + attr + "' attribute in element '" + name + "'");
		// Unit conversion
		if (xe->Attribute("unit")) {
			if (unit == "")
				throw ParseException("Element " + name + " does not have a unit (" + unit + " specified)", xe);
			val = (int)units::convert(val,xe->Attribute("unit"),unit);
		}
		return val;		
	}
	
	const double XMLParser::parseDoubleAttribute(const TiXmlElement* xe, const std::string& attr, bool usedef, const double def)
	{
		double val;
		int ret = xe->QueryDoubleAttribute(attr.c_str(),&val);
		if (ret == TIXML_WRONG_TYPE)
			throw ParseException("wrong type of value for attribute '" + attr + "' in '" + xe->Value() + "'", xe);
		else if (ret != TIXML_SUCCESS) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("no value for attribute '" + attr + "' in '" + xe->Value() + "'", xe);
		}
		return val;		
	}
	
	const double XMLParser::parseDouble(const TiXmlElement* parent, const std::string& name, const std::string& unit, bool usedef, const double def, const std::string& attr) 
	{
		const TiXmlElement* xe = parent->FirstChildElement(name.c_str());
		if (!xe) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		double val;
		if (xe->Attribute(attr.c_str()))
			val = parseDoubleAttribute(xe,attr,usedef,def);
		else if ((attr == "text" || attr == "value") && xe->GetText())
			val = atof(xe->GetText());
		else
			throw ParseException("missing '" + attr + "' attribute in element '" + name + "'");
		// Unit conversion
		if (xe->Attribute("unit")) {
			if (unit == "")
				throw ParseException("Element " + name + " does not have a unit (" + unit + " specified)", xe);
			val = units::convert(val,xe->Attribute("unit"),unit);
		}
		return val;		
	}
	
	const Eigen::Vector3d XMLParser::parseVector3String(const std::string& str) 
	{
		std::string str2 = str;
		Eigen::Vector3d v;
		int i;
		while ((i = str2.find_first_of(",;:")) > 0)
			str2.replace(i,1," ");
		std::stringstream ss(str2);
		for (int i = 0; i < 3; i++)
			if (!(ss >> v[i]))
				throw ParseException("missing vector component");
		return v;
	}
	
	
	const Eigen::Vector3d XMLParser::parseVector3Attribute(const TiXmlElement* xe, const std::string& attr, bool usedef, const Eigen::Vector3d& def)
	{
		Eigen::Vector3d v;
		if (xe->Attribute(attr.c_str()))
			v = parseVector3String(xe->Attribute(attr.c_str()));
		else if (usedef)
			v = def;
		else
			throw ParseNoElementException("Element " + std::string(xe->Value()) + " does not have a value");
		return v;
	}
	
	const Eigen::Vector3d XMLParser::parseVector3(const TiXmlElement* parent, const std::string& name, const std::string& unit, bool usedef, const Eigen::Vector3d& def, const std::string& attr) 
	{
		const TiXmlElement* xe = parent->FirstChildElement(name);
		if (!xe) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		Eigen::Vector3d v;
		if (xe->Attribute(attr.c_str()))
			v = parseVector3Attribute(xe,attr,usedef,def);
		else if ((attr == "text" || attr == "value") && xe->GetText())
			v = parseVector3String(xe->GetText());
		else
			throw ParseException("missing '" + attr + "' attribute in element '" + name + "'");
		// Unit conversion
		if (xe->Attribute("unit")) {
			if (unit == "")
				throw ParseException("Element " + name + " does not have a unit (" + xe->Attribute("unit") + " specified)", xe);
			v.x() = units::convert(v.x(),xe->Attribute("unit"),unit);
			v.y() = units::convert(v.y(),xe->Attribute("unit"),unit);
			v.z() = units::convert(v.z(),xe->Attribute("unit"),unit);
		}
		return v;
	}
	
	const Eigen::Matrix3d XMLParser::parseMatrix3(const TiXmlElement* parent, const std::string& name, const std::string& unit, bool usedef, const Eigen::Matrix3d& def) 
	{
		Eigen::Matrix3d M;
		const TiXmlElement* xe = parent->FirstChildElement(name);
		if (!xe) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		if (!xe->GetText())
			throw ParseException("missing text in element '" + name +"'");
		std::string str = xe->GetText();
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");	
		std::stringstream sin(str);
		for (int r = 0; r < 3; r++) {
			for (int c = 0; c < 3; c++) {
				if (!(sin >> M(r,c)))
					throw ParseException("missing matrix component");
				// Unit conversion
				if (xe->Attribute("unit")) {
					if (unit == "")
						throw ParseException("Element " + name + " does not have a unit (" + xe->Attribute("unit") + " specified)", xe);
					M(r,c) = units::convert(M(r,c),xe->Attribute("unit"),unit);
				}
			}
		}
		return M;
	}
	
	const bool parseBooleanString(const std::string& str)
	{
		return str == "true" || str == "True" || str == "TRUE" || str == "1" || str=="on" || str == "ON";
	}
	
	const bool XMLParser::parseBooleanAttribute(const TiXmlElement* xe, const std::string& attr, const bool usedef, const bool def)
	{
		std::string str;
		if (xe->Attribute(attr.c_str()))
			str = xe->Attribute(attr.c_str());
		else if (usedef)
			return def;
		else
			throw ParseNoElementException("Element '" + std::string(xe->Value()) + "' does not have attribute '" + attr + "'", xe);
		return parseBooleanString(str);
	}
	
	const bool XMLParser::parseBoolean(const TiXmlElement* parent, const std::string& name, const bool usedef, const bool def, const std::string& attr)
	{
		const TiXmlElement* xe = parent->FirstChildElement(name.c_str());
		if (!xe) {
			if (usedef)
				return def;
			else
				throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		if (xe->Attribute(attr.c_str()))
			return parseBooleanAttribute(xe,attr,usedef,def);
		else if ((attr == "text" || attr == "value") && xe->GetText())
			return parseBooleanString(xe->GetText());
		else
			throw ParseException("missing '" + attr + "' attribute in element '" + name + "'");
	}
	
	void XMLParser::parseTable(const TiXmlElement* parent, const std::string& name, std::vector<double>& x, std::vector<double>& y, const std::string& unit, const std::string& unit2)
	{
		const TiXmlElement* xe = parent->FirstChildElement(name);
		if (!xe) {
			throw ParseNoElementException("missing element '" + name + "' in '" + parent->Value() + "'", parent);
		}
		if (!xe->GetText())
			throw ParseException("missing text in element '" + name +"'");
		std::string str = xe->GetText();
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");	
		std::stringstream sin(str);
		double xi, yi;
		while (sin >> xi) {
			if (sin >> yi) {
				// Unit conversion
				if (xe->Attribute("unit")) {
					if (unit == "")
						throw ParseException("Element " + name + " does not have a unit (" + xe->Attribute("unit") + " specified)", xe);
					x.push_back(units::convert(xi,xe->Attribute("unit"),unit));
				} else
					x.push_back(xi);
				// Unit conversion for second value
				if (xe->Attribute("unit2")) {
					if (unit2 == "")
						throw ParseException("Element " + name + " does not have a second unit (" + unit + " specified)", xe);
					y.push_back(units::convert(yi,xe->Attribute("unit2"),unit2));
				} else
					y.push_back(yi);
			} else
				throw ParseException("nonrectangular table '" + name + "' in '" + parent->Value() + "'", xe);
		}
	}
	
	bool XMLParser::parseMatrix(Eigen::MatrixXd& M, const TiXmlElement* parent, const std::string& name, const std::string& unit)
	{
		const TiXmlElement* xe = parent->FirstChildElement(name);
		if (!xe)
			return false;
		if (!xe->GetText())
			throw ParseException("missing text in element '" + name +"'");
		std::string str = xe->GetText();
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");	
		std::stringstream sin(str);
		for (int r = 0; r < M.rows(); r++) {
			for (int c = 0; c < M.cols(); c++) {
				if (!(sin >> M(r,c))) {
					std::stringstream ss;
					ss << "invalid matrix size for element '" << name << "' in '" << parent->Value() << "' (should be " << M.rows() << "x" << M.cols() << ")";
					throw ParseException(ss.str());
				}
				// Unit conversion
				if (xe->Attribute("unit")) {
					if (unit == "")
						throw ParseException("Element " + name + " does not have a unit (" + xe->Attribute("unit") + " specified)", xe);
					M(r,c) = units::convert(M(r,c),xe->Attribute("unit"),unit);
				}
			}
		}
		double d;
		if (sin >> d) {
			std::stringstream ss;
			ss << "invalid matrix size for element '" << name << "' in '" << parent->Value() << "' (should be " << M.rows() << "x" << M.cols() << ")";
			throw ParseException(ss.str());
		}
		return true;
	}
	
	bool XMLParser::parseVector(Eigen::VectorXd& V, const TiXmlElement* parent, const std::string& name, const std::string& unit)
	{
		const TiXmlElement* xe = parent->FirstChildElement(name);
		if (!xe)
			return false;
		if (!xe->GetText())
			throw ParseException("missing text in element '" + name +"'");
		std::string str = xe->GetText();
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");	
		std::stringstream sin(str);
		for (i = 0; i < V.size(); i++) {
			if (!(sin >> V[i])) {
				std::stringstream ss;
				ss << "invalid vector length for element '" << name << "' in '" << parent->Value() << " (should be " << V.size() << ")";
				throw ParseException(ss.str());
			}
			// Unit conversion
			if (xe->Attribute("unit")) {
				if (unit == "")
					throw ParseException("Element " + name + " does not have a unit (" + xe->Attribute("unit") + " specified)", xe);
				V[i] = units::convert(V[i],xe->Attribute("unit"),unit);
			}
		}
		double d;
		if (sin >> d) {
			std::stringstream ss;
			ss << "invalid vector length for element '" << name << "' in '" << parent->Value() << " (should be " << V.size() << ")";
			throw ParseException(ss.str());
		}
		return true;
	}
	
	void XMLParser::setString(TiXmlElement* parent, const std::string& element, const std::string& value, const std::string& attr)
	{
		TiXmlElement *el = parent->FirstChildElement(element.c_str());
		if (!el) {
			el = new TiXmlElement(element.c_str());
			parent->LinkEndChild(el);
		}
		el->SetAttribute(attr.c_str(), value.c_str());
	}
	
	void XMLParser::setInt(TiXmlElement* parent, const std::string& element, const int value, const std::string& attr)
	{
		std::stringstream s;
		s << value;
		setString(parent,element,s.str(),attr);
	}
	
	void XMLParser::setDouble(TiXmlElement* parent, const std::string& element, const double value, const std::string& attr)
	{
		std::stringstream s;
		s << value;
		setString(parent,element,s.str(),attr);
	}
	
	void XMLParser::setBoolean(TiXmlElement* parent, const std::string& element, const bool value, const std::string& attr)
	{
		setString(parent,element,value ? "true" : "false",attr);
	}
	
	void XMLParser::setVector(TiXmlElement* parent, const std::string& element, const Eigen::VectorXd& vec)
	{
		std::stringstream ss;
		for (size_t i = 0; i < vec.size(); i++) {
			ss << vec[i];
			if (i < vec.size()-1)
				ss << ", ";
		}
		TiXmlElement *el = parent->FirstChildElement(element.c_str());
		if (!el) {
			el = new TiXmlElement(element.c_str());
			parent->LinkEndChild(el);
		}
		TiXmlText *text = new TiXmlText(ss.str());
		el->LinkEndChild(text);
	}
	
	XMLParser::XMLParser()
	:	initialized(false)
	{
		path.addEnvironmentVariable("SIMBLOX_DATA_PATH");
		path.addEnvironmentVariable("SIMBLOX_HOME", "/data");
	}
	
	XMLParser& XMLParser::instance()
	{
		if (!instanceptr)
			instanceptr = new XMLParser;
		return *instanceptr;
	}
	
	XMLParser *XMLParser::instanceptr = NULL;
	
} // namespace sbx
