#ifndef SBXBUILDER_DECORATIONS_H
#define SBXBUILDER_DECORATIONS_H

#include <string>
#include <map>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <Eigen/Core>

class TiXmlElement;

namespace sbxBuilder {
	
	struct Decoration {
		Fl_Boxtype box;
		Fl_Image *image;
		Eigen::Vector3d color;
		
		Decoration();
	};
	
	/// A singleton handler for model block decoration
	class Decorations {
	public:
		/// Load decorations from file
		void load(const std::string& filename);
		void parseXML(const TiXmlElement *element);
		/// Set widget properties for a given decoration name
		void apply(const std::string& name, Fl_Widget *widget);
		
		/// Singleton accessor
		static Decorations& instance();
		
	private:
		Decorations();
		
		typedef std::map<std::string, Decoration> DecorationMap;
		DecorationMap decorations;
		
		static Decorations* instanceptr;
	};
	
}

#endif
