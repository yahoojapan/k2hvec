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

#ifndef	K2HVECDBG_H
#define K2HVECDBG_H

#include <strings.h>

#include "k2hveccommon.h"

//---------------------------------------------------------
// Debug Mode
//---------------------------------------------------------
enum class K2hvecDbgMode : int {
	VECDBG_SILENT	= 0,
	VECDBG_ERR		= 1,
	VECDBG_WARN		= 3,
	VECDBG_MSG		= 7,
	VECDBG_DUMP		= 15
};

//
// Override for "&" operation
//
inline constexpr K2hvecDbgMode operator&(K2hvecDbgMode lhs, K2hvecDbgMode rhs)
{
	using T = std::underlying_type_t<K2hvecDbgMode>;
	return static_cast<K2hvecDbgMode>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

//
// Extern "C" Area
//
DECL_EXTERN_C_START

extern K2hvecDbgMode	k2hvec_debug_mode;		// Do not use directly this variable.
extern FILE*			k2hvec_dbg_fp;

K2hvecDbgMode SetK2hvecDbgMode(K2hvecDbgMode mode);
K2hvecDbgMode SetK2hvecDbgModeWithK2hash(K2hvecDbgMode mode);
K2hvecDbgMode BumpupK2hvecDbgMode(void);
K2hvecDbgMode BumpupK2hvecDbgModeWithK2hash(void);
K2hvecDbgMode GetK2hvecDbgMode(void);
const char* GetK2hvecDbgModeString(void);
bool LoadK2hvecDbgEnv(void);
bool LoadK2hvecDbgEnvWithK2hash(void);
bool SetK2hvecDbgFile(const char* filepath);
bool SetK2hvecDbgFileWithK2hash(const char* filepath);
bool UnsetK2hvecDbgFile(void);
bool UnsetK2hvecDbgFileWithK2hash(void);
const char* GetK2hvecDbgFile(void);

DECL_EXTERN_C_END

//
// Check debug mode
//
inline bool IsVecDbgEnabled(K2hvecDbgMode mode) noexcept
{
	return (k2hvec_debug_mode & mode) == mode;
}

//---------------------------------------------------------
// Utility : The mode to string
//---------------------------------------------------------
inline constexpr const char* K2hvecDbgMode_STR(K2hvecDbgMode mode) noexcept
{
	switch(mode){
		case K2hvecDbgMode::VECDBG_SILENT:	return "SLT";
		case K2hvecDbgMode::VECDBG_ERR:		return "ERR";
		case K2hvecDbgMode::VECDBG_WARN:	return "WAN";
		case K2hvecDbgMode::VECDBG_MSG:		return "MSG";
		case K2hvecDbgMode::VECDBG_DUMP:	return "DMP";
		default:							return "";
	}
}

inline constexpr const char* K2hvecDbgMode_FULLSTR(K2hvecDbgMode mode) noexcept
{
	switch(mode){
		case K2hvecDbgMode::VECDBG_SILENT:	return "silent";
		case K2hvecDbgMode::VECDBG_ERR:		return "error";
		case K2hvecDbgMode::VECDBG_WARN:	return "warning";
		case K2hvecDbgMode::VECDBG_MSG:		return "message";
		case K2hvecDbgMode::VECDBG_DUMP:	return "dump";
		default:							return "";
	}
}

inline bool IsDbgModeString(const std::string& str, K2hvecDbgMode mode)
{
	switch(mode){
		case K2hvecDbgMode::VECDBG_SILENT:	return (0 == strcasecmp(str.c_str(), "silent")	|| 0 == strcasecmp(str.c_str(), "slt"));
		case K2hvecDbgMode::VECDBG_ERR:		return (0 == strcasecmp(str.c_str(), "error")	|| 0 == strcasecmp(str.c_str(), "err"));
		case K2hvecDbgMode::VECDBG_WARN:	return (0 == strcasecmp(str.c_str(), "warning")	|| 0 == strcasecmp(str.c_str(), "warn")	|| 0 == strcasecmp(str.c_str(), "wan"));
		case K2hvecDbgMode::VECDBG_MSG:		return (0 == strcasecmp(str.c_str(), "message")	|| 0 == strcasecmp(str.c_str(), "msg")	|| 0 == strcasecmp(str.c_str(), "information") || 0 == strcasecmp(str.c_str(), "info"));
		case K2hvecDbgMode::VECDBG_DUMP:	return (0 == strcasecmp(str.c_str(), "dump")	|| 0 == strcasecmp(str.c_str(), "dmp"));
		default:							return false;
	}
}

//---------------------------------------------------------
// Utility : Templates / Macros for printing
//---------------------------------------------------------
template <typename... Args>
inline void LowK2hvecPrintImpl(K2hvecDbgMode mode, const char* func, int line, const char* fmt, Args&&... args)
{
	FILE*	fp = k2hvec_dbg_fp ? k2hvec_dbg_fp : stderr;

	std::fprintf(fp, "[VEC-%s] %s(%d) : ", K2hvecDbgMode_STR(mode), func, line);
	// cppcheck-suppress wrongPrintfScanfArgNum
	std::fprintf(fp, fmt, std::forward<Args>(args)...);
	std::fputc('\n', fp);
	std::fflush(fp);
}

template <typename... Args>
inline void RawK2hvecPrintImpl(const char* fmt, Args&&... args)
{
	FILE*	fp = k2hvec_dbg_fp ? k2hvec_dbg_fp : stderr;

	std::fprintf(fp, fmt, std::forward<Args>(args)...);
	std::fputc('\n', fp);
	std::fflush(fp);
}

//
// Macros
//
#define LOW_K2HVECPRINT(mode, fmt, ...) \
	LowK2hvecPrintImpl(mode, __func__, __LINE__, fmt, ##__VA_ARGS__)

#define K2HVECPRINT(mode, fmt, ...) \
	do{ \
		if(IsVecDbgEnabled(mode)){ \
			LOW_K2HVECPRINT(mode, fmt, ##__VA_ARGS__); \
		} \
	}while(0)

#define RAW_K2HVECPRINT(mode, fmt, ...) \
	do{ \
		if(IsVecDbgEnabled(mode)){ \
			RawK2hvecPrintImpl(fmt, ##__VA_ARGS__); \
		} \
	}while(0)

inline bool IS_VECDBG_SILENT() noexcept		{ return IsVecDbgEnabled(K2hvecDbgMode::VECDBG_SILENT);	}
inline bool IS_VECDBG_ERROR() noexcept		{ return IsVecDbgEnabled(K2hvecDbgMode::VECDBG_ERR);	}
inline bool IS_VECDBG_WARNING() noexcept	{ return IsVecDbgEnabled(K2hvecDbgMode::VECDBG_WARN);	}
inline bool IS_VECDBG_MESSAGE() noexcept	{ return IsVecDbgEnabled(K2hvecDbgMode::VECDBG_MSG);	}
inline bool IS_VECDBG_DUMP() noexcept		{ return IsVecDbgEnabled(K2hvecDbgMode::VECDBG_DUMP);	}

#define	SLT_VECPRN(...)		K2HVECPRINT(K2hvecDbgMode::VECDBG_SILENT,	##__VA_ARGS__, "")	// This means nothing...
#define	ERR_VECPRN(...)		K2HVECPRINT(K2hvecDbgMode::VECDBG_ERR,		##__VA_ARGS__, "")
#define	WAN_VECPRN(...)		K2HVECPRINT(K2hvecDbgMode::VECDBG_WARN,		##__VA_ARGS__, "")
#define	MSG_VECPRN(...)		K2HVECPRINT(K2hvecDbgMode::VECDBG_MSG,		##__VA_ARGS__, "")
#define	DMP_VECPRN(...)		K2HVECPRINT(K2hvecDbgMode::VECDBG_DUMP,		##__VA_ARGS__, "")
#define	RAW_VECPRN(...)		RAW_K2HVECPRINT(K2hvecDbgMode::VECDBG_DUMP,	##__VA_ARGS__, "")

#endif	// K2HVECDBG_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
