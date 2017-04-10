#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <string.h>

/**
 * Some OS hacks
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define SLASH '\\'
#else
#define SLASH '/'
#endif

/**
 * Printing
 */
#define __FILENAME__ (strrchr(__FILE__, SLASH) ? strrchr(__FILE__, SLASH) + 1 : __FILE__)

#define ASSERT_PRINT(_stream) \
    cout << __FILENAME__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << _stream << "\n"

/**
 * For use in live code
 */
#define DEBUG_PRINT_ON 0
#if DEBUG_PRINT_ON
#define TRACE(_stream) ASSERT_PRINT(_stream)
#else
#define TRACE(_)
#endif



#endif // UTILS_H_INCLUDED
