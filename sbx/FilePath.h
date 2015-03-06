#ifndef SBX_FILEPATH_H
#define SBX_FILEPATH_H

#include "Export.h"
#include <vector>
#include <string>
#include <iostream>

namespace sbx {
	
	typedef std::vector<std::string> FileList;
	
	class SIMBLOX_API FilePath : public std::vector<std::string>
	{
	public:
		FilePath();
		
		void add(const std::string& path, const std::string& base = "");
		void addFront(const std::string& path, const std::string& base = "");
		void addEnvironmentVariable(const std::string& name, const std::string& subpath = "");
		void addEnvironmentVariableFront(const std::string& name, const std::string& subpath = "");
		std::string find(const std::string& filename);
		
#ifndef WIN32
		FileList getFilesByPattern(const std::string& pattern);
#endif
		FileList getFilesByExtension(const std::string& extension);
		
		void setSeparator(const std::string& sep) { separator = sep; }
		const std::string& getSeparator() const { return separator; }
		
		// Some utility functions
		static std::string dirname(const std::string& path);
		static std::string clean(const std::string& path);
		static bool isAbsolutePath(const std::string& path);
		static FileList getDirectoryContents(const std::string& path);
	protected:
		std::string separator;
	};
	
	inline std::ostream& operator << (std::ostream& output, const FilePath& path)
	{
		for (FilePath::const_iterator i = path.begin(); i != path.end(); i++) {
			if (i != path.begin())
				output << path.getSeparator();
			output << *i;
		}
		return output;     // to enable cascading
	}
	
	
}

#endif
