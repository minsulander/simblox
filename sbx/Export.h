#ifndef EXPORT_H
#define EXPORT_H

#if defined(WIN32) || defined(__CYGWIN__)
#  ifdef SIMBLOX_LIBRARY
#    define SIMBLOX_API __declspec(dllexport)
#  else
#    define SIMBLOX_API __declspec(dllimport)
#  endif
#else
#  define SIMBLOX_API
#endif

#if defined(WIN32) || defined(__CYGWIN__)
#  ifdef SIMBLOX_PLUGIN
#    define SIMBLOX_PLUGIN_API __declspec(dllexport)
#  else
#    define SIMBLOX_PLUGIN_API __declspec(dllimport)
#  endif
#else
#  define SIMBLOX_PLUGIN_API
#endif

#endif
