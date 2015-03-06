#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include "Export.h"
#include "Eigen/Core"
#include "TinyXML/tinyxml.h"
#include "FilePath.h"
#include <string>
#include <vector>
#include <stack>

namespace sbx
{
	
	// Forward declarations
	class Group;
	class Simulation;
	
	/// XML parser for the SimBlox configuration and data specification XML files.
	class SIMBLOX_API XMLParser
	{
	public:
		
		void init();
		Group* loadModels(const std::string& filename);
		Simulation* loadSimulation(const std::string& filename);
		
		FilePath& getPath() { return path; }
		const char* getCurrentFile();
		
		static const char* parseStringAttribute(const TiXmlElement* element, const std::string& name, bool usedef = false, const std::string& def = "");
		static const char* parseString(const TiXmlElement* parent, const std::string& name, bool usedef = false, const std::string& def = "", const std::string& attr = "value");
		static const int parseIntAttribute(const TiXmlElement* xe, const std::string& attr, bool usedef = false, const int def = 0);
		static const int parseInt(const TiXmlElement* parent, const std::string& name, const std::string& unit = "", bool usedef = false, const int def = 0, const std::string& attr = "value");
		static const double parseDoubleAttribute(const TiXmlElement* xe, const std::string& attr, bool usedef = false, const double def = 0.0);
		static const double parseDouble(const TiXmlElement* parent, const std::string& name, const std::string& unit = "", bool usedef = false, const double def = 0.0, const std::string& attr = "value");
		static const Eigen::Vector3d parseVector3String(const std::string& str);
		static const Eigen::Vector3d parseVector3Attribute(const TiXmlElement* xe, const std::string& attr, bool usedef = false, const Eigen::Vector3d& def = Eigen::Vector3d::zero());
		static const Eigen::Vector3d parseVector3(const TiXmlElement* parent, const std::string& name, const std::string& unit = "", bool usedef = false, const Eigen::Vector3d& def = Eigen::Vector3d::zero(), const std::string& attr = "value");
		static const Eigen::Matrix3d parseMatrix3(const TiXmlElement* parent, const std::string& name, const std::string& unit = "", bool usedef = false, const Eigen::Matrix3d& def = Eigen::Matrix3d::zero());
		static const bool parseBooleanAttribute(const TiXmlElement* element, const std::string& attr = "value", const bool usedef = false, const bool def = false);
		static const bool parseBoolean(const TiXmlElement* parent, const std::string& name, const bool usedef = false, const bool def = false, const std::string& attr = "value");
		static void parseTable(const TiXmlElement* parent, const std::string& name, std::vector<double>& x, std::vector<double>& y, const std::string& unit = "", const std::string& unit2 = "");
		static bool parseMatrix(Eigen::MatrixXd& M, const TiXmlElement* parent, const std::string& name, const std::string& unit = "");
		static bool parseVector(Eigen::VectorXd& V, const TiXmlElement* parent, const std::string& name, const std::string& unit = "");
		
		// Alternative versions
		static const int parseInt(const TiXmlElement* parent, const std::string& name, const std::string& unit, const int def) { return parseInt(parent,name,unit,true,def); }
		static const double parseDouble(const TiXmlElement* parent, const std::string& name, const std::string& unit, const double def) { return parseDouble(parent,name,unit,true,def); }
		static const Eigen::Vector3d parseVector3(const TiXmlElement* parent, const std::string& name, const std::string& unit, const Eigen::Vector3d& def) { return parseVector3(parent,name,unit,true,def); }
		static const int parseInt(const TiXmlElement* parent, const std::string& name, const int def) { return parseInt(parent,name,"",true,def); }
		static const double parseDouble(const TiXmlElement* parent, const std::string& name, const double def) { return parseDouble(parent,name,"",true,def); }
		static const Eigen::Vector3d parseVector3(const TiXmlElement* parent, const std::string& name, const Eigen::Vector3d& def) { return parseVector3(parent,name,"",true,def); }
		static const Eigen::Matrix3d parseMatrix3(const TiXmlElement* parent, const std::string& name, const Eigen::Matrix3d& def) { return parseMatrix3(parent,name,"",true,def); }
		
		static void setString(TiXmlElement* parent, const std::string& element, const std::string& value, const std::string& attr = "value");
		static void setInt(TiXmlElement* parent, const std::string& element, const int value, const std::string& attr = "value");
		static void setDouble(TiXmlElement* parent, const std::string& element, const double value, const std::string& attr = "value");
		static void setBoolean(TiXmlElement* parent, const std::string& element, const bool value, const std::string& attr = "value");
		static void setVector(TiXmlElement* parent, const std::string& element, const Eigen::VectorXd& vec);
		
		static XMLParser& instance();
	protected:
		XMLParser(); ///< pure singleton constructor
		static XMLParser *instanceptr;
		
		bool initialized;
		FilePath path;
		std::stack<std::string> filestack;
	};
	
	class ParseException : public std::exception {
	public:
		ParseException(const std::string& message = "", const TiXmlNode *node = NULL)
		{ 
			std::stringstream ss;
			ss << XMLParser::instance().getCurrentFile();
			if (node) {
				if (node->ToDocument()) 
					ss << ":" << node->ToDocument()->ErrorRow();
				else if (node->Row())
					ss << ":" << node->Row();
			}
			ss << ": ";
			ss << message;
			this->message = ss.str();
			this->node = node; 
		}
		~ParseException() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
		std::string message;
		const TiXmlNode *node;
	};
	
	class ParseNoElementException : public ParseException
		{
		public:
			ParseNoElementException(const std::string& message = "", const TiXmlNode *node = NULL)
			: ParseException(message, node)
			{ }
		};
	
}

#endif /*XMLPARSER_H_*/
