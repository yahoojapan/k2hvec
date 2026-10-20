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

#ifndef	K2HVECCOMMON_H
#define	K2HVECCOMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//---------------------------------------------------------
// Macros for compiler
//---------------------------------------------------------
#if defined(__cplusplus)
#define	DECL_EXTERN_C_START			extern "C" {
#define	DECL_EXTERN_C_END			}
#else	// __cplusplus
#define	DECL_EXTERN_C_START
#define	DECL_EXTERN_C_END
#endif	// __cplusplus

//---------------------------------------------------------
// Templates & macros
//---------------------------------------------------------
#if defined(__cplusplus)
	template<typename T> inline bool VECEMPTYSTR(const T pstr)
	{
		return (nullptr == (pstr) || '\0' == *(pstr)) ? true : false;
	}
#else	// __cplusplus
	#define	VECEMPTYSTR(pstr)		(nullptr == (pstr) || '\0' == *(pstr))
#endif	// __cplusplus


//---------------------------------------------------------
// For endian
//---------------------------------------------------------
#ifndef	_GNU_SOURCE
	#define _GNU_SOURCE				1
	#define	SET_LOCAL_GNU_SOURCE	1
#endif

#ifdef	HAVE_ENDIAN_H
	#include <endian.h>
#else
	#ifdef	HAVE_SYS_ENDIAN_H
		#include <sys/endian.h>
	#endif
#endif

#ifdef	SET_LOCAL_GNU_SOURCE
	#undef _GNU_SOURCE
#endif

//---------------------------------------------------------
// Include C types
//---------------------------------------------------------
#define	__STDC_FORMAT_MACROS
#include <inttypes.h>

#endif	// K2HVECCOMMON_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
