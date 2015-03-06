#ifndef UNITS_H_
#define UNITS_H_

#include <exception>
#include <string>
#include <sbx/Export.h>

namespace units
{
	
	class SIMBLOX_API UnitsException : public std::exception
	{
		public:
			UnitsException(const std::string& message) { this->message = std::string("UnitsException: ") + message; }
		~UnitsException() throw() {}
		virtual const char* what() const throw()
		{
			return message.c_str();
		}
		std::string message;
	};

	
	void SIMBLOX_API initialize(const std::string& userfile);
	double SIMBLOX_API convert(const double value, const std::string& havestr, const std::string& wantstr);
}

#endif /*UNITS_H_*/
