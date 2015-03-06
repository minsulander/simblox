#ifndef DEBUG_H
#define DEBUG_H

#include "Export.h"
#include <iostream>
#include <fstream>
#include <ios>

namespace sbx {

  enum DebugLevels {
	ERROR = -2,
	WARN,
	INFO,
	DEBUG
    };
    
    /// \brief Set debug level for log output. 
    /// \param level Any output above this level will be discarded.
    /// \sa dout()   
    extern SIMBLOX_API void setDebugLevel(int level);

    /// Get current debug level.
    extern SIMBLOX_API int getDebugLevel();
    
    /// Set wether to use ANSI colors for \c ERROR and \c WARN levels
    extern SIMBLOX_API void useLogColors(const bool value);
    
    /// Get wether using ANSI colors for \c ERROR and \c WARN levels
    extern SIMBLOX_API bool usingLogColors();

    /// Return an appropriate output stream for the given debug level.
    extern SIMBLOX_API std::ostream& dout(const int level);

    /// Alternate form for debug log output, taking a string argument \a message instead of providing a string
    extern SIMBLOX_API void dout(const int level, const std::string& message);
    
    /// Send output to a stream (instead of the default cout/cerr combination)
    extern SIMBLOX_API void setLogStream(std::ostream *stream);
    
    /// Send output to a logfile
    extern SIMBLOX_API void setLogFile(const std::string& filename);
    
	/// Get the current log stream
    extern SIMBLOX_API std::ostream* getLogStream();
    
    /// If using log stream or colors, this should be called before exit
    extern SIMBLOX_API void closeLog();
}
#endif
