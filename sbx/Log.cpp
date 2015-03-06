
#include "Log.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>

namespace sbx {

static int debug_level = 0;
static bool initialized = false;
static bool use_colors = false;
static std::ostream *logstream = NULL;

#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
static const char* snull = "nul";
#else
static const char* snull = "/dev/null";
#endif

void setDebugLevel(int level) {
    debug_level = level;
    initialized = true;
}

int getDebugLevel() {
  return debug_level;
}

void useLogColors(const bool value) {
	use_colors = value;
}

bool usingLogColors() {
	return use_colors;
}

/**
This function is the heart of the logging functionality. Debug levels above the \c INFO level
can be used freely, although levels above, say 10, seems rather meaningless.
\param level The debug level of the message to be passed on to the stream.
\sa setDebugLevel(), DebugLevels
\return \c std::cerr for \c ERROR and \c WARN levels, \c std::cout for levels below the currently set debuglevel,
\c /dev/null (or \c NUL on Win32) for levels above the current debuglevel (discards output).

As an all-in-one example, consider:
\code
setDebugLevel(1); // level DEBUG == 1
dout(ERROR) << "The upper starboard magic flux generator has somehow disconnected" << std::endl;
dout(WARN)  << "Cross-dimensional gate has spontaneously opened itself" << std::endl;
dout(3)     << "  gate angle is " << gate->getAngle() << "\n";
dout(INFO)  << "Enabling creature passthrough countermeasures\n"; // level INFO == 0
dout(DEBUG) << "  animated gatekeeper curse key=" << std::hex << gatekeeper.key << "\n";
\endcode
The above code would generate the following output:
\verbatim
Error: The upper starboard magic flux generator has somehow disconnected
Warning: Cross-dimensional gate has spontaneously opened itself
Enabling creature passthrough countermeasures
  animated gatekeeper curse key=0x1f7a
\endverbatim

Note that the message at debug level 3 was skipped. Also, the \c ERROR and \c WARN messages
had \c "Error:" and \c "Warning:" prepended to them.  The first two lines
(\c ERROR and \c WARN levels) are written to \c std::cerr, the rest to \c std::cout.

Debug level can be set by the application using the setDebugLevel() function, but it can also
be controlled through the \c SIMBLOX_DEBUG_LEVEL (checked upon the first call to dout()).

Default debug level is 0.
*/
std::ostream& dout(const int level) {
    // Initialize if needed
    static std::ofstream nullout(snull);
    if (!initialized) {
		char *envs = getenv("SIMBLOX_DEBUG_LEVEL");
		if (envs)
			debug_level = atoi(envs);
		if (getenv("SIMBLOX_LOG_COLORS"))
			use_colors = true;
		initialized = true;
    }
    static int last_level = 0;
    // Return either stderr, stdout or /dev/null as appropriate
    if (debug_level < level)
    	return nullout;
    else if (level < 0) {
    	if (logstream) {
			if (level != last_level && level == WARN)
				*logstream << "Warning: ";
			else if (level != last_level && level == ERROR)
				*logstream << "Error: ";
			last_level = level;
			return *logstream;

    	} else {
#ifdef WIN32
			// Use only stdout on Windows
			if (level != last_level && level == WARN)
				std::cout << "Warning: ";
			else if (level != last_level && level == ERROR)
				std::cout << "Error: ";
			last_level = level;
			return std::cout;
#else
			if (level != last_level && level == WARN) {
				if (use_colors)
					std::cerr << (unsigned char)27 << "[33m";
				std::cerr << "Warning: ";
			} else if (level != last_level && level == ERROR) {
				if (use_colors)
					std::cerr << (unsigned char)27 << "[31m";
				std::cerr << "Error: ";
			}
			last_level = level;
			return std::cerr;
#endif
		}
	}
	if (level != last_level) {
		if (use_colors && (last_level == ERROR || last_level == WARN) && !logstream)
			std::cerr << (unsigned char)27 << "[0m";
	}
	last_level = level;
	if (logstream)
		return *logstream;
	else
		return std::cout;
}

void dout(const int level, const std::string& message) {
    dout(level) << message;
}

void setLogStream(std::ostream *stream) {
	logstream = stream;
	use_colors = false;
}

void setLogFile(const std::string& filename) {
	logstream = new std::ofstream(filename.c_str());
}

std::ostream* getLogStream() {
	return logstream;
}

void closeLog() {
	if (use_colors &&  !logstream) {
		std::cerr << (unsigned char)27 << "[0m";
		std::cout << (unsigned char)27 << "[0m";
	}
	//if (logstream)
	//	logstream->close();
}

} // namespace sbx
