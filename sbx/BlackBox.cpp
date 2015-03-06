#include "BlackBox.h"
#include <sstream>

#define MAX_PACKET_SIZE 4096

namespace sbx
{
	
	BlackBoxDataHandler::BlackBoxDataHandler()
	{
		logcount = 0;
		header = "BlackBoxDataHandler";
		BlackBox::instance().addHandler(this);
	}
	
	BlackBoxDataHandler::~BlackBoxDataHandler()
	{
		BlackBox::instance().removeHandler(this);
	}
	
	void BlackBoxDataHandler::fetch()
	{
		logcount++;
	}
	
	void BlackBoxDataHandler::subscribe(const std::string& name)
	{
		// Allow a bunch of separators for multiple variables (whitespace and ,;:)
		std::string str = name;
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");
		
		std::stringstream names(str);
		std::string varname;
		while (names >> varname) {
			if (varname == "*") // Add '*' - all variables
				subscribeAll();
			else if (varname.at(varname.length()-1) == '*') { // Add 'group.*' - all variables in a group
				varname = varname.substr(0,varname.length()-2);
				subscribeGroup(varname);
			} else {
				int index = BlackBox::instance().findVariableIndex(varname);
				if (index < 0)
					throw BlackBoxException(std::string("Unknown variable '") + varname + "'");
				varindices.push_back(index);
			}
		}
	}
	
	void BlackBoxDataHandler::subscribeAll()
	{
		varindices.clear();
		for (unsigned int i = 0; i < BlackBox::instance().getNumVariables(); i++)
			varindices.push_back(i);
	}
	
	void BlackBoxDataHandler::subscribeGroup(const std::string& groupname)
	{
		const void* group_ptr = BlackBox::instance().findGroupPtr(groupname);
		if (!group_ptr)
			throw BlackBoxException(std::string("Unknown group '") + groupname + "'");
		for (unsigned int i = 0; i < BlackBox::instance().getNumVariables(); i++)
			if (BlackBox::instance().getVariable(i).group_ptr == group_ptr)
				varindices.push_back(i);
	}
	
	void BlackBoxDataHandler::unsubscribe(const std::string& name)
	{
		// Allow a bunch of separators for multiple variables (whitespace and ,;:)
		std::string str = name;
		int i;
		while ((i = str.find_first_of(",;:")) > 0)
			str.replace(i,1," ");
		
		std::stringstream names(str);
		std::string varname;
		while (names >> varname) {
			if (varname == "*") // Remove '*' - all variables
				unsubscribeAll();
			else if (varname.at(varname.length()-1) == '*') { // Remove 'group.*' - all variables in a group
				varname = varname.substr(0,varname.length()-2);
				unsubscribeGroup(varname);
			} else {
				int index = BlackBox::instance().findVariableIndex(varname);
				if (index < 0)
					throw BlackBoxException(std::string("Unknown variable '") + varname + "'");
				for (std::vector<unsigned int>::iterator i = varindices.begin(); i != varindices.end(); i++)
					if (*i == index)
						varindices.erase(i);
			}
		}
	}
	
	void BlackBoxDataHandler::unsubscribeAll()
	{
		varindices.clear();
	}
	
	void BlackBoxDataHandler::unsubscribeGroup(const std::string& groupname)
	{
		const void* group_ptr = BlackBox::instance().findGroupPtr(groupname);
		if (!group_ptr)
			throw BlackBoxException(std::string("Unknown group '") + groupname + "'");
		for (std::vector<unsigned int>::iterator i = varindices.begin(); i != varindices.end(); i++) {
			const BlackBox::LogVariable& var = BlackBox::instance().getVariable(*i);
			if (var.group_ptr == group_ptr) {
				varindices.erase(i);
				i--;
			}
		}
	}
	
	BlackBoxTextLog::BlackBoxTextLog(const std::string& filename) : BlackBoxDataHandler()
	{
		this->filename = filename;
		fout.open(filename.c_str());
		separator = "\t";
		header = "BlackBoxTextLog";
	}
	
	BlackBoxTextLog::~BlackBoxTextLog()
	{
		BlackBox::instance().removeHandler(this);
		if (!fout)
			return;
		if (!lines.empty())
			write();
		fout.close();
	}
	
	void BlackBoxTextLog::fetch()
	{
		if (logcount == 0) {
			// Write header
			lines.push_back(std::string("%") + header + "\n"); // TODO date, version info and stuff maybe?
			std::stringstream shead;
			shead << "%";
			for (unsigned int i = 0; i < varindices.size(); i++) {
				const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
				if (!var.enabled) continue;
				if (i > 0)
					shead << separator;
				if (var.group_ptr)
					shead << BlackBox::instance().getGroupName(var.group_ptr) << ".";
				shead << var.name;
			}
			shead << "\n";
			lines.push_back(shead.str());
		}
		std::stringstream sout;
		for (unsigned int i = 0; i < varindices.size(); i++) {
			const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
			if (i > 0)
				sout << separator;
			if (!var.enabled) continue;
			switch (var.type) {
				case BlackBox::LogVariable::FLOAT: sout << *((const float*) var.value_ptr); break;
				case BlackBox::LogVariable::DOUBLE: sout << *((const double*) var.value_ptr); break;
				case BlackBox::LogVariable::INT: sout << *((const int*) var.value_ptr); break;
				case BlackBox::LogVariable::BOOLEAN: sout << *((const bool*) var.value_ptr); break;
			}
		}
		sout << "\n";
		lines.push_back(sout.str());
		logcount++;
	}
	
	void BlackBoxTextLog::write(int numlines)
	{
		if (!fout.good())
			throw BlackBoxException(std::string("Failed to write to file '") + filename + "'");
		for (int i = 0; (i < numlines || numlines == 0) && fout.good() && !lines.empty(); i++) {
			fout << lines.front();
			lines.pop_front();
		}
		fout.flush();
	}
	
	BlackBoxBinaryLog::BlackBoxBinaryLog(const std::string& filename) : BlackBoxDataHandler()
	{
		this->filename = filename;
		fout.open(filename.c_str(),std::ios::binary);
		header = "BlackBoxBinaryLog";
		writecount = 0;
		doubleprecision = false;
	}
	
	BlackBoxBinaryLog::~BlackBoxBinaryLog()
	{
		BlackBox::instance().removeHandler(this);
		if (!fout)
			return;
		if (!values.empty())
			write();
		fout.close();
	}
	
	void BlackBoxBinaryLog::fetch()
	{
		for (unsigned int i = 0; i < varindices.size(); i++) {
			const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
			if (!var.enabled)
				throw BlackBoxException(std::string("BinaryLog can't handle disabled variables")); // TODO fix
			double val;
			switch (var.type) {
				case BlackBox::LogVariable::FLOAT: val = (double) *((const float*) var.value_ptr); break;
				case BlackBox::LogVariable::DOUBLE: val = (double) *((const double*) var.value_ptr); break;
				case BlackBox::LogVariable::INT: val = (double) *((const int*) var.value_ptr); break;
				case BlackBox::LogVariable::BOOLEAN: val = (double) *((const bool*) var.value_ptr); break;
				default:
					throw BlackBoxException(std::string("BinaryLog can only handle numeric variables"));
			}
			values.push_back(val);
		}
		logcount++;
	}
	
	void BlackBoxBinaryLog::write(int numentries)
	{
		if (!fout.good())
			throw BlackBoxException(std::string("Failed to write to file '") + filename + "'");
		if (writecount == 0) {
			// Write header
			fout.put(BLACKBOX_FILE_VER);
			fout.put((char)doubleprecision);
			unsigned int len = header.length();
			fout.write((char*)&len,sizeof(unsigned int));
			fout << header;
			std::stringstream shead;
			for (unsigned int i = 0; i < varindices.size(); i++) {
				const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
				if (!var.enabled) continue;
				if (i > 0)
					shead << ",";
				if (var.group_ptr)
					shead << BlackBox::instance().getGroupName(var.group_ptr) << ".";
				shead << var.name;
			}
			len = shead.str().length();
			fout.write((char*)&len,sizeof(unsigned int));
			fout << shead.str();
			len = varindices.size();
			fout.write((char*)&len,sizeof(unsigned int));
			writecount++;
		}
		double val;
		float fval;
		for (int i = 0; (i < numentries || numentries == 0) && fout.good() && !values.empty(); i++) {
			for (int j = 0; j < varindices.size(); j++) {
				if (doubleprecision) {
					val = values.front();
					fout.write((char*)&val,sizeof(double));
				} else {
					fval = values.front();
					fout.write((char*)&fval,sizeof(float));
				}
				values.pop_front();
			}
		}
		fout.flush();
		writecount += numentries;
	}
	
#ifdef HAVE_PRACTICAL_SOCKETS
	BlackBoxUDPSender::BlackBoxUDPSender(const std::string& hostname, const int port) : BlackBoxDataHandler()
	{
		this->hostname = hostname;
		this->port = port;
		writecount = 0;
		doubleprecision = false;
	}
	
	BlackBoxUDPSender::~BlackBoxUDPSender()
	{
		BlackBox::instance().removeHandler(this);
	}
	
	void BlackBoxUDPSender::fetch()
	{
		for (unsigned int i = 0; i < varindices.size(); i++) {
			const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
			if (!var.enabled)
				throw BlackBoxException(std::string("UDPSender can't handle disabled variables")); // TODO fix
			double val;
			switch (var.type) {
				case BlackBox::LogVariable::FLOAT: val = (double) *((const float*) var.value_ptr); break;
				case BlackBox::LogVariable::DOUBLE: val = (double) *((const double*) var.value_ptr); break;
				case BlackBox::LogVariable::INT: val = (double) *((const int*) var.value_ptr); break;
				case BlackBox::LogVariable::BOOLEAN: val = (double) *((const bool*) var.value_ptr); break;
				default:
					throw BlackBoxException(std::string("UDPSender can only handle numeric variables"));
			}
			values.push_back(val);
		}
		logcount++;
	}
	
	char sendBuffer[MAX_PACKET_SIZE]; // FIXME not thread safe - only one UDPsender can be used at a time right now
	
	void BlackBoxUDPSender::writeHeader()
	{
		std::stringstream sheader;
		sheader << '!';
		for (unsigned int i = 0; i < varindices.size(); i++) {
			const BlackBox::LogVariable& var = BlackBox::instance().getVariable(varindices[i]);
			if (!var.enabled) continue;
			if (i > 0)
				sheader << " ";
			if (var.group_ptr)
				sheader << BlackBox::instance().getGroupName(var.group_ptr) << ".";
			sheader << var.name;
		}
		sheader << '\n';
		int len = sheader.str().length();
		socket.sendTo(sheader.str().c_str(),len,hostname,port);
		writecount++;
	}
	
	void BlackBoxUDPSender::write(int numentries)
	{
		if (writecount == 0)
			writeHeader();
		double val;
		float fval;
		for (int i = 0; (i < numentries || numentries == 0) && !values.empty(); i++) {
			sendBuffer[0]='>';
			if (doubleprecision)
				sendBuffer[1]=1;
			else
				sendBuffer[1]=0;
			char *pbuf = sendBuffer+2;
			unsigned int u = varindices.size();
			*((unsigned int*)pbuf) = u;
			pbuf += sizeof(unsigned int);
			for (int j = 0; j < varindices.size(); j++) {
				if (doubleprecision) {
					val = values.front();
					*((double*)pbuf) = val;
					pbuf += sizeof(double);
				} else {
					fval = values.front();
					*((float*)pbuf) = fval;
					pbuf += sizeof(float);
				}
				values.pop_front();
			}
			*pbuf='\n'; pbuf++;
			socket.sendTo(sendBuffer,(int)(pbuf-sendBuffer),hostname,port);
		}
		writecount += numentries;
	}
	
#endif
	
	
	BlackBox* BlackBox::instance_ptr = NULL;
	
	BlackBox::BlackBox()
	{
		groupnames[0] = "";
		curgroup = 0;
	}
	
	BlackBox::~BlackBox()
	{
	}
	
	void BlackBox::addHandler(BlackBoxDataHandler* log)
	{
		// Check that the log is not already there or filename isn't taken
		for (std::vector<BlackBoxDataHandler*>::iterator i = logs.begin(); i < logs.end(); i++) {
			if (*i == log)
				return;
			/*
			 if ((*i)->getFileName() == log->getFileName())
			 throw BlackBoxException(std::string("Log filename '") + log->getFileName() + "' is taken");
			 */
		}
		logs.push_back(log);
	}
	
	void BlackBox::removeHandler(BlackBoxDataHandler* log)
	{
		for (std::vector<BlackBoxDataHandler*>::iterator i = logs.begin(); i < logs.end(); i++)
			if (*i == log)
				logs.erase(i--);
	}
	
	void BlackBox::registerVariable(const std::string& name, const void* ptr, LogVariable::VariableType type)
	{
		LogVariable logv;
		logv.name = name;
		logv.type = type;
		logv.group_ptr = curgroup;
		logv.value_ptr = ptr;
		variables.push_back(logv);
	}
	
	void BlackBox::beginGroup(const void* ptr, const std::string& group)
	{
		if (curgroup != 0)
			throw BlackBoxException("beginGroup() called again before endGroup()");
		groupnames[ptr] = group;
		curgroup = ptr;
	}
	
	void BlackBox::endGroup()
	{
		curgroup = 0;
	}
	
	void BlackBox::setGroupName(const void *ptr, const std::string& name)
	{
		groupnames[ptr] = name;
	}
	
	void BlackBox::unregisterVariable(const void *ptr)
	{
		if (!ptr) return;
		for (std::vector<LogVariable>::iterator i = variables.begin(); i < variables.end(); i++) {
			if (i->value_ptr == ptr) {
				i->enabled = false;
				i->value_ptr = NULL;
				//variables.erase(i--);
			}
		}
	}
	
	void BlackBox::unregisterGroup(const void* ptr)
	{
		if (!ptr) return;
		for (std::vector<LogVariable>::iterator i = variables.begin(); i < variables.end(); i++) {
			if (i->group_ptr == ptr) {
				i->enabled = false;
				i->value_ptr = NULL;
				i->group_ptr = NULL;
				//variables.erase(i--);
			}
		}	
		std::map<const void*,std::string>::iterator i = groupnames.find(ptr);
		if (i != groupnames.end())
			groupnames.erase(i);
	}
	
	void BlackBox::fetch()
	{
		for (std::vector<BlackBoxDataHandler*>::iterator i = logs.begin(); i < logs.end(); i++)
			(*i)->fetch();
	}
	
	void BlackBox::write(int numlines)
	{
		for (std::vector<BlackBoxDataHandler*>::iterator i = logs.begin(); i < logs.end(); i++)
			(*i)->write(numlines);
	}
	
	BlackBox& BlackBox::instance()
	{
		if (instance_ptr)
			return *instance_ptr;
		else {
			instance_ptr = new BlackBox();
			return *instance_ptr;
		}
	}
	
	int BlackBox::findVariableIndex(const std::string& name)
	{
		std::string groupname, varname;
		int groupsepindex = name.find('.',0);
		const void* group_ptr = NULL;
		if (groupsepindex != std::string::npos) {
			groupname = name.substr(0,groupsepindex);
			group_ptr = findGroupPtr(groupname);
			varname = name.substr(groupsepindex+1);
		} else
			varname = name;
		std::vector<int> matches;
		for (std::vector<LogVariable>::iterator i = variables.begin(); i != variables.end(); i++) {
			if (i->group_ptr == group_ptr && i->name == varname)
				return (i-variables.begin()); // unique match
			else if (i->name == varname)
				matches.push_back(i-variables.begin()); // match without groupname
		}
		if (matches.size() == 1)
			return matches[0];
		else if (matches.size() <= 0)
			throw BlackBoxException(std::string("Variable '") + name + "' not found");
		else
			throw BlackBoxException(std::string("Variable '") + name + "' exists in multiple groups (need to specify group)");
		return -1;
	}
	
	const void* BlackBox::findGroupPtr(const std::string& name)
	{
		for (std::map<const void*,std::string>::iterator i = groupnames.begin(); i != groupnames.end(); i++) {
			if (i->second == name)
				return i->first;
		}
		return NULL;
	}
	
} // namespace sbx
