#include "Decorations.h"
#include <sbx/XMLParser.h>
#include <sbx/Log.h>
#include <FL/Fl_Shared_Image.H>

namespace sbxBuilder {
	
	Decoration::Decoration()
	: box(FL_THIN_UP_BOX), image(NULL) { color.setZero(); }
	
	void Decorations::load(const std::string& filename)
	{
		sbx::dout(1) << "Loading decorations from " << filename << "\n";
		TiXmlDocument doc(filename);
		bool ok = doc.LoadFile();
		if (!ok)
			throw sbx::ParseException(doc.ErrorDesc(), &doc);
		TiXmlHandle hDoc(&doc);
		TiXmlElement* pElem;
		pElem=hDoc.FirstChildElement().Element();
		if (!pElem->Value() || std::string(pElem->Value()) != "SimBlox")
			throw sbx::ParseNoElementException("No top-level SimBlox element", &doc);
		pElem = pElem->FirstChildElement("Decorations");
		if (!pElem)
			throw sbx::ParseNoElementException("No 'Decorations' element", &doc);		
		parseXML(pElem);
	}
	
	void Decorations::parseXML(const TiXmlElement *element)
	{
		for (const TiXmlElement *el = element->FirstChildElement(); el; el = el->NextSiblingElement()) {
			Decoration dec;
			if (el->Attribute("box")) {
				std::string boxstr = el->Attribute("box");
				if (boxstr == "up")
					dec.box = FL_UP_BOX;
				else if (boxstr == "thinup")
					dec.box = FL_THIN_UP_BOX;
				else if (boxstr == "down")
					dec.box = FL_DOWN_BOX;
				else if (boxstr == "thindown")
					dec.box = FL_THIN_DOWN_BOX;
				else if (boxstr == "engraved")
					dec.box = FL_ENGRAVED_BOX;
				else if (boxstr == "embossed")
					dec.box = FL_EMBOSSED_BOX;
				else if (boxstr == "border")
					dec.box = FL_BORDER_BOX;
				else if (boxstr == "shadow")
					dec.box = FL_SHADOW_BOX;
				else if (boxstr == "rounded")
					dec.box = FL_ROUNDED_BOX;
				else if (boxstr == "rshadow")
					dec.box = FL_RSHADOW_BOX;
				else if (boxstr == "roundup")
					dec.box = FL_ROUND_UP_BOX;
				else if (boxstr == "rounddown")
					dec.box = FL_ROUND_DOWN_BOX;
				else
					throw sbx::ParseException("Unknown box type '" + boxstr + "'", el);
			}
			if (el->Attribute("image")) {
				std::string filename = sbx::XMLParser::instance().getPath().find(el->Attribute("image"));
				if (filename.length() > 0) {
					int w = sbx::XMLParser::parseIntAttribute(el, "image_w", true, 0);
					int h = sbx::XMLParser::parseIntAttribute(el, "image_h", true, 0);
					dec.image = Fl_Shared_Image::get(filename.c_str(), w, h);
				} else
					sbx::dout(sbx::WARN) << "Decoration image '" << el->Attribute("image") << "' not found\n";
			}
			dec.color = sbx::XMLParser::parseVector3Attribute(el, "color", true, dec.color);
			
			std::string type = el->Value();
			if (type.find("_") != std::string::npos)
				type = type.substr(0,type.find("_")) + "::" + type.substr(type.find("_")+1);
			sbx::dout(5) << "  decoration " << type << "\n";
			decorations[type] = dec;
		}
	}
	
	void Decorations::apply(const std::string& name, Fl_Widget *widget)
	{
		if (!widget)
			return;
		DecorationMap::iterator it = decorations.find(name);
		if (it == decorations.end())
			return;
		Decoration& dec = it->second;
		widget->box(dec.box);
		if (dec.image)
			widget->image(dec.image);
		if (dec.color.norm() > 1e-5)
		  widget->color(fl_rgb_color((uchar)(dec.color[0]*255), (uchar)(dec.color[1]*255), (uchar)(dec.color[2]*255)));
	}
	
	Decorations::Decorations()
	{
	}
	
	Decorations& Decorations::instance()
	{
		if (!instanceptr)
			instanceptr = new Decorations;
		return *instanceptr;
	}
	
	Decorations* Decorations::instanceptr = NULL;

} // namespace
