/*
 *  Ports.h
 *  SimBlox
 *
 *  Created by Martin Insulander on 2007-11-18.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

// TODO inline keyword to make sure things go zuper fazt?

#ifndef PORTS_H
#define PORTS_H

#include "Export.h"
#include "units/units.h"
#include <string>
#include <sstream>
#include <vector>
#include <smrt/observer_ptr.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

namespace sbx
{
	
	// Some forward declarations
	class Model;
	class Port;
	template <typename T> class InPort;
	template <typename T> class OutPort;
	template <typename T> class InUnitPort;
	template <typename T> class OutUnitPort;
	
	class SIMBLOX_API PortException : public std::exception {
	public:
		PortException(const std::string& message = "", const Port* port1 = NULL, const Port* port2 = NULL);
		~PortException() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
		std::string message;
		const Port *port1, *port2;
	};
	
	
	/// Base class for input/output ports.
	class SIMBLOX_API Port
	{
	public:
		
		Port() : owner(NULL) { }
		
		/// Connect this port to another port
		/** \throw PortException when an illegal connection is attempted */
		virtual void connect(Port* otherend)=0;
		/// Returns true if this port can connect to the other port
		virtual bool canConnect(Port *otherend)=0;
		/// Disconnect this port from another port
		virtual void disconnect(Port* otherend)=0;
		/// Disconnect all connections held by this port
		virtual void disconnect()=0;
		/// Returns connection status
		virtual bool isConnected()=0;
		/// Returns wether the ports is connected to another port
		virtual bool isConnectedTo(Port *otherend)=0;
		/// Returns the number of connections this port has
		virtual unsigned int getNumConnections()=0;
		/// Returns the other end of a port connection
		virtual Port* getOtherEnd(unsigned int index=0)=0;
		/// Returns true if the port has valid data available (through a connection, default value or other means)
		virtual bool isValid()=0;
		
		/// Returns the model that owns this port
		Model* getOwner() { return owner.get(); } 
		const Model* getOwner() const { return owner.get(); }
		
		/// Get the unit of the value this port holds, if supported
		const std::string& getUnit() const { return unit; }
		/// Get the unit of the value this port holds, if supported
		/** \throw PortException if called on any other than InUnitPort and OutUnitPort */
		virtual void setUnit(const std::string& unit) { if (unit.length() > 0) throw PortException("Specified port does not support units", this, NULL); }
		bool isInput() { return input; }
		bool isOutput() { return !input; }
		
	protected:
		void setOwner(Model* owner) { this->owner = owner; }

		void notifyOwnerConnect(Port *otherend);
		void notifyOwnerDisconnect(Port *otherend);
		
		std::string unit;
		smrt::observer_ptr<Model> owner;
		bool input;
		
		friend class Model;
	};
	
	/// Input port interface.
	class InputPort : public Port
		{
		public:
			InputPort() { input = true; loose = false; }
			virtual void connect(Port* otherend)=0;
			virtual bool canConnect(Port *otherend)=0;
			virtual void disconnect(Port* otherend)=0;
			virtual void disconnect()=0;
			virtual bool isConnected()=0;
			virtual bool isConnectedTo(Port *otherend)=0;
			virtual unsigned int getNumConnections()=0;
			virtual Port* getOtherEnd(unsigned int index=0)=0;
			virtual bool isValid()=0;
			
			/// Set "loose" state of this port
			/** A "loose" input doesn't mind if data is from a previous iteration. This helps solving "looping dependencies",
			 i.e. when two models have both inputs and outputs connected to each other. It also doesn't care about update
			 passes.
			 \todo Rename "loose" to something more sensical
			 */
			void setLoose(const bool loose) { this->loose = loose; }
			const bool isLoose() { return loose; }
		protected:
			bool loose; // TODO find a better name
		};
	
	/// Type specific implementation of an input port.
	template <typename T>
	class InPort : public InputPort
		{
		public:
			InPort() :
			connection_out(NULL),
			connection_in(NULL),
			useDefault(false) { }
			~InPort() { disconnect(); }
			
			virtual void connect(Port* otherend)
			{
				disconnect();
				if (otherend == this)
					throw PortException("Cannot connect a port to itself", this);
				else if (!otherend)
					throw PortException("NULL port connection", this);
				else if (dynamic_cast<OutPort<T>*>(otherend)) {
					doConnect((OutPort<T>*)otherend);
					connection_out->doConnect(this);
				} else if (dynamic_cast<InPort<T>*>(otherend)) {
					doConnect((InPort<T>*)otherend);
					//connection_in->doConnect(this);
				} else
					throw PortException("Invalid port connection",this,otherend); 
			}
			
			virtual bool canConnect(Port* otherend)
			{
				if (otherend == this)
					return false;
				if (dynamic_cast<OutPort<T>*>(otherend) || dynamic_cast<InPort<T>*>(otherend))
					return true;
				return false;
			}
			
			virtual void disconnect(Port* otherend)
			{
				//std::cout << "InPort disconnect" << std::endl;
				if (!otherend)
					return;
				if (otherend == connection_out) {
					connection_out->doDisconnect(this);
					doDisconnect(connection_out);
				} else if (otherend == connection_in) {
					doDisconnect(connection_in);
				}
			}
			
			virtual void disconnect()
			{
				disconnect(connection_out);
				disconnect(connection_in);
			}
			
			virtual bool isConnected() { return (connection_out || connection_in); }
			virtual bool isConnectedTo(Port *otherend)
			{
				return (connection_out == otherend || connection_in == otherend);
			}
			virtual unsigned int getNumConnections() 
			{ 
				if (isConnected())
					return 1;
				return 0;
			}
			virtual Port* getOtherEnd(unsigned int index=0) 
			{
				if (index != 0)
					throw PortException("Invalid connection index", this);
				if (connection_out) 
					return connection_out; 
				else 
					return connection_in; 
			}
			virtual bool isValid() { return (connection_out || (connection_in && connection_in->isValid()) || useDefault); }
			
			/// Set a default value which will be returned by get() when not connected
			void setDefault(const T& value) { useDefault = true; default_value = value; }
			
			/// Get the value
			/** \return the value of the connected port or, if unconnected and a default value is set,
			 the default value.
			 \see setDefault(), operator*() */
			virtual const T get() const {
				if (connection_out)
					return connection_out->get();
				else if (connection_in)
					return connection_in->get();
				else {
					if (useDefault)
						return default_value;
					else
						throw PortException("Access to unconnected port",this);
				}
			}
			
			/// Operator for convenience, same as get()
			const T operator*() const { return get(); }
			/// Simply throws an exception, provided for "safety"...
			/** \throw PortException when used */
			void operator=(const T& value) { throw PortException("Attempt to assign value of InPort",this); }
			
		protected:
			virtual void doConnect(OutPort<T>* otherend)
			{
				connection_out = otherend;
				notifyOwnerConnect(otherend);
				otherend->notifyOwnerConnect(this);
			}
			
			virtual void doConnect(InPort<T>* otherend)
			{
				connection_in = otherend;
				notifyOwnerConnect(otherend);
				otherend->notifyOwnerConnect(this);
			}
			
			void doDisconnect(Port *otherend)
			{ 
				if (otherend == connection_out) {
					notifyOwnerDisconnect(connection_out);
					connection_out->notifyOwnerDisconnect(this);
					connection_out = NULL;
				} else if (otherend == connection_in) {
					notifyOwnerDisconnect(connection_in);
					connection_in->notifyOwnerDisconnect(this);
					connection_in = NULL;
				}
			}
			
			OutPort<T>* connection_out;
			InPort<T>* connection_in;
			friend class OutPort<T>;
			bool useDefault;
			T default_value;
		};
	
	/// Type specific implementation of an input port with on-the-fly unit conversion.
	template <typename T>
	class InUnitPort : public InPort<T>
	{
	public:
		InUnitPort(const std::string& unit = "") : InPort<T>(),
		unitScaleFactor(1)
		{
			this->unit = unit;
		}
		
		virtual void connect(Port* otherend)
		{
			this->disconnect();
			if (otherend == this)
				throw PortException("Cannot connect a port to itself", this);
			else if (!otherend)
				throw PortException("NULL port connection", this);
			else if (dynamic_cast<OutUnitPort<T>*>(otherend)) {
				doConnect((OutUnitPort<T>*)otherend);
				((OutUnitPort<T>*)this->connection_out)->doConnect(this);
			} else if (dynamic_cast<InUnitPort<T>*>(otherend)) {
				doConnect((InUnitPort<T>*)otherend);
				((InUnitPort<T>*)this->connection_in)->doConnect(this);
			} else if (InPort<T>::canConnect(otherend)) {
				unitScaleFactor = 1;
				return InPort<T>::connect(otherend);
			} else
				throw PortException("Invalid port connection",this,otherend); 
		}
		
		virtual bool canConnect(Port* otherend)
		{
			if (otherend == this)
				return false;
			if (dynamic_cast<OutUnitPort<T>*>(otherend) || dynamic_cast<InUnitPort<T>*>(otherend) || InPort<T>::canConnect(otherend))
				return true;
			return false;
		}
		
		virtual const T get() const {
			return unitScaleFactor*InPort<T>::get();
		}
		
		virtual void setUnit(const std::string& unit) { this->unit = unit; }
		void operator=(const T& value) { throw PortException("Attempt to assign value of InUnitPort",this); }
		
	protected:
		virtual void doConnect(OutUnitPort<T>* otherend)
		{
			if (this->unit.length() > 0 && otherend->unit.length() > 0)
				unitScaleFactor = units::convert(1.0,otherend->unit, this->unit);
			InPort<T>::doConnect(otherend);
		}
		
		virtual void doConnect(InUnitPort<T>* otherend)
		{
			if (this->unit != "" && otherend->unit != "")
				unitScaleFactor = units::convert(1.0,otherend->unit, this->unit);
			InPort<T>::doConnect(otherend);
		}
		
		double unitScaleFactor;
		friend class OutUnitPort<T>;
	};
	
	/// Output port.
	class OutputPort : public Port
		{
		public:
			OutputPort() { input = false; }
			virtual void connect(Port* otherend)=0;
			virtual bool canConnect(Port* otherend)=0;
			virtual void disconnect(Port* otherend)=0;
			virtual void disconnect()=0;
			virtual bool isConnected()=0;
			virtual bool isConnectedTo(Port *otherend)=0;
			virtual bool isValid()=0;
			virtual unsigned int getNumConnections()=0;
			virtual Port* getOtherEnd(unsigned int index)=0;
		};
	
	/// Type specific implementation of an output port.
	template <typename T>
	class OutPort : public OutputPort
		{
		public:
			OutPort() : value_ptr(NULL) { }
			OutPort(const OutPort& source) : value(source.value), value_ptr(NULL), mutex() { }
			~OutPort() { disconnect(); }
			
			virtual void connect(Port* otherend)
			{
				if (isConnectedTo(otherend))
					return;
				else if (!otherend)
					throw PortException("NULL port connection", this);
				if (dynamic_cast<InPort<T>*>(otherend)) {
					connections.push_back((InPort<T>*)otherend);
					((InPort<T>*)otherend)->doConnect(this);
				} else
					throw PortException("Invalid port connection",this,otherend);
			}
			
			virtual bool canConnect(Port* otherend)
			{
				if (dynamic_cast<InPort<T>*>(otherend))
					return true;
				return false;
			}
			
			virtual void disconnect(Port* otherend)
			{
				InPort<T>* otherendIn = dynamic_cast<InPort<T>*>(otherend);
				if (!otherendIn)
					return;
				doDisconnect(otherendIn);
				otherendIn->doDisconnect(this);
			}
			
			virtual void disconnect()
			{
				while (connections.size() > 0)
					disconnect(connections[0]);
			}
			
			virtual bool isConnected() { return (connections.size() > 0); }
			virtual bool isConnectedTo(Port *otherend)
			{
				for (size_t i = 0; i < connections.size(); i++)
					if (connections[i] == otherend)
						return true;
				return false;
			}
			
			virtual bool isValid() { if (value_ptr) return (value_ptr != NULL); else return true; }
			virtual unsigned int getNumConnections() { return connections.size(); }
			virtual Port* getOtherEnd(unsigned int index) { return connections[index]; }

			virtual const T& get() const
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock((OpenThreads::Mutex&)mutex);
				if (value_ptr)
					return *value_ptr;
				else
					return value;
			}

			const T& operator*() const { return get(); }
			virtual void set(const T& value)
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
				this->value = value;
			}
			void operator=(const T& value) { set(value); }
			void setPtr(T* value_ptr)
			{ 
				OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);
				this->value_ptr = value_ptr;
			}
		protected:
			virtual void doConnect(InPort<T>* otherend)
			{ 
				connections.push_back(otherend);
			}
			
			virtual void doDisconnect(InPort<T>* otherend)
			{
				if (connections.size() == 0)
					return;
				for (int i = 0; i < connections.size(); i++)
					if (connections[i] == otherend) {
						connections.erase(connections.begin()+i);
						return;
					}
			}
			
			typedef std::vector<InPort<T>*> ConnectionList;
			ConnectionList connections;
			T value;
			T* value_ptr;
			OpenThreads::Mutex mutex;
			friend class InPort<T>;
		};
	
	/// Type specific implementation of an output port with on-the-fly unit conversion.
	template <typename T>
	class OutUnitPort : public OutPort<T> {
	public:
		OutUnitPort(const std::string& unit = "") : OutPort<T>()
		{
			this->unit = unit;
		}
		
		virtual void setUnit(const std::string& unit) { this->unit = unit; }
		void operator=(const T& value) { set(value); }
	protected:
		friend class InUnitPort<T>;
	};
	
	typedef InPort<double> InPortd;
	typedef OutPort<double> OutPortd;
	typedef InUnitPort<double> InUnitPortd;
	typedef OutUnitPort<double> OutUnitPortd;
	
}

#endif

/**
 \class sbx::Port
 
 Ports are used to connect simulation models. In addition to providing a means of passing data, they
 can also used to resolve dependencies and the order of update models.
 
 An input port can be connected to <b>one</b> input or output port. A default value (which is used when the port is not connected)
 can be specified. If the port is not connected and no default value is specified, a PortException will be
 thrown if access is attempted. When connecting to another port, units::convert() is called if both
 ports have units specified, and a unitScaleFactor is stored for subsequent access.
 
 An output port can be connected to <b>one or more</b> input ports. A value pointer is set using setPtr(). If no pointer
 is set, the output port can store a value (using operator=() or set()).
 
 The actual implementation of ports is implemented using templates, so that any data (e.g. strings and
 vectors) can be passed between models. However, the user is advised to stick to ports of template type
 \c double as much as possible. Note that, for example, an InPort<double> and OutPort<double> cannot be
 connected (and vice versa). Hence the provided typedefs InPortd and OutPortd.
 
 A small example:
 \code
 OutPortd out;
 InPortd in, anotherin;
 out.connect(&in);
 out.connect(&anotherin);
 out = 47.11;
 std::cout << "the value is " << *in << ", indeed " << anotherin.get() << "\n";
 \endcode
 
 \see Model
 
 \class sbx::InputPort
 
 See Port for an overview of the concept.
 
 \class sbx::OutputPort
 
 See Port for an overview of the concept.
 */


