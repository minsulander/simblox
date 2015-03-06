#include "FilePath.h"

#include <fstream>
#include <stdlib.h>

#ifdef WIN32
#	include <io.h>
#	include <direct.h>
#	include <windows.h>
	const char* DIR_SEPARATOR = "/";
	const char* PATH_SEPARATOR = ";";
#else
#	include <glob.h>
#	include <dirent.h>
	const char* DIR_SEPARATOR = "/";
	const char* PATH_SEPARATOR = ":";
#endif

namespace sbx {
	
	FilePath::FilePath()
	{
		separator = PATH_SEPARATOR;
	}
	
	void FilePath::add(const std::string& thepath, const std::string& base)
	{
		if (thepath.length() == 0)
			return;
		std::string path = thepath;
		for (std::string::size_type i = path.find(separator,0); path.length() > 0; i = path.find(separator,0)) {
			if (i == std::string::npos)
				i = path.length();
			std::string subpath = path.substr(0,i);
			if (subpath.length() > 0) {
				if (base.length() != 0 && !isAbsolutePath(subpath))
					push_back(clean(base + DIR_SEPARATOR + subpath));
				else
					push_back(clean(subpath));
			}
			path.erase(0,i+1);
		}
	}
	
	void FilePath::addFront(const std::string& thepath, const std::string& base)
	{
		if (thepath.length() == 0)
			return;
		std::string path = thepath;
		for (std::string::size_type i = path.find(separator,0); path.length() > 0; i = path.find(separator,0)) {
			if (i == std::string::npos)
				i = path.length();
			std::string subpath = path.substr(0,i);
			if (subpath.length() > 0) {
				if (base.length() != 0 && !isAbsolutePath(subpath))
					insert(begin(), clean(base + DIR_SEPARATOR + subpath));
				else
					insert(begin(), clean(subpath));
			}
			path.erase(0,i+1);
		}
	}
	
	void FilePath::addEnvironmentVariable(const std::string& name, const std::string& subpath)
	{
		if (getenv(name.c_str()))
			add(std::string(getenv(name.c_str())) + subpath);
	}
	
	void FilePath::addEnvironmentVariableFront(const std::string& name, const std::string& subpath)
	{
		if (getenv(name.c_str()))
			addFront(std::string(getenv(name.c_str())) + subpath);
	}
	
	std::string FilePath::find(const std::string& filename)
	{
		if (isAbsolutePath(filename))
			return filename;
		for (iterator i = begin(); i != end(); i++) {
			std::string fullname = *i + DIR_SEPARATOR + filename;
			std::ifstream ifs(fullname.c_str());
			if (ifs.good())
				return fullname;
		}
		return "";
	}
	
#ifndef WIN32
	FileList FilePath::getFilesByPattern(const std::string& pattern)
	{
		/// \todo Win32 implementation
		glob_t g;
		g.gl_pathc = 0;
		for (iterator i = begin(); i != end(); i++) {
			glob((*i + DIR_SEPARATOR + pattern).c_str(), (i == begin() ? 0 : GLOB_APPEND), NULL, &g);
		}
		FileList list;
		for (int i = 0; i < g.gl_pathc; i++) {
			list.push_back(std::string(g.gl_pathv[i]));
		}
		//globfree(&g);
		return list;
	}
#endif
	
	FileList FilePath::getFilesByExtension(const std::string& extension)
	{
		FileList list;
		for (iterator i = begin(); i != end(); i++) {
			FileList contents = getDirectoryContents(*i);
			for (FileList::iterator j = contents.begin(); j != contents.end(); j++) {
				if (j->length() >= extension.length() && j->substr(j->length()-extension.length()) == extension)
					list.push_back(*i + DIR_SEPARATOR + *j);
			}
		}
		return list;
	}
	
	std::string FilePath::dirname(const std::string& path)
	{
		if (path.length() == 0)
			return ".";
		std::string::size_type index = path.rfind(DIR_SEPARATOR, path.length()-1);
		if (index == std::string::npos)
			return ".";
		std::string dir;
		dir = path.substr(0,index);
		if (dir.length() == 0)
			return "/";
		return dir;
	}
	
	std::string FilePath::clean(const std::string& path)
	{
		if (path.length() == 0)
			return "";
		std::string newpath = path;
		// Remove duplicate separators
		do {
			std::string::size_type i = newpath.find(std::string(DIR_SEPARATOR) + std::string(DIR_SEPARATOR), 0);
			if (i == std::string::npos)
				break;
			newpath = newpath.substr(0,i) + newpath.substr(i+1);
		} while (true);
		// Remove "up-paths" e.g. a/b/../c -> a/c
		std::string::size_type i = 0;
		do {
			i = newpath.find(std::string(DIR_SEPARATOR) + ".." + std::string(DIR_SEPARATOR), i);
			if (i == std::string::npos)
				break;
			std::string::size_type j = newpath.rfind(DIR_SEPARATOR,i-1);
			if (j == std::string::npos || newpath.substr(j+1,i-j-1) == "..") {
				i += 4;
				continue;
			}
			newpath = newpath.substr(0,j+1) + newpath.substr(i+4);
			i = 0;
		} while (true);
		if (newpath.substr(0,2) == "./")
			newpath = newpath.substr(2);
		return newpath;
	}
	
	bool FilePath::isAbsolutePath(const std::string& path)
	{
		if (path.length() == 0)
			return false;
		// for example /usr/bin
		if (path.substr(0,1) == DIR_SEPARATOR)
			return true;
		// for example c:\program files\stuff
		if (path.length() > 1 && path[1] == ':' && path.substr(2,1) == DIR_SEPARATOR) 
			return true;
		return false;
	}
	
#if defined(WIN32) && !defined(__CYGWIN__)
    FileList FilePath::getDirectoryContents(const std::string& dirName)
    {
        FileList contents;
        WIN32_FIND_DATA data;
        HANDLE handle = FindFirstFile((dirName + "\\*").c_str(), &data);
        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                contents.push_back(data.cFileName);
            }
            while (FindNextFile(handle, &data) != 0);
			
            FindClose(handle);
        }
        return contents;
    }
#else
    FileList FilePath::getDirectoryContents(const std::string& dirName)
    {
        FileList contents;
        DIR *handle = opendir(dirName.c_str());
        if (handle)
        {
            dirent *rc;
            while((rc = readdir(handle))!=NULL)
            {
                contents.push_back(rc->d_name);
            }
            closedir(handle);
        }
        return contents;
    }
#endif // unix getDirectoryContexts
	
} // namespace sbx
