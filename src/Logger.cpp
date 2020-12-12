#ifndef _LOGGER_H_INCLUDED
#define _LOGGER_H_INCLUDED

#include <iostream>
#include "Logger.h"

namespace smf {

std::ostream devNull(0);

std::basic_ostream<char>* logDebug = &devNull;
std::basic_ostream<char>* logWarning = &devNull;
std::basic_ostream<char>* logError = &devNull;

};

#endif /* _LOGGER_H_INCLUDED */
