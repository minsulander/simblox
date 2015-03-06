/*
 *  Ports.cpp
 *  SimBlox
 *
 *  Created by Martin Insulander on 2007-11-18.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Model.h"
#include "Ports.h"

namespace sbx
{
	
	// Most of the code is template classes and must reside in the header, so 
	// that's why not a lot goes here...
	
	void Port::notifyOwnerConnect(Port *otherend)
	{
		if (owner.valid())
			owner->onPortConnect(this, otherend);
	}
	
	void Port::notifyOwnerDisconnect(Port *otherend)
	{
		if (owner.valid())
			owner->onPortDisconnect(this, otherend);
	}
	
	PortException::PortException(const std::string& message, const Port *port1, const Port *port2)
	{ 
		std::stringstream ss;
		ss << "PortException";
		if (port1 && port1->getOwner())
			ss << " for port '" << port1->getOwner()->getPath() << "." << port1->getOwner()->getPortName(port1) << "'";
		if (port2 && port2->getOwner())
			ss << " and " << port2->getOwner()->getPath() << "." << port2->getOwner()->getPortName(port2) << "'";
		ss << ": " << message;
		this->message = ss.str();
		this->port1 = port1;
		this->port2 = port2;
	}
	
	
	
} // namespace sbx
