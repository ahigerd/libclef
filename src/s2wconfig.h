#ifndef S2W_S2WCONFIG_H
#define S2W_S2WCONFIG_H

#define _USE_MATH_DEFINES

#include <cstdlib>
#ifdef _MSC_VER
typedef unsigned int ssize_t;
#endif

#ifdef _WIN32
#define PATH_CHARS "\\/"
#define EXPORT __declspec(dllexport)
#else
#define PATH_CHARS "/"
#define EXPORT __attribute__((visibility("default")))
#endif

#endif
