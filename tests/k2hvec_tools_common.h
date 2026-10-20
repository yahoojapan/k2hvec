/*
 * K2HVEC
 *
 * Copyright 2026 LY Corporation.
 *
 * K2HVEC is a high-performance vector database for fast vector search,
 * built on top of the K2HASH key-value store.
 *
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Tue, Jul 21 2026
 * REVISION:
 *
 */

#ifndef	K2HVEC_TOOLS_COMMON_H
#define	K2HVEC_TOOLS_COMMON_H

#include <cstdio>
#include <cstdarg>
#include <string>
#include <libgen.h>
#include <unistd.h>

//---------------------------------------------------------
// Common Utility Functions
//---------------------------------------------------------
inline void PRN(const char* format, ...)
{
	if(format){
		va_list ap;
		va_start(ap, format);
		vfprintf(stdout, format, ap); 
		va_end(ap);
	}
	fputc('\n', stdout);
	fflush(stdout);
}

inline void PRNSUCCESS(const char* format, ...)
{
	static const char* CGRN = ::isatty(STDOUT_FILENO) ? "\033[32m" : "";	// Color Green
	static const char* CDEF = ::isatty(STDOUT_FILENO) ? "\033[0m"  : "";	// To default

	fprintf(stderr, "%s[SUCCEED]%s ", CGRN, CDEF);
	if(format){
		va_list ap;
		va_start(ap, format);
		vfprintf(stdout, format, ap); 
		va_end(ap);
	}
	fputc('\n', stdout);
	fflush(stdout);
}

inline void ERR(const char* format, ...)
{
	static const char* CRED = ::isatty(STDOUT_FILENO) ? "\033[31m" : "";	// Color Red
	static const char* CDEF = ::isatty(STDOUT_FILENO) ? "\033[0m"  : "";	// To default

	fprintf(stderr, "%s[ERR]%s ", CRED, CDEF);
	if(format){
		va_list ap;
		va_start(ap, format);
		vfprintf(stderr, format, ap); 
		va_end(ap);
	}
	fputc('\n', stderr);
	fflush(stdout);
}

inline std::string programname(const char* prgpath)
{
	if(!prgpath){
		return "";
	}
	std::string	path_copy	= prgpath;
	std::string	prgname		= ::basename(&path_copy[0]);

	if(0 == prgname.compare(0, 3, "lt-")){
		prgname.erase(0, 3);
	}
	return prgname;
}

#endif	// K2HVEC_TOOLS_COMMON_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
