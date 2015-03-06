#ifndef BLACKBOX_H
#define BLACKBOX_H

#include "Export.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <exception>
#include <map>

#ifdef HAVE_PRACTICAL_SOCKETS
#include "PracticalSockets/PracticalSocket.h"
#endif

namespace sbx
{
	static const int BLACKBOX_FILE_VER = 1;
	
	class BlackBoxException : public std::exception
	{
	public:
		BlackBoxException(const std::string& message = "") { this->message = std::string("BlackBoxException: ") + message; }
		~BlackBoxException() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
	protected:
		std::string message;
	};

	/** Base class for loggers etc. */
	
	class SIMBLOX_API BlackBoxDataHandler
	{
	public:
		BlackBoxDataHandler();
		~BlackBoxDataHandler();
		void subscribe(const std::string& varname);
		void subscribeAll();
		void subscribeGroup(const std::string& groupname);
		void unsubscribe(const std::string& varname);
		void unsubscribeAll();
		void unsubscribeGroup(const std::string& groupname);
		virtual void fetch();
		virtual void write(int numentries=0)=0;
		void setHeader(const std::string& hdr) { header = hdr; }
	protected:
		unsigned int logcount;
		std::vector<unsigned int> varindices;
		std::string header;
	};
	
	/** Logger that saves values in a CSV (comma separated values) file. */
	
	class SIMBLOX_API BlackBoxTextLog : public BlackBoxDataHandler
	{
	public:
		BlackBoxTextLog(const std::string& filename = "datalog.csv");
		~BlackBoxTextLog();
		const std::string& getFileName() const { return filename; }
		
		virtual void fetch();
		virtual void write(int numlines=0);

	protected:
		std::string filename;
		std::ofstream fout;
		std::deque<std::string> lines;
		std::string separator;
	};
	
	/** Logger that save values (floating point only) in a proprietary binary format. */
	
	class SIMBLOX_API BlackBoxBinaryLog : public BlackBoxDataHandler
	{
	public:
		BlackBoxBinaryLog(const std::string& filename = "datalog.bblog");
		~BlackBoxBinaryLog();
		const std::string& getFilename() const { return filename; }
		
		virtual void fetch();
		virtual void write(int numentries=0);
		void useDoublePrecision(const bool use) { doubleprecision = use; }
		
	protected:
		std::string filename;
		std::ofstream fout;
		std::deque<double> values;
		unsigned int writecount;
		bool doubleprecision;
	};

#ifdef HAVE_PRACTICAL_SOCKETS	
	/** Data handler that send values via UDP to a remote host. */
	
	class SIMBLOX_API BlackBoxUDPSender : public BlackBoxDataHandler
	{
	public:
		BlackBoxUDPSender(const std::string& hostname, const int port);
		~BlackBoxUDPSender();
		
		virtual void fetch();
		void writeHeader();
		virtual void write(int numentries=0);
		void useDoublePrecision(const bool use) { doubleprecision = use; }
		
	protected:
		std::string hostname;
		int port;
		UDPSocket socket;
		std::deque<double> values;
		unsigned int writecount;
		bool doubleprecision;
	};
#endif

	/** Practical data handling class. Lets you register variables by name, type and pointer. 
		Then add loggers etc to log data or handle the data in any way using the BlackBoxDataHandler class */
	
	class SIMBLOX_API BlackBox
	{
	public:

		struct LogVariable
		{
			enum VariableType {
				INVALID,
				FLOAT,
				DOUBLE,
				INT,
				BOOLEAN,
				STRING,
				CSTR
			};
			
			LogVariable() : name(""), type(INVALID), group_ptr(0), value_ptr(0), enabled(true) {}
			std::string name;
			VariableType type;
			const void* group_ptr;
			const void* value_ptr;
			bool enabled;
		};
		
		BlackBox();
		~BlackBox();
		void addHandler(BlackBoxDataHandler* log);
		void removeHandler(BlackBoxDataHandler* log);
		const LogVariable& getVariable(unsigned int index) { return variables[index]; }
		unsigned int getNumVariables() { return variables.size(); }
		void registerVariable(const std::string& name, const void* ptr, LogVariable::VariableType type);
		void registerFloat(const std::string& name, const float* ptr) { registerVariable(name,(const void*) ptr,LogVariable::FLOAT); }
		void registerDouble(const std::string& name, const double* ptr) { registerVariable(name,(const void*) ptr,LogVariable::DOUBLE); }
		void registerint(const std::string& name, const int* ptr) { registerVariable(name,(const void*) ptr,LogVariable::INT); }
		void registerBool(const std::string& name, const bool* ptr) { registerVariable(name,(const void*) ptr,LogVariable::BOOLEAN); }
		void beginGroup(const void *ptr, const std::string& name);
		void endGroup();
		void setGroupName(const void *ptr, const std::string& name);
		const std::string& getGroupName(const void* ptr) { return groupnames[ptr]; }
		void unregisterVariable(const void* ptr); // TODO make one with name as parameter (that can handle 'group.name')
		void unregisterGroup(const void *ptr);
		void unregisterGroup(const std::string& name) { unregisterGroup(findGroupPtr(name)); }
		void fetch();
		void write(int numlines=0);
		static BlackBox& instance();
		int findVariableIndex(const std::string& name);
		const void* findGroupPtr(const std::string& name);

		
	protected:			
		
		std::vector<LogVariable> variables;
		std::vector<BlackBoxDataHandler*> logs;
		static BlackBox* instance_ptr;
		const void* curgroup;
		std::map<const void*,std::string> groupnames;
	};
}

#endif
