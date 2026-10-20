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

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <strings.h>

#include <k2hash/k2hash.h>
#include <k2hash/k2hutil.h>

#include "k2hveccommon.h"
#include "k2hvecdbg.h"

using namespace	std;

//---------------------------------------------------------
// Global Variable
//---------------------------------------------------------
K2hvecDbgMode	k2hvec_debug_mode	= K2hvecDbgMode::VECDBG_SILENT;
FILE*			k2hvec_dbg_fp		= nullptr;

//---------------------------------------------------------
// Class K2hvecDbgControl
//---------------------------------------------------------
class K2hvecDbgControl
{
	protected:
		static const char*	DBGENVNAME;
		static const char*	DBGENVFILE;

		K2hvecDbgMode*		pk2hvec_debug_mode;
		FILE**				pk2hvec_dbg_fp;
		string				k2hvec_dbg_file;

	protected:
		K2hvecDbgControl() : pk2hvec_debug_mode(&k2hvec_debug_mode), pk2hvec_dbg_fp(&k2hvec_dbg_fp), k2hvec_dbg_file("")
		{
			*pk2hvec_debug_mode	= K2hvecDbgMode::VECDBG_SILENT;
			*pk2hvec_dbg_fp		= nullptr;
			DbgCtrlLoadEnv();
		}
		virtual ~K2hvecDbgControl()
		{
		}

		bool DbgCtrlLoadEnvName(void);
		bool DbgCtrlLoadEnvFile(void);

	public:
		static K2hvecDbgControl& GetK2hvecDbgCtrl(void)
		{
			static K2hvecDbgControl	singleton;			// singleton
			return singleton;
		}

		bool DbgCtrlLoadEnv(void);
		K2hvecDbgMode SetDbgCtrlMode(K2hvecDbgMode mode);
		K2hvecDbgMode BumpupDbgCtrlMode(void);
		K2hvecDbgMode GetDbgCtrlMode(void);
		bool SetDbgCtrlFile(const char* filepath);
		bool UnsetDbgCtrlFile(void);
		const char* GetDbgCtrlFile(void);
};

//---------------------------------------------------------
// Class variables
//---------------------------------------------------------
const char*	K2hvecDbgControl::DBGENVNAME = "VECDBGMODE";
const char*	K2hvecDbgControl::DBGENVFILE = "VECDBGFILE";

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
bool K2hvecDbgControl::DbgCtrlLoadEnv(void)
{
	if(!DbgCtrlLoadEnvName() || !DbgCtrlLoadEnvFile()){
		return false;
	}
	return true;
}

bool K2hvecDbgControl::DbgCtrlLoadEnvName(void)
{
	string	value;
	if(!k2h_getenv(K2hvecDbgControl::DBGENVNAME, value)){
		MSG_VECPRN("%s ENV is not set.", K2hvecDbgControl::DBGENVNAME);
		return true;
	}
	if(0 == strcasecmp(value.c_str(), "SLT") || 0 == strcasecmp(value.c_str(), "SILENT")){
		SetDbgCtrlMode(K2hvecDbgMode::VECDBG_SILENT);
	}else if(0 == strcasecmp(value.c_str(), "ERR") || 0 == strcasecmp(value.c_str(), "ERROR")){
		SetDbgCtrlMode(K2hvecDbgMode::VECDBG_ERR);
	}else if(0 == strcasecmp(value.c_str(), "WAN") || 0 == strcasecmp(value.c_str(), "WARNING")){
		SetDbgCtrlMode(K2hvecDbgMode::VECDBG_WARN);
	}else if(0 == strcasecmp(value.c_str(), "MSG") || 0 == strcasecmp(value.c_str(), "INFO")){
		SetDbgCtrlMode(K2hvecDbgMode::VECDBG_MSG);
	}else if(0 == strcasecmp(value.c_str(), "DMP") || 0 == strcasecmp(value.c_str(), "DUMP")){
		SetDbgCtrlMode(K2hvecDbgMode::VECDBG_DUMP);
	}else{
		MSG_VECPRN("%s ENV is not unknown string(%s).", K2hvecDbgControl::DBGENVNAME, value.c_str());
		return false;
	}
	return true;
}

bool K2hvecDbgControl::DbgCtrlLoadEnvFile(void)
{
	string	value;
	if(!k2h_getenv(K2hvecDbgControl::DBGENVFILE, value)){
		MSG_VECPRN("%s ENV is not set.", K2hvecDbgControl::DBGENVFILE);
		return true;
	}
	if(!SetDbgCtrlFile(value.c_str())){
		MSG_VECPRN("%s ENV is unsafe string(%s).", K2hvecDbgControl::DBGENVFILE, value.c_str());
		return false;
	}
	return true;
}

K2hvecDbgMode K2hvecDbgControl::SetDbgCtrlMode(K2hvecDbgMode mode)
{
	K2hvecDbgMode oldmode	= *pk2hvec_debug_mode;
	*pk2hvec_debug_mode		= mode;
	return oldmode;
}

K2hvecDbgMode K2hvecDbgControl::BumpupDbgCtrlMode(void)
{
	K2hvecDbgMode	mode = GetDbgCtrlMode();

	if(K2hvecDbgMode::VECDBG_SILENT == mode){
		mode = K2hvecDbgMode::VECDBG_ERR;
	}else if(K2hvecDbgMode::VECDBG_ERR == mode){
		mode = K2hvecDbgMode::VECDBG_WARN;
	}else if(K2hvecDbgMode::VECDBG_WARN == mode){
		mode = K2hvecDbgMode::VECDBG_MSG;
	}else if(K2hvecDbgMode::VECDBG_MSG == mode){
		mode = K2hvecDbgMode::VECDBG_DUMP;
	}else{	// K2hvecDbgMode::VECDBG_DUMP == mode
		mode = K2hvecDbgMode::VECDBG_SILENT;
	}
	return SetDbgCtrlMode(mode);
}

K2hvecDbgMode K2hvecDbgControl::GetDbgCtrlMode(void)
{
	return *pk2hvec_debug_mode;
}

bool K2hvecDbgControl::SetDbgCtrlFile(const char* filepath)
{
	if(VECEMPTYSTR(filepath)){
		ERR_VECPRN("Parameter is wrong.");
		return false;
	}
	if(!UnsetDbgCtrlFile()){
		return false;
	}
	FILE*	newfp;
	if(nullptr == (newfp = fopen(filepath, "a+"))){
		ERR_VECPRN("Could not open debug file(%s). errno = %d", filepath, errno);
		// cppcheck-suppress resourceLeak
		return false;
	}
	*pk2hvec_dbg_fp	= newfp;
	k2hvec_dbg_file	= filepath;
	return true;
}

bool K2hvecDbgControl::UnsetDbgCtrlFile(void)
{
	k2hvec_dbg_file.clear();

	if(*pk2hvec_dbg_fp){
		if(0 != fclose(*pk2hvec_dbg_fp)){
			ERR_VECPRN("Could not close debug file. errno = %d", errno);
			*pk2hvec_dbg_fp = nullptr;		// On this case, k2hvec_dbg_fp is not correct pointer after error...
			return false;
		}
		*pk2hvec_dbg_fp = nullptr;
	}
	return true;
}

const char* K2hvecDbgControl::GetDbgCtrlFile(void)
{
	if(*pk2hvec_dbg_fp && !k2hvec_dbg_file.empty()){
		return k2hvec_dbg_file.c_str();
	}
	return nullptr;
}

//---------------------------------------------------------
// Global Function
//---------------------------------------------------------
K2hvecDbgMode SetK2hvecDbgMode(K2hvecDbgMode mode)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().SetDbgCtrlMode(mode);
}

K2hvecDbgMode SetK2hvecDbgModeWithK2hash(K2hvecDbgMode mode)
{
	// Set k2hash debug level
	if(K2hvecDbgMode::VECDBG_SILENT == mode){
		k2h_set_debug_level_silent();
	}else if(K2hvecDbgMode::VECDBG_ERR == mode){
		k2h_set_debug_level_error();
	}else if(K2hvecDbgMode::VECDBG_WARN == mode){
		k2h_set_debug_level_warning();
	}else if(K2hvecDbgMode::VECDBG_MSG == mode || K2hvecDbgMode::VECDBG_DUMP == mode){
		k2h_set_debug_level_message();
	}

	return SetK2hvecDbgMode(mode);
}

K2hvecDbgMode BumpupK2hvecDbgMode(void)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().BumpupDbgCtrlMode();
}

K2hvecDbgMode BumpupK2hvecDbgModeWithK2hash(void)
{
	k2h_bump_debug_level();							// Bumpup k2hash debug level
	return BumpupK2hvecDbgMode();
}

K2hvecDbgMode GetK2hvecDbgMode(void)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().GetDbgCtrlMode();
}

const char* GetK2hvecDbgModeString(void)
{
	return K2hvecDbgMode_FULLSTR(GetK2hvecDbgMode());
}

bool LoadK2hvecDbgEnv(void)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().DbgCtrlLoadEnv();
}

bool LoadK2hvecDbgEnvWithK2hash(void)
{
	bool result = k2h_load_debug_env();				// Load environment for k2hash
	if(!LoadK2hvecDbgEnv()){
		return false;
	}
	return result;
}

bool SetK2hvecDbgFile(const char* filepath)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().SetDbgCtrlFile(filepath);
}

bool SetK2hvecDbgFileWithK2hash(const char* filepath)
{
	bool result = k2h_set_debug_file(filepath);		// Setup k2hash debug file
	if(!SetK2hvecDbgFile(filepath)){
		return false;
	}
	return result;
}

bool UnsetK2hvecDbgFile(void)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().UnsetDbgCtrlFile();
}

bool UnsetK2hvecDbgFileWithK2hash(void)
{
	bool result = k2h_unset_debug_file();			// Unset k2hash debug file
	if(!UnsetK2hvecDbgFile()){
		return false;
	}
	return result;
}

const char* GetK2hvecDbgFile(void)
{
	return K2hvecDbgControl::GetK2hvecDbgCtrl().GetDbgCtrlFile();
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
