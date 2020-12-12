#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

#include <iostream>

namespace smf {

extern std::ostream devNull;

extern std::basic_ostream<char>* logDebug;
extern std::basic_ostream<char>* logWarning;
extern std::basic_ostream<char>* logError;

};

#endif /* _LOGGER_H_INCLUDED */
