/* cppcheck-suppress-file duplInheritedMember
 *
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

#include <fstream>
#include <cstring>
#include <cerrno>

#include "k2hvec.h"
#include "k2hvecutil.h"
#include "k2hvecmath.h"
#include "k2hvecvalue.h"
#include "k2hvec_tools_common.h"
#include "k2hvec_cli_cmd.h"
#include "k2hvec_cli_promptcmd.h"

//---------------------------------------------------------
// PromptCommandBase Class variables
//---------------------------------------------------------
//
// ConsoleInput object pointer
//
// [NOTE]
// Operations on the K2hvecCommand and ConsoleInput class are
// performed directly from several derived classes, Therefore,
// a pointer to the ConsoleInput object is maintained as a
// member of this base class. 
// To ensure safe operation, a subclass named ConsoleInputScope
// is being implemented to utilize the RAII(Resource Acquisition
// Is Initialization) pattern.
//
// [NOTE]
// About PrintHelp class method
//
// The class method(static void PrintHelp()) in PromptCommandBase shares the
// same name with methods in its derived classes.
// While this triggers a cppcheck 'duplInheritedMember' warning, we deliberately
// choose not to:
//	1) Change it to a virtual method(instantiation is unnecessary).
//	2) Rename the method(due to the large number of derived classes).
//
// Instead, 'cppcheck-suppress-file duplInheritedMember' is applied to top of
// this file.
//

//
// Variables
//
const int		PromptCommandBase::DEFAULT_BITSIZE;
const int		PromptCommandBase::DEFAULT_DIMENSION;
const char*		PromptCommandBase::OPT_DELIMITERS		= " \t\n\r\f\v";
const char*		PromptCommandBase::DEFAULT_CUSTOM_NAME	= "custom";
const char*		PromptCommandBase::DEFAULT_VERSION		= "1.0";
const char*		PromptCommandBase::EMBEDDING_ARRAY_NAME	= "EMBEDDING";

K2hvecCommand*	PromptCommandBase::pK2hvecCmdObj		= nullptr;
ConsoleInput*	PromptCommandBase::pConsoleObj			= nullptr;
bool			PromptCommandBase::useLapTime			= false;

//
// Command options for prompt command
//
const std::vector<OptionDefinition> PromptCommandBase::PromptOptionList = {
	// control commands
	{CommandId::CMDID_PROMPT_HELP,			{"help",						"h"},										0,	1,		true},
	{CommandId::CMDID_PROMPT_QUIT,			{"quit",						"exit",			"q"},						0,	0,		false},
	{CommandId::CMDID_PROMPT_SHELL,			{"shell",						"sh",			"!"},						0,	0,		false},
	{CommandId::CMDID_PROMPT_ECHO,			{"echo",						"print"},									1,	999,	true},
	{CommandId::CMDID_PROMPT_SLEEP,			{"sleep",						"wait"},									1,	1,		false},
	{CommandId::CMDID_PROMPT_RUN,			{"run"},																	1,	1,		false},
	{CommandId::CMDID_PROMPT_BREAK,			{"break",						"brk"},										0,	0,		true},
	{CommandId::CMDID_PROMPT_NOBREAK,		{"nobreak",						"nbrk"},									0,	0,		true},

	// history
	{CommandId::CMDID_PROMPT_HISTORYMAX,	{"history_max",					"hismax",		"hmax"},					1,	1,		false},
	{CommandId::CMDID_PROMPT_HISTORY,		{"history",						"history_log",	"his"},						0,	0,		false},
	{CommandId::CMDID_PROMPT_SAVEHIS,		{"save_history",				"save_his",		"save"},					1,	1,		false},

	// setting/infomation
	{CommandId::CMDID_PROMPT_LIBVER,		{"library_version",				"libversion",	"libver"},					0,	0,		false},
	{CommandId::CMDID_PROMPT_LAPTIME,		{"lap_time",					"lap"},										0,	2,		false},
	{CommandId::CMDID_PROMPT_INFO,			{"information",					"info",			"i",			"status"},	1,	1,		true},
	{CommandId::CMDID_PROMPT_DUMP,			{"dump",						"dmp"},										1,	2,		true},
	{CommandId::CMDID_PROMPT_DBG,			{"debug_level",					"debug",		"d"},						0,	2,		false},
	{CommandId::CMDID_PROMPT_DBGFILE,		{"debug_logfile",				"debug_log",	"dlog"},					0,	2,		false},
	{CommandId::CMDID_PROMPT_LASTSTATUS,	{"last_status",					"laststatus",	"$?"},						0,	1,		true},
	{CommandId::CMDID_PROMPT_COMATTR,		{"common_attribute",			"common_attr",	"com_attr",		"attr"},	0,	2,		true},
	{CommandId::CMDID_PROMPT_COMMON_CONF,	{"common_config",				"common_conf",	"comconf",		"cc"},		0,	3,		true},

	// process for key
	{CommandId::CMDID_PROMPT_CREATEK2HKEY,	{"create_k2hash_key",			"k2h_key",		"k2hkey",		"key"},		0,	1,		true},
	{CommandId::CMDID_PROMPT_SET,			{"set",							"s",			"push",			"put"},		0,	4,		true},
	{CommandId::CMDID_PROMPT_GET,			{"get",							"g",			"print",		"p"},		0,	4,		true},
	{CommandId::CMDID_PROMPT_REMOVE,		{"remove",						"rm",			"delete",		"del"},		0,	2,		true},

	// find/search
	{CommandId::CMDID_PROMPT_READSUMMARY,	{"read_summary",				"read",			"r"},						1,	3,		true},
	{CommandId::CMDID_PROMPT_SEARCH,		{"search",						"find",			"f"},						0,	5,		true},

	// load data
	{CommandId::CMDID_PROMPT_LOADPM,		{"load_projection_matrix",		"pm"},										1,	1,		true},
	{CommandId::CMDID_PROMPT_LOADEM,		{"load_embedding",				"em"},										1,	1,		true},

	// create data
	{CommandId::CMDID_PROMPT_CREATEPM,		{"create_projection_matrix",	"create_pm",	"cpm"},						0,	5,		true},
	{CommandId::CMDID_PROMPT_CREATEEM,		{"create_embedding",			"create_em",	"cem"},						0,	2,		true},

	// calcuration
	{CommandId::CMDID_PROMPT_CALBH,			{"calculate_binary_hash",		"calculate_bh",	"cal_bh",		"cbh"},		0,	2,		true},
	{CommandId::CMDID_PROMPT_CALCOS,		{"calculate_cosin_similarity",	"calculate_cos","cal_cos",		"cos"},		1,	5,		true}
};

//---------------------------------------------------------
// PromptCommandBase Classes Methods : Parsers
//---------------------------------------------------------
bool PromptCommandBase::IsProjectionMatrixParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "ProjectionMmatrix") || 0 == strcasecmp(parameter.c_str(), "Projection") || 0 == strcasecmp(parameter.c_str(), "Matrix") || 0 == strcasecmp(parameter.c_str(), "PM")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsEmbeddingDataParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "EmbeddingData") || 0 == strcasecmp(parameter.c_str(), "Embedding") || 0 == strcasecmp(parameter.c_str(), "EM")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsSimpleParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "simple") || 0 == strcasecmp(parameter.c_str(), "spl")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsWithK2hashParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "with_k2hash") || 0 == strcasecmp(parameter.c_str(), "k2hash") || 0 == strcasecmp(parameter.c_str(), "wk2h") || 0 == strcasecmp(parameter.c_str(), "k2h")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsLongParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "long") || 0 == strcasecmp(parameter.c_str(), "l")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsOnParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "ON") || 0 == strcasecmp(parameter.c_str(), "YES") || 0 == strcasecmp(parameter.c_str(), "Y") || 0 == strcasecmp(parameter.c_str(), "1")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsOffParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "OFF") || 0 == strcasecmp(parameter.c_str(), "NO") || 0 == strcasecmp(parameter.c_str(), "N") || 0 == strcasecmp(parameter.c_str(), "0")){
		return true;
	}
	return false;
}

bool PromptCommandBase::IsMessageParameter(const std::string& parameter)
{
	if(0 == strcasecmp(parameter.c_str(), "message") || 0 == strcasecmp(parameter.c_str(), "msg") || 0 == strcasecmp(parameter.c_str(), "m")){
		return true;
	}
	return false;
}

bool PromptCommandBase::GetParameter(const std::string& parameter, const std::vector<std::string>& names, std::string& value)
{
	std::size_t	eq_pos = parameter.find('=');
	if(std::string::npos == eq_pos){
		return false;
	}
	std::size_t	name_len = eq_pos;

	for(const auto& name: names){
		if(name.length() != name_len){
			continue;
		}
		if(0 == strncasecmp(parameter.c_str(), name.c_str(), name_len)){
			value = parameter.substr(eq_pos + 1);
			return true;
		}
	}
	return false;
}

bool PromptCommandBase::GetExpireParameter(const std::string& parameter, time_t& expire)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"expire_time", "expire", "exp", "e"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "unset") || 0 == strcasecmp(value.c_str(), "no") || 0 == strcasecmp(value.c_str(), "0")){
		expire = 0;
	}else{
		int	nExpire = 0;
		if(!StringToInt(value, nExpire)){
			ERR("The \"expire\"(expire_time, exp) parameter value(%s) must be \"unset\"(no) or a decimal integer string.", value.c_str());
			return false;
		}
		expire = static_cast<time_t>(nExpire);
	}
	return true;
}

bool PromptCommandBase::GetPassphraseParameter(const std::string& parameter, std::string& passphrase)
{
	if(!PromptCommandBase::GetParameter(parameter, {"passphrase", "pass", "p"}, passphrase)){
		return false;
	}
	if(passphrase.empty() || 0 == strcasecmp(passphrase.c_str(), "unset") || 0 == strcasecmp(passphrase.c_str(), "no")){
		passphrase.clear();
	}
	return true;
}

bool PromptCommandBase::GetNoAttributeCheckParameter(const std::string& parameter, bool& is_noattrs)
{
	if(0 == strcasecmp(parameter.c_str(), "noattrcheck") || 0 == strcasecmp(parameter.c_str(), "noattr") || 0 == strcasecmp(parameter.c_str(), "noa")){
		is_noattrs = true;
		return true;
	}
	return false;
}

bool PromptCommandBase::GetMainkeyOnlyCheckParameter(const std::string& parameter, bool& is_mainkeyonly)
{
	if(0 == strcasecmp(parameter.c_str(), "mainkey-only") || 0 == strcasecmp(parameter.c_str(), "mainkeyonly") || 0 == strcasecmp(parameter.c_str(), "mainkey") || 0 == strcasecmp(parameter.c_str(), "main") || 0 == strcasecmp(parameter.c_str(), "only")){
		is_mainkeyonly = true;
		return true;
	}
	return false;
}

bool PromptCommandBase::GetDimensionParameter(const std::string& parameter, int& dimension)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"dimension", "dim", "d"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "default") || 0 == strcasecmp(value.c_str(), "def") || 0 == strcasecmp(value.c_str(), "0")){
		dimension = 0;
	}else{
		if(!StringToInt(value, dimension)){
			ERR("The \"dimension\"(dim, d) parameter value(%s) must be \"default\"(def) or a decimal integer string.", value.c_str());
			return false;
		}
		if(dimension <= 0 || ProjectionMatrix::MAX_MATRIX_DIMENSION < dimension){
			ERR("The \"dimension\"(dim, d) parameter value(%s) must be over 0 to 2^16(65536).", value.c_str());
			return false;
		}
	}
	return true;
}

bool PromptCommandBase::GetBitSizeParameter(const std::string& parameter, int& bitsize)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"bit_size", "bitsize", "bit_count", "bitcount", "bit_cnt", "bitcnt", "bit", "b"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "default") || 0 == strcasecmp(value.c_str(), "def") || 0 == strcasecmp(value.c_str(), "0")){
		bitsize = 0;
	}else{
		if(!StringToInt(value, bitsize)){
			ERR("The \"bit_size\"(bitsize, bit, b) parameter value(%s) must be \"default\"(def) or a decimal integer string.", value.c_str());
			return false;
		}
		if(bitsize <= 0 || ProjectionMatrix::MAX_MATRIX_BITSIZE < bitsize){
			ERR("The \"bit_size\"(bitsize, bit, b) parameter value(%s) must be over 0 to 2^12(4096).", value.c_str());
			return false;
		}
	}
	return true;
}

bool PromptCommandBase::GetNameParameter(const std::string& parameter, std::string& name)
{
	if(!PromptCommandBase::GetParameter(parameter, {"name", "n"}, name)){
		return false;
	}
	if(name.empty() || 0 == strcasecmp(name.c_str(), "default") || 0 == strcasecmp(name.c_str(), "def")){
		name.clear();
	}
	return true;
}

bool PromptCommandBase::GetVersionParameter(const std::string& parameter, std::string& version)
{
	if(!PromptCommandBase::GetParameter(parameter, {"version", "ver", "v"}, version)){
		return false;
	}
	if(version.empty() || 0 == strcasecmp(version.c_str(), "default") || 0 == strcasecmp(version.c_str(), "def")){
		version.clear();
	}
	return true;
}

bool PromptCommandBase::GetK2HashKeyParameter(const std::string& parameter, std::string& key, bool& usePreloadedEm, std::string& emfile)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"key", "k"}, value)){
		if(!PromptCommandBase::GetEmfileParameter(parameter, value)){
			return false;
		}else{
			// emfile=
			key.clear();
			usePreloadedEm	= false;
			emfile			= value;
		}
	}else{
		if(PromptCommandBase::IsEmbeddingDataParameter(value)){
			// key=em
			key.clear();
			usePreloadedEm = true;
			emfile.clear();
		}else{
			// key=<key>
			key				= value;
			usePreloadedEm	= false;
			emfile.clear();
		}
	}
	return true;
}

bool PromptCommandBase::GetEmbeddingParameter(const std::string& parameter, bool& usePreloadedEm, std::string& emfile)
{
	std::string	key("");
	if(!PromptCommandBase::GetK2HashKeyParameter(parameter, key, usePreloadedEm, emfile)){
		return false;
	}
	if(!key.empty()){
		// this method allows only "key=em"
		return false;
	}
	return true;
}

bool PromptCommandBase::GetOutputfileParameter(const std::string& parameter, std::string& file)
{
	if(!PromptCommandBase::GetParameter(parameter, {"output_file", "outputfile", "output", "ofile", "of"}, file)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetEmfileParameter(const std::string& parameter, std::string& file)
{
	if(!PromptCommandBase::GetParameter(parameter, {"emfile", "emf"}, file)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetPmfileParameter(const std::string& parameter, std::string& file)
{
	if(!PromptCommandBase::GetParameter(parameter, {"pmfile", "pmf"}, file)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetValuefileParameter(const std::string& parameter, std::string& file)
{
	if(!PromptCommandBase::GetParameter(parameter, {"valuefile", "vf"}, file)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetValueParameter(const std::string& parameter, std::string& contents)
{
	if(!PromptCommandBase::GetParameter(parameter, {"value", "v"}, contents)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetScoreParameter(const std::string& parameter, std::string& contents)
{
	if(!PromptCommandBase::GetParameter(parameter, {"score", "s"}, contents)){
		return false;
	}
	return true;
}

bool PromptCommandBase::GetBinaryHashBitSizeParameter(const std::string& parameter, int& bitsize)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"cal_bit_size", "bit_size", "bitsize", "bit_count", "bitcount", "bit_cnt", "bitcnt", "bit", "b"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "default") || 0 == strcasecmp(value.c_str(), "def") || 0 == strcasecmp(value.c_str(), "0")){
		bitsize = 0;
	}else{
		if(!StringToInt(value, bitsize)){
			ERR("The \"cal_bit_size\"(bit_size, bitsize, bit, b) parameter value(%s) must be \"default\"(def) or a decimal integer string.", value.c_str());
			return false;
		}
		if(bitsize <= 0 || ProjectionMatrix::MAX_MATRIX_BITSIZE < bitsize){
			ERR("The \"cal_bit_size\"(bit_size, bitsize, bit, b) parameter value(%s) must be over 0 to 2^12(4096).", value.c_str());
			return false;
		}
	}
	return true;
}

bool PromptCommandBase::GetNeighborKeyLimitParameter(const std::string& parameter, size_t& neighborkeylimit)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"neighbor_key_limit", "neighbor_limit", "neighbor_key", "key_limit", "nkl", "kl"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "default") || 0 == strcasecmp(value.c_str(), "def") || 0 == strcasecmp(value.c_str(), "0")){
		neighborkeylimit = 0;
	}else{
		int	tmpval = 0;
		if(!StringToInt(value, tmpval) || tmpval < 0){
			ERR("The \"neighbor_key_limit\"(neighbor_limit, neighbor_key, key_limit, nkl, kl) parameter value(%s) must be \"default\"(def) or a decimal positive string.", value.c_str());
			return false;
		}
		if(tmpval <= 0 || (1 << 22) < tmpval){
			ERR("The \"neighbor_key_limit\"(neighbor_limit, neighbor_key, key_limit, nkl, kl) parameter value(%s) must be over 0 to 2^22(4194304).", value.c_str());
			return false;
		}
		neighborkeylimit = static_cast<size_t>(tmpval);
	}
	return true;
}

bool PromptCommandBase::GetContentDirectLimitParameter(const std::string& parameter, size_t& directlimit)
{
	std::string	value;
	if(!PromptCommandBase::GetParameter(parameter, {"content_direct_limit", "direct_limit", "content_direct", "content_limit", "cdl", "dl"}, value)){
		return false;
	}
	if(0 == strcasecmp(value.c_str(), "default") || 0 == strcasecmp(value.c_str(), "def") || 0 == strcasecmp(value.c_str(), "0")){
		directlimit = 0;
	}else{
		int	tmpval = 0;
		if(!StringToInt(value, tmpval) || tmpval < 0){
			ERR("The \"content_direct_limit\"(direct_limit, content_direct, content_limit, cdl, dl) parameter value(%s) must be \"default\"(def) or a decimal positive string.", value.c_str());
			return false;
		}
		directlimit = static_cast<size_t>(tmpval);
	}
	return true;
}

//
// Utility : Convert the Cosine Similarity Score string to float value
//
bool PromptCommandBase::CvtScoreStringToFloat(const std::string& str, float& value)
{
	if(str.empty()){
		return false;
	}
	try{
		// try to convert
		size_t		end_idx	= 0;
		value				= std::stof(str, &end_idx);
		std::string	remain	= str.substr(end_idx);

		// check remain string(allow only "f")
		if(!remain.empty() && 0 != strcasecmp(remain.c_str(), "f")){
			return false;
		}
	}catch(const std::invalid_argument&){
		// invalid string(not number for float)
		return false;
	}catch(const std::out_of_range&){
		// overflow
		return false;
	}

	if(1.0f < value){
		// upper limit is 1.0f
		return false;
	}
	return true;
}

//---------------------------------------------------------
// PromptCommandBase and Inherit Classes Methods for Help
//---------------------------------------------------------
//
// Print help
//
void PromptCommandBase::PrintHelp()
{
	static const char summary[] =
		"\n"
		"Prompt Commands:\n"
		"    help(h) <command>                     Print command detail\n"
		"    quit(q)                               Quit(exit) the dialogue and terminate the program (alternative command: exit(e))\n"
		"    shell(sh)                             Escape to a shell (alternative command: !)\n"
		"    echo <strings...>                     Print argument string (alternative command: print)\n"
		"    sleep <second>                        Wait second (alternative command: wait)\n"
		"\n"
		"    library_version(libver)               Displays the library version (alternative command: libversion)\n"
		"    history_max(hmax) <count>             Set maximum count for prompt command history (alternative command: hismax)\n"
		"    history(his)                          Display all command history (alternative command: history_log)\n"
		"    save_history(save) <file>             Save all command history to the file (alternative command: save_his)\n"
		"    debug_level(d) <debug level>          Specifies the debug level (alternative command: debug)\n"
		"    debug_logfile(dlog) <file>            Specifies the file for debug log output (alternative command: debug_log)\n"
		"\n"
		"    common_attribute(attr) <...>          Set and Print common expire and passphrase for data in k2hash.\n"
		"    common_config(cc) <...>               Set or Print Binary Hash bit count, neighbor keys limit and content direct limit.\n"
		"    last_status($?)                       Print the exit status of the previous prompt command(0: success, 1: failure)\n"
		"    lap_time(lap) <\"on\" | \"off\"> <...>    Toggle disable/enable the display of the elapsed time for command execution\n"
		"    run <file>                            Load a file containing prompt commands and execute those commands\n"
		"    break                                 Specify stop to run even if an error occurs while executing the prompt commands(default)\n"
		"    nobreak                               Specify continue to run even if an error occurs while executing the prompt commands\n"
		"\n"
		"    load_projection_matrix(pm) <file>     Load and set the Projection Matrix data\n"
		"    load_embedding(em) <file>             Load and set the Embedding data as Preloaded data\n"
		"    create_projection_matrix(cpm) <...>   Create new Projection Matrix, and either load it or save it to file (alternative command: create_pm)\n"
		"    create_embedding(cem) <...>           Create embedding data, and either load it as preloaded data or save it to file (alternative command: create_em)\n"
		"    information(i) <pm | em>              Display information of Projection Matrix or preloaded embeddings data (alternative command: info, status)\n"
		"    dump(dmp) <pm | em> <...>             Dump and save projection matrix or preloaded embeddings data\n"
		"    create_k2hash_key(key) <file>         Create and print a K2Hash key for embedding data (alternative command: k2h_key, k2hkey)\n"
		"    calculate_binary_hash(cbh) <...>      Calculate Binary hash value (alternative command: calculate_bh, cal_bh)\n"
		"\n"
		"    set(s) <...>                          Set embedding data to K2Hash (alternative command: push, put)\n"
		"    get(g) <...>                          Get raw embedding data from K2Hash to file (alternative command: print, p)\n"
		"    remove(rm) <...>                      Remove embedding data in K2Hash (alternative command: delete, del)\n"
		"    read_summary(r) <...>                 Read summary embedding data in K2Hash (alternative command: read)\n"
		"    search(find) <...>                    Run vector search for the embedding data in K2Hash and find the closest data (alternative command: f)\n"
		"    calculate_cos(cos) <key> <...>        Calculate Cosine Similarity value (alternative command: calculate_cosin_similarity, cal_cos)\n"
		"\n"
		"  *) Please use \"help <command>\" for details about each command.\n"
		"\n";

	fprintf(stdout, summary);
}

void PromptCommandHelp::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: help(h) <command>\n"
		"\n"
		"Discription:\n"
		"    Displays help for <command>.\n"
		"    If <command> is omitted, displays a summary of the Prompt command's help.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandQuit::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: quit(q, exit)\n"
		"\n"
		"Discription:\n"
		"    Terminate the input during console input and exit the program.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandShell::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: shell(sh, !)\n"
		"\n"
		"Discription:\n"
		"    Escape to a shell specified by the SHELL environment variable(or similar).\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandEcho::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: echo(print) <strings...>\n"
		"\n"
		"Discription:\n"
		"    Outputs the specified strings.\n"
		"    This allows you to insert an arbitrary string between command results when a file is specified via the\n"
		"    \"--run_file(-run)\" command line option or the \"run\" command, or when a command is specified directly\n"
		"    using the \"--\" command line option.\n"
		"    This makes the command execution logs easier to read.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandSleep::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: sleep(wait) <second>\n"
		"\n"
		"Discription:\n"
		"    Pauses command execution for <second>.\n"
		"    This allows you to halt sequential operations for a specified period, enabling you to adjust the timing.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandRun::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: run <command file path>\n"
		"\n"
		"Discription:\n"
		"    Load a file containing Prompt commands and execute those commands.\n"
		"    Please prepare a file containing a list of Prompt commands.\n"
		"    For example, you can load a command history file saved using the \"save_history\" command, and re-execute\n"
		"    the same operations.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandBreak::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: break(brk)\n"
		"       nobreak(nbrk)\n"
		"\n"
		"Discription:\n"
		"       Configures whether to stop or continue execution when an error occurs while running consecutive\n"
		"       Prompt commands.\n"
		"\n"
		"       \"break\"(brk) is to stops execution upon encountering an error.\n"
		"       \"nobreak\" is continues execution even if an error occurs.\n"
		"       Default is to stop on error(\"break\").\n"
		"\n"
		"       Consecutive Prompt commands refer to:\n"
		"         Prompt command files executed via the \"--run_file\" launch option.\n"
		"         Direct Prompt commands executed via the \"--\" startup option.\n"
		"         Prompt command files executed via the \"run\" Prompt command.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandHistoryMax::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: history_max(hismax, hmax) <max count>\n"
		"\n"
		"Discription:\n"
		"    Sets the maximum number of entries for the prompt command history.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandHistory::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: history(history_log, his)\n"
		"\n"
		"Discription:\n"
		"    Displays the entire Prompt command history.\n"
		"    The maximum history count can be set using the \"history_max(hismax, hmax)\" command.\n"
		"    Use the \"save_history(save_his, save)\" command to save this history.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandSaveHistory::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: save_history(save_his, save) <output file path>\n"
		"\n"
		"Discription:\n"
		"    Save all prompt command history(maximum history count) to the <output file path> file.\n"
		"    You can load the saved prompt command history file using the \"run\" prompt command or\n"
		"    \"--run_file(-run)\" command line option and re-execute those commands.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandLibVersion::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: library_version(libversion, libver)\n"
		"\n"
		"Discription:\n"
		"    Displays the versions of the K2HVEC, K2HASH, and FULLOCK libraries used by this program.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandLapTime::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: lap_time(lap) <\"on\" | \"off\">\n"
		"\n"
		"Discription:\n"
		"    Toggle disable/enable the display of the elapsed time for command execution.\n"
		"    Specify \"on\" or \"off\".\n"
		"    If this is omitted, the setting toggles between enabled and disabled.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandInfo::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: information(info, status, i) <pm | em>\n"
		"\n"
		"Discription:\n"
		"    Display information of Projection Matrix(pm) or preloaded embeddings data(em).\n"
		"\n"
		"    This command displays information regarding the Projection Matrix and embedding data held internally by this\n"
		"    program.\n"
		"\n"
		"    The Projection Matrix includes \"builtin\" data integrated into the k2hvec library.\n"
		"    This data can be replaced using the \"load_projection_matrix(pm)\" or \"create_projection_matrix(cpm)\" commands.\n"
		"\n"
		"    The embedding data can be stored internally as preloaded data using the \"load_embedding(em)\" or\n"
		"    \"create_embedding(cem)\" commands.\n"
		"    While embedding data can be specified via a file when executing commands such as \"create_main_key(key)\",\n"
		"    \"create_binary_hash(cbh)\", or \"calculate_cos(cos)\", storing it as preloaded data allows it to be used in\n"
		"    place of a file.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandDump::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: dump(dmp) <pm | em> [output=<output file>]\n"
		"\n"
		"Discription:\n"
		"    Dump the Projection Matrix or preloaded embedding data\n"
		"\n"
		"    The Projection Matrix includes \"builtin\" data integrated into the k2hvec library.\n"
		"    This data can be replaced using the \"load_projection_matrix(pm)\" or \"create_projection_matrix(cpm)\" commands.\n"
		"\n"
		"    If \"output=<output file>\" is specified, the dumped content will be output to that file. If not specified,\n"
		"    it will be output to stdout.\n"
		"\n"
		"    The embedding data can be stored internally as preloaded data using the \"load_embedding(em)\" or\n"
		"    \"create_embedding(cem)\" commands.\n"
		"    While embedding data can be specified via a file when executing commands such as \"create_main_key(key)\",\n"
		"    \"create_binary_hash(cbh)\", or \"calculate_cos(cos)\", storing it as preloaded data allows it to be used in\n"
		"    place of a file.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandDebugLevel::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: debug_level(debug, d) <debug level> [\"with_k2hash\"]\n"
		"\n"
		"Discription:\n"
		"    Specifies the debug level: : SILENT(SLT), ERROR(ERR), WARNING(WAN), MESSAGE(MSG), or DUMP(DMP)\n"
		"    If \"<debug level>\" is omitted, the current debug level, the path to the debug log file are displayed.\n"
		"\n"
		"    If the \"with_k2hash\"(k2hash, k2h, wk2h) parameter is specified, the debug level for libk2hash is also set to the\n"
		"    same level.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandDebugFile::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: debug_logfile(debug_log, dlog) <output file> [\"with_k2hash\"]\n"
		"\n"
		"Discription:\n"
		"    Specifies the file to which the debug log is output.\n"
		"    Normally, the debug log is output to stdout. If <output file> is omitted, the output is set to\n"
		"    (or reverts to) stdout.\n"
		"\n"
		"    If the \"with_k2hash\"(k2hash, k2h, wk2h) parameter is specified, the debug level for libk2hash is also set to the\n"
		"    same level.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandLastStatus::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: last_status(laststatus, $?) [\"long\"]\n"
		"\n"
		"Discription:\n"
		"    Print the exit status of the previous prompt command.\n"
		"    Displays \"0\" if the previous command completed successfully, or \"1\" otherwise.\n"
		"\n"
		"    If \"long\"(l) is specified, the result is displayed as a detailed text description\n"
		"    rather than as \"0\"/\"1\".\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCommonAttribute::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: common_attribute(common, common_attr, com_attr, attr) [expire=<second | \"unset\">] [pass=<passphrase | \"unset\">]\n"
		"\n"
		"Discription:\n"
		"    Set or Print common values for the data expiration period(\"expire\") and passphrase(\"pass\") used when storing\n"
		"    or retrieving data in K2Hash.\n"
		"\n"
		"    By setting these values, you can omit the expiration period and passphrase parameters in the \"set(s)\", \"get(g)\",\n"
		"    \"remove(rm)\", \"read_summary(r)\", and \"search(find)\" commands, allowing them to use these common values instead.\n"
		"    If these values are \"unset\" and the parameters are omitted in each command, reading and writing will be performed\n"
		"    with no expiration period and no passphrase.\n"
		"\n"
		"    Executing this command without arguments displays the currently configured expiration time(in seconds) and passphrase.\n"
		"\n"
		"    Specifying the \"expire=value\" argument sets or unsets the expiration time(in seconds).\n"
		"    For \"value\", specify the time in seconds as a decimal number. If \"0\" or \"unset\" is specified, the expiration\n"
		"    time will be disabled.\n"
		"\n"
		"    Specifying the \"pass=value\" argument sets or unsets the passphrase.\n"
		"    For \"value\", specify a passphrase string. If empty or \"unset\" is specified, the passphrase will be disabled.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCommonConfig::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: common_config(common_conf, comconf, cc) [cal_bit_size=<bit count | \"default\">] [neighbor_key_limit=<limit | \"default\">] [content_direct_limit=<limit | \"default\">]\n"
		"\n"
		"Discription:\n"
		"    Set or Print the Vecotr Search keys configuration about the Binary Hash bit count for embedding data and\n"
		"    the maximum number of neighbor keys used during search.\n"
		"\n"
		"    Use the \"cal_bit_size\" parameter to specify the bit size used when calculating the BinaryHash\n"
		"    from embedding data. Specify \"default\" to reset it to the k2hvec library's default value.\n"
		"\n"
		"    Use the \"neighbor_key_limit\" parameter to specify the maximum number of neighbor keys used during search.\n"
		"    Specify \"default\" to reset it to the k2hvec library's default value.\n"
		"\n"
		"    Use the \"content_direct_limit\" parameter to specify the maximum size for saving content text directly to\n"
		"    the main key.\n"
		"    Specify \"default\" to reset it to the k2hvec library's default value.\n"
		"\n"
		"    If called without any parameters, the currently configured values will be displayed.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandK2hKey::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: create_k2hash_key(k2h_key, k2hkey, key) <embedding file>\n"
		"\n"
		"Discription:\n"
		"    Create and print a K2Hash key for embedding data.\n"
		"\n"
		"    The \"<embedding file>\" is optional. If \"<embedding file>\" is not specified, If there is\n"
		"    Preloaded embedding data, use it instead of \"<embedding file>\".\n"
		"    You can load Preloaded embedding data using \"load_embedding(em)\" or \"create_embedding(cem)\".\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandSet::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: set(s, push, put) [expire=<second | \"unset\">] [pass=<passphrase | \"unset\">] [emfile=<embedding file>] [valuefile=<content file> | value=<contents>]\n"
		"\n"
		"Discription:\n"
		"    Set embedding data to K2Hash.\n"
		"\n"
		"    \"valuefile=<content file>\" or \"<contents>\" is the value for embedding data to be registered in K2hash.\n"
		"    Specify either \"valuefile=<content file>\" or \"value=<contents>\".\n"
		"    If the \"value=<contents>\" value contains spaces, enclose it in double quotes(\").\n"
		"    If both are omitted, an empty string(\"\") will be set.\n"
		"\n"
		"    The \"emfile=<embedding file>\" is optional. If \"emfile=<embedding file>\" is not specified, If there is\n"
		"    Preloaded embedding data, use it instead of \"<embedding file>\".\n"
		"    You can load Preloaded embedding data using \"load_embedding(em)\" or \"create_embedding(cem)\".\n"
		"\n"
		"    Executing this command without specifying the \"expire\" and \"pass\" arguments sets data using the\n"
		"    values specified by the Common Attribute command(\"common_attribute\").\n"
		"    If the Common Attribute command(\"common_attribute\") is also not configured, embedded data is stored\n"
		"    in k2hash without setting an expiration period or passphrase.\n"
		"\n"
		"    Specifying the \"expire=value\" argument sets or unsets the expiration time(in seconds).\n"
		"    For \"value\", specify the time in seconds as a decimal number. If \"0\" or \"unset\" is specified, the expiration\n"
		"    time will be disabled.\n"
		"\n"
		"    Specifying the \"pass=value\" argument sets or unsets the passphrase.\n"
		"    For \"value\", specify a passphrase string. If empty or \"unset\" is specified, the passphrase will be disabled.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandGet::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: get(g, print, p) [key=<k2hash key> | key=\"em\" | emfile=<embedding file>] [noattrcheck] [pass=<passphrase | \"unset\">] [output=<output file>]\n"
		"\n"
		"Discription:\n"
		"    This command writes the value in the k2hash database corresponding to the embedded data to the\n"
		"    specified output file.\n"
		"\n"
		"    There are three ways to specify the key for retrieving K2Hash data:\n"
		"      key=<key name>    Directly specify the K2Hash key name.\n"
		"      key=\"em\"          Use Preloaded embedding data.\n"
		"      emfile=<file>     Specify the embedding data file.\n"
		"\n"
		"    If \"key\" and \"emfile\" are omitted, Preloaded embedding data(equivalent to \"key=em\") is used.\n"
		"\n"
		"    The data read from K2Hash is written to the \"output=<output file>\".\n"
		"    If \"output=<output file>\" is omitted, a summary of the K2Hash data will be displayed the data length\n"
		"    and FNV-1a hash value instead.\n"
		"\n"
		"    Executing this command without specifying the \"pass\" argument reads data using the value\n"
		"    specified by the Common Attribute command(\"common_attribute\").\n"
		"    If the Common Attribute command(\"common_attribute\") is also unset, data will be read from\n"
		"    k2hash without a passphrase.\n"
		"\n"
		"    When \"noattrcheck\" is specified, reading is performed ignoring the attributes set on the K2Hash\n"
		"    data. Normally, reading is affected by the set attributes(such as expire piriod time setting).\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandRemove::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: remove(rm, delete, del) [key=<k2hash key> | key=\"em\" | emfile=<embedding file>] [mainkey-only]\n"
		"\n"
		"Discription:\n"
		"    Remove the embedding data specified by \"<k2hash key>\" from K2Hash, and Index key entries for vector search\n"
		"    linked to that embedding data is also removed.\n"
		"\n"
		"    There are three ways to specify the key for retrieving K2Hash data:\n"
		"      key=<key name>    Directly specify the K2Hash key name.\n"
		"      key=\"em\"          Use Preloaded embedding data.\n"
		"      emfile=<file>     Specify the embedding data file.\n"
		"\n"
		"    If \"key\" and \"emfile\" are omitted, Preloaded embedding data(equivalent to \"key=em\") is used.\n"
		"\n"
		"    If \"mainkey-only\" is specified, only the embedding data is removed, but the index key entries is not removed.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandReadSummary::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: read_summary(read, r) <k2hash key> [noattrcheck] [pass=<passphrase | \"unset\">]\n"
		"\n"
		"Discription:\n"
		"    Reads the embedding data in K2Hash specified by \"<k2hash key>\" and displays its summary.\n"
		"\n"
		"    Executing this command without specifying the \"pass\" argument reads data using the value\n"
		"    specified by the Common Attribute command(\"common_attribute\").\n"
		"    If the Common Attribute command(\"common_attribute\") is also unset, data will be read from\n"
		"    k2hash without a passphrase.\n"
		"\n"
		"    When \"noattrcheck\" is specified, reading is performed ignoring the attributes set on the K2Hash\n"
		"    data. Normally, reading is affected by the set attributes(such as expire piriod time setting).\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandSearch::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: search(find, f) [score=<float string>] [key=\"em\" | emfile=<embedding file>] [output=<output file>] [noattrcheck] [pass=<passphrase | \"unset\">]\n"
		"\n"
		"    This command executes a vector search in K2Hash to find the embedding data closest to the value\n"
		"    specified in the argument.\n"
		"\n"
		"    Set the minimum cosine approximation value for \"score=<float string>\".\n"
		"    This value should be set between \"0.0f\" and \"1.0f\".\n"
		"    If omitted, it defaults to \"0.0f,\" and detectes the closest value.\n"
		"\n"
		"    To specify the embedding data using a file, use \"emfile=<embedding file>\".\n"
		"    Alternatively, to specify Preloaded embedding data, use key=\"em\". If both are omitted, the\n"
		"    Preloaded embedding data will be used by default.\n"
		"\n"
		"    If \"output=<output file>\" is specified, the detected embedding data will be written to that file.\n"
		"    If omitted, a summary of the detected embedding data will be displayed.\n"
		"\n"
		"    Executing this command without specifying the \"pass\" argument reads data using the value\n"
		"    specified by the Common Attribute command(\"common_attribute\").\n"
		"    If the Common Attribute command(\"common_attribute\") is also unset, data will be read from\n"
		"    k2hash without a passphrase.\n"
		"\n"
		"    When \"noattrcheck\" is specified, reading is performed ignoring the attributes set on the K2Hash\n"
		"    data. Normally, reading is affected by the set attributes(such as expire piriod time setting).\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandLoadProjection::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: load_projection_matrix(pm) <projection matrix file>\n"
		"\n"
		"Discription:\n"
		"    Load and set the Projection Matrix data.\n"
		"\n"
		"    This command loads Projection Matrix data from a file and sets it as the internal data used by the\n"
		"    k2hvec library.\n"
		"\n"
		"    The k2hvec library maintains an internal Projection Matrix, it includes a built-in Projection Matrix\n"
		"    known as \"builtin\" by default.\n"
		"    This command overwrites and replaces the Projection Matrix held by the k2hvec library.\n"
		"\n"
		"    The Projection Matrix held by the k2hvec library can be verified using the \"information(i)\" or\n"
		"    \"dump(dmp)\" commands.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandLoadEmbedding::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: load_embedding(em) <embedding file>\n"
		"\n"
		"Discription:\n"
		"    Load and set the Embedding data as Preloaded data.\n"
		"\n"
		"    This command loads embedding data as preloaded data.\n"
		"    By preloading the embedding data, you can use this data directly instead of loading it from a file\n"
		"    when executing commands such as \"calculate_binary_hash(cbh)\" or \"calculate_cos(cos)\".\n"
		"\n"
		"    You can verify the preloaded embedding data using the \"information(i)\" or \"dump(dmp)\" commands.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCreateProjection::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: create_projection_matrix(create_pm, cpm) [dimension=<dimension | \"default\">]\n"
		"                                                [bitsize=<bit count | \"default\">]\n"
		"                                                [name=<name | \"default\">]\n"
		"                                                [version=<version | \"default\">]\n"
		"                                                [output=<output file>]\n"
		"\n"
		"Discription:\n"
		"    Create new Projection Matrix data and either load it as the internal Projection Matrix in the\n"
		"    k2hvec library or save it to \"<output file>\" without loading it.\n"
		"\n"
		"    If the generated Projection Matrix will be loaded as the internal data, do not specify \"<output file>\",\n"
		"    \"name\", and \"version\".\n"
		"    If it is specified \"<output file>\", the generated  Projection Matrix will be output to \"<output file>\"\n"
		"    without loading as the internal data.\n"
		"\n"
		"    Each argument follows the specifications below:\n"
		"      <dimension>    Specify the dimension of Projection Matrix as a decimal number. If omit it or specify\n"
		"                     \"deffault\"(def), use the default value(4096).\n"
		"                     (The alias for this option is \"dim\" and \"d\".)\n"
		"      <bitsize>      When creating a Binary hash, the bit size corresponds to the bit size of the Projection\n"
		"                     Matrix, specify this bit size as a decimal number. If omit it or specify \"deffault\"(def),\n"
		"                     use the default value(48).\n"
		"                     (The alias for this option is \"bit_size\", \"bit_count\", \"bitcount\", \"bit_cnt\", \"bitcnt\",\n"
		"                     \"bit\", and \"b\".)\n"
		"      <name>         When saving generated Projection Matrix to \"<output file>\", specify the name to assign\n"
		"                     to it. If omit it or specify \"deffault\"(def), use the default value(\"custom\").\n"
		"                     (The alias for this option is \"n\".)\n"
		"      <version>      When saving generated Projection Matrix to \"<output file>\", specify the version string to\n"
		"                     assign toit. If omit it or specify \"deffault\"(def), use the default value(\"1.0\").\n"
		"                     (The alias for this option is \"ver\" and \"v\".)\n"
		"      <output>       Specify a file path when saving the generated Projection Matrix.\n"
		"\n"
		"    Each argument is optional.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCreateEmbedding::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: create_embedding(create_em, cem) [dimension=<dimension | \"default\">] [output=<output file>]\n"
		"\n"
		"Discription:\n"
		"    Create an Embedding data, and either load it as preloaded data in this program or save it to\n"
		"    \"<output file>\".\n"
		"\n"
		"    If the generated embedding data will be loaded as the preloaded data, do not specify \"<output file>\".\n"
		"    If it is specified arguments after \"<output file>\", the generated  embedding data will be output to\n"
		"    \"<output file>\" without loading as the preloaded data.\n"
		"\n"
		"    Each argument follows the specifications below:\n"
		"      <dimension>    Specify the dimension of Projection Matrix as a decimal number. If omit it or specify\n"
		"                     \"deffault\"(def), use the default value(4096).\n"
		"                     (The alias for this option is \"dim\" and \"d\".)\n"
		"      <output>       Specify a file path when saving the generated embedding data.\n"
		"\n"
		"    Each argument is optional.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCalBinaryHash::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: calculate_binary_hash(calculate_bh, cal_bh, cbh) [key=\"em\" | emfile=<embedding file>] [pmfile=<projection matrix file>]\n"
		"\n"
		"Discription:\n"
		"    Calculate the Binary hash value of the embedding data using a Projection Matrix.\n"
		"\n"
		"    Please specify the embedding data using either \"emfile=<embedding file>\" or \"key=em\".\n"
		"    If \"key=em\" is specified, the Preloaded embedding data will be used.\n"
		"    If both are omitted, the Preloaded embedding data will also be used.\n"
		"\n"
		"    Normally, the Projection Matrix used is either the \"builtin\" data in the k2hvec library\n"
		"    or replaced it by \"load_projection_matrix(pm)\" or \"create_projection_matrix(cpm)\" commands.\n"
		"    If \"pmfile=<projection matrix file>\" is specified, it is loaded into the k2hvec library in the\n"
		"    same way as the \"load_projection_matrix(pm)\" or \"create_projection_matrix(cpm)\" command and\n"
		"    it is used as Projection Matrix.\n"
		"    Note that when this parameter is specified, the Projection Matrix will remain replaced for\n"
		"    subsequent operations.\n"
		"\n";

	fprintf(stdout, help);
}

void PromptCommandCalCosineSimilarity ::PrintHelp()
{
	static const char help[] =
		"\n"
		"Usage: calculate_cos(calculate_cosin_similarity, cal_cos, cos) <k2hash key> [noattrcheck] [pass=<passphrase | \"unset\">] [key=\"em\" | emfile=<embedding file>] [simple]\n"
		"\n"
		"Discription:\n"
		"    Calculates Cosine Similarity value between two sets of embedding data.\n"
		"\n"
		"    The Cosine Similarity value is calculated using the embedding data stored in K2hash and the embedding\n"
		"    data specified via options(either specified embedding data file or Preloaded embedding data).\n"
		"\n"
		"    One set of embedding data is retrieved from the K2Hash specified by the argument(\"<k2hash key>\").\n"
		"    Executing this command without specifying the \"pass\" argument reads data using the value\n"
		"    specified by the Common Attribute command(\"common_attribute\").\n"
		"    If the Common Attribute command(\"common_attribute\") is also unset, data will be read from\n"
		"    k2hash without a passphrase.\n"
		"    When \"noattrcheck\" is specified, reading is performed ignoring the attributes set on the K2Hash\n"
		"    data. Normally, reading is affected by the set attributes(such as expire piriod time setting).\n"
		"\n"
		"    The other set is specified by \"key=em\" or \"emfile=<embedding file>\" argument. If \"<embedding file>\"\n"
		"    is specified for this argument, the contents as embedding data of that file are used.\n"
		"    If the argument is set to \"key=em\", Preloaded embedding data is used. If both are omitted, the Preloaded\n"
		"    embedding data will also be used.\n"
		"\n"
		"    When the \"simple\" parameter is specified, the cosine similarity calculation is simplified\n"
		"    under the assumption that the target embedding data is normalized.\n"
		"\n";

	fprintf(stdout, help);
}

//---------------------------------------------------------
// PromptCommandBase Class Methods : Attributes
//---------------------------------------------------------
void PromptCommandBase::SetLapTime(bool enable)
{
	PromptCommandBase::useLapTime = enable;
}

void PromptCommandBase::ToggleLapTime()
{
	PromptCommandBase::useLapTime = !PromptCommandBase::useLapTime;
}

void PromptCommandBase::EnableLapTime()
{
	PromptCommandBase::SetLapTime(true);
}

void PromptCommandBase::DisableLapTime()
{
	PromptCommandBase::SetLapTime(false);
}

//---------------------------------------------------------
// PromptCommandBase Class Methods : Parser
//---------------------------------------------------------
//
// Prompt Command Parser
//
// This function parses commands (an array of strings) entered via the prompt.
// It also supports parsing prompt commands passed as command line arguments following
// the "--" delimiter.
// The parsed commands are stored individually in the "PromptCommandArgs" variable.
// This function only parses the provided arguments(an array of strings), it does not
// validate the individual commands (such as command names or arguments) within the
// "PromptCommandArgs" variable.
//
// The following are notable examples:
//	<input argvs>							<PromptCommandArgs>
//	cmd a1 a2								{["cmd", "a1", "a2"]}
//	"cmd a1 a2"								{["cmd", "a1", "a2"]}
//	'cmd a1 a2'								{["cmd", "a1", "a2"]}
//	"cmd1 a1; cmd2 a2"						{["cmd1", "a1"], ["amd2", "a2"]}
//	cmd1 a1\; cmd2 a2						{["cmd1", "a1"], ["amd2", "a2"]}
//	cmd1 a1 ";" cmd2 a2						{["cmd1", "a1"], ["amd2", "a2"]}
//	cmd1 a1 ';' cmd2 a2						{["cmd1", "a1"], ["amd2", "a2"]}
//
bool PromptCommandBase::ParsePromptArgs(const std::vector<std::string>& Argvs, PromptCommandArgs& PromptArgs)
{
	// Quote range status
	enum class QuoteState{
		NO_QUOTE,
		SINGLE_QUOTE,
		DOUBLE_QUOTE
	};

	//
	// [Primary processing]
	//
	// Parsing distinguishes between content within quotes (single or double) and
	// content outside of them. The quotation marks themselves are output as-is.
	// Escape characters('\') and quote characters are not included in the output
	// unless they are escaped(in which case they are included).
	// Additionally, if a semicolon(';') is encountered outside of quoted sections,
	// the preceding string is delimited as a single command.
	// Semicolons found within quoted sections are treated as literal characters
	// rather than command delimiters and are included in the output as-is.
	//
	PromptCommandArgs	FirstResult;
	PromptArgList		OneCommand;
	std::string			strOneArg("");
	QuoteState			QuoteArea			= QuoteState::NO_QUOTE;
	bool				beforeEscapeChar	= false;
	for(auto argiter = Argvs.begin(); argiter != Argvs.end(); ++argiter){
		const std::string&	strOrgArg = *argiter;

		for(auto citer = strOrgArg.cbegin(); citer != strOrgArg.cend(); ++citer){
			char	carg = *citer;

			if('"' == carg && !beforeEscapeChar){
				// Double quote (add charactor for second parsing)
				if(QuoteState::NO_QUOTE == QuoteArea){
					// Start double quote area
					QuoteArea = QuoteState::DOUBLE_QUOTE;
				}else if(QuoteState::DOUBLE_QUOTE == QuoteArea){
					// End of double quote area
					QuoteArea = QuoteState::NO_QUOTE;
				}
				strOneArg += carg;
				beforeEscapeChar = false;

			}else if('\'' == carg && !beforeEscapeChar){
				// Single quote (add charactor for second parsing)
				if(QuoteState::NO_QUOTE == QuoteArea){
					// Start single quote area
					QuoteArea = QuoteState::SINGLE_QUOTE;
				}else if(QuoteState::SINGLE_QUOTE == QuoteArea){
					// End of single quote area
					QuoteArea = QuoteState::NO_QUOTE;
				}
				strOneArg += carg;
				beforeEscapeChar = false;

			}else if(';' == carg && !beforeEscapeChar){
				// Semicolon
				if(QuoteState::NO_QUOTE != QuoteArea){
					// treated as a "normal character"
					strOneArg += carg;
				}else{
					// End of command
					const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
					const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
					if(std::string::npos != first){
						// add new argument in command afer trimming
						strOneArg = strOneArg.substr(first, (last - first + 1));
						if(!strOneArg.empty()){
							OneCommand.emplace_back(strOneArg);
						}
					}
					strOneArg.clear();

					if(!OneCommand.empty()){
						// add new command
						FirstResult.emplace_back(OneCommand);
						OneCommand.clear();
					}
				}
				beforeEscapeChar = false;

			}else if('\\' == carg){
				// Escape
				if(beforeEscapeChar){
					// put escape charactor
					strOneArg += carg;
					beforeEscapeChar = false;
				}else{
					// skip this charactor
					beforeEscapeChar = true;
				}

			}else if(std::isspace(static_cast<unsigned char>(carg))){
				// Space charactor
				if(QuoteState::NO_QUOTE != QuoteArea){
					// treated as a "normal character"
					strOneArg += ' ';					// convert it to space charactor
				}else{
					// End of one argument
					const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
					const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
					if(std::string::npos != first){
						// add new argument in command afer trimming
						strOneArg = strOneArg.substr(first, (last - first + 1));
						if(!strOneArg.empty()){
							OneCommand.emplace_back(strOneArg);
						}
					}
					strOneArg.clear();
				}
				beforeEscapeChar = false;

			}else{
				// Other: normal character
				strOneArg += carg;
				beforeEscapeChar = false;
			}
		}
		if(QuoteState::NO_QUOTE != QuoteArea || beforeEscapeChar){
			// Error: Unmatched quote or Espace charactor
			return false;
		}
		if(!strOneArg.empty()){
			// Last command arg
			const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
			const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
			if(std::string::npos != first){
				// add new argument in command afer trimming
				strOneArg = strOneArg.substr(first, (last - first + 1));
				if(!strOneArg.empty()){
					OneCommand.emplace_back(strOneArg);
				}
			}
			strOneArg.clear();
		}
	}
	if(QuoteState::NO_QUOTE != QuoteArea || beforeEscapeChar){
		// Error: Unmatched quote or Espace charactor
		return false;
	}
	if(!strOneArg.empty()){
		// Last command arg
		const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
		const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
		if(std::string::npos != first){
			// add new argument in command afer trimming
			strOneArg = strOneArg.substr(first, (last - first + 1));
			if(!strOneArg.empty()){
				OneCommand.emplace_back(strOneArg);
			}
		}
	}
	if(!OneCommand.empty()){
		// add last command
		FirstResult.emplace_back(OneCommand);
	}

	//
	// [Secondary processing]
	//
	// This is performed on the command list(a list of arguments for each command)
	// that has already undergone initial processing(primary processing).
	//
	// Processing is carried out for each command as follows:
	//
	// (1) When there is only a single command argument list
	//		This applies to cases such as when the command line is enclosed in quotes.
	//		for example, when the command is wrapped in quotes like "<command> <arg> ...".
	//
	//		In this case, the arguments are parsed based on spaces and broken down
	//		into a list of arguments(this is essentially the same as the primary
	//		processing).
	//		Additionally, if a semicolon(";") is detected, the command is split.
	//
	// (2) When there are multiple command argument lists
	//		Each individual argument is checked; if the entire argument is enclosed
	//		in quotes, the quotes are removed.
	//
	PromptCommandArgs	SecondResult;
	beforeEscapeChar	= false;
	QuoteArea			= QuoteState::NO_QUOTE;
	OneCommand.clear();
	strOneArg.clear();
	for(auto firstiter = FirstResult.begin(); firstiter != FirstResult.end(); ++firstiter){
		const PromptArgList&	CommandArgList	= *firstiter;

		if(1 == CommandArgList.size()){
			// CASE: only one argument for a command
			std::string	strTmpArg = CommandArgList.front();

			// Cut quotes
			if(	2 <= strTmpArg.size() &&																		// 2 < argement length
				(
					('"' == strTmpArg.front() && strTmpArg.find_first_of('"', 1) == (strTmpArg.size() - 1))	||	// "..."
					('\'' == strTmpArg.front() && strTmpArg.find_first_of('\'', 1) == (strTmpArg.size() - 1))	// '...'
				)
			){
				// This argument is enclosed in its entirety by "..." or '...', so cut both quote
				strTmpArg = strTmpArg.substr(1, strTmpArg.size() - 2);
			}

			// Parse arguments
			for(auto citer = strTmpArg.cbegin(); citer != strTmpArg.cend(); ++citer){
				char	carg = *citer;

				if('"' == carg && !beforeEscapeChar){
					// Double quote
					if(QuoteState::NO_QUOTE == QuoteArea){
						// Start double quote area
						QuoteArea = QuoteState::DOUBLE_QUOTE;
					}else if(QuoteState::DOUBLE_QUOTE == QuoteArea){
						// End of double quote area
						QuoteArea = QuoteState::NO_QUOTE;
					}else{
						strOneArg += carg;
					}
					beforeEscapeChar = false;

				}else if('\'' == carg && !beforeEscapeChar){
					// Single quote
					if(QuoteState::NO_QUOTE == QuoteArea){
						// Start single quote area
						QuoteArea = QuoteState::SINGLE_QUOTE;
					}else if(QuoteState::SINGLE_QUOTE == QuoteArea){
						// End of single quote area
						QuoteArea = QuoteState::NO_QUOTE;
					}else{
						strOneArg += carg;
					}
					beforeEscapeChar = false;

				}else if(';' == carg && !beforeEscapeChar){
					// Semicolon
					if(QuoteState::NO_QUOTE != QuoteArea){
						// treated as a "normal character"
						strOneArg += carg;
					}else{
						// End of command
						const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
						const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
						if(std::string::npos != first){
							// add new argument in command afer trimming
							strOneArg = strOneArg.substr(first, (last - first + 1));
							if(!strOneArg.empty()){
								OneCommand.emplace_back(strOneArg);
							}
						}
						strOneArg.clear();

						if(!OneCommand.empty()){
							// add new command
							SecondResult.emplace_back(OneCommand);
							OneCommand.clear();
						}
					}
					beforeEscapeChar = false;

				}else if('\\' == carg){
					// Escape
					if(beforeEscapeChar){
						// put escape charactor
						strOneArg += carg;
						beforeEscapeChar = false;
					}else{
						// skip this charactor
						beforeEscapeChar = true;
					}

				}else if(std::isspace(static_cast<unsigned char>(carg))){
					// Space charactor
					if(QuoteState::NO_QUOTE != QuoteArea){
						// treated as a "normal character"
						strOneArg += ' ';					// convert it to space charactor
					}else{
						// End of one argument
						const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
						const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
						if(std::string::npos != first){
							// add new argument in command afer trimming
							strOneArg = strOneArg.substr(first, (last - first + 1));
							if(!strOneArg.empty()){
								OneCommand.emplace_back(strOneArg);
							}
						}
						strOneArg.clear();
					}
					beforeEscapeChar = false;

				}else{
					// Other: normal character
					strOneArg += carg;
					beforeEscapeChar = false;
				}
			}
			if(QuoteState::NO_QUOTE != QuoteArea || beforeEscapeChar){
				// Error: Unmatched quote or Espace charactor
				return false;
			}
			if(!strOneArg.empty()){
				// Last command arg
				const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
				const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
				if(std::string::npos != first){
					// add new argument in command afer trimming
					strOneArg = strOneArg.substr(first, (last - first + 1));
					if(!strOneArg.empty()){
						OneCommand.emplace_back(strOneArg);
					}
				}
				strOneArg.clear();
			}
			if(!OneCommand.empty()){
				// add last command
				SecondResult.emplace_back(OneCommand);
				OneCommand.clear();
			}

		}else{
			// CASE: some arguments for a command
			for(auto argiter = CommandArgList.begin(); argiter != CommandArgList.end(); ++argiter){
				strOneArg = *argiter;

				// Cut quotes
				if(	2 <= strOneArg.size() &&																		// 2 < argement length
					(
						('"' == strOneArg.front() && strOneArg.find_first_of('"', 1) == (strOneArg.size() - 1))	||	// "..."
						('\'' == strOneArg.front() && strOneArg.find_first_of('\'', 1) == (strOneArg.size() - 1))	// '...'
					)
				){
					// This argument is enclosed in its entirety by "..." or '...', so cut both quote
					strOneArg = strOneArg.substr(1, strOneArg.size() - 2);
				}

				// Set an argument to command
				if(!strOneArg.empty()){
					const auto	first	= strOneArg.find_first_not_of(OPT_DELIMITERS);
					const auto	last	= strOneArg.find_last_not_of(OPT_DELIMITERS);
					if(std::string::npos != first){
						// add new argument in command afer trimming
						strOneArg = strOneArg.substr(first, (last - first + 1));
						if(!strOneArg.empty()){
							OneCommand.emplace_back(strOneArg);
						}
					}
					strOneArg.clear();
				}
			}
			if(!OneCommand.empty()){
				SecondResult.emplace_back(OneCommand);
				OneCommand.clear();
			}
		}
	}

	// Completed persing all arguments for command list.
	PromptArgs = std::move(SecondResult);

	return true;
}

//---------------------------------------------------------
// PromptCommandBase Methods : Parser
//---------------------------------------------------------
//
// Utility : Remove Comment
//
void PromptCommandBase::RemoveComment(std::string& oneLine)
{
	bool inSingleQuote	= false;
	bool inDoubleQuote	= false;
	bool isEscaped		= false;

	for(size_t pos = 0; pos < oneLine.size(); ++pos){
		char ch = oneLine[pos];
		if('\\' == ch){
			isEscaped = !isEscaped;
		}else if('\'' == ch){
			if(isEscaped){
				isEscaped = false;
			}else{
				if(!inDoubleQuote){
					inSingleQuote = !inSingleQuote;
				}
			}
		}else if('"' == ch){
			if(isEscaped){
				isEscaped = false;
			}else{
				if(!inSingleQuote){
					inDoubleQuote = !inDoubleQuote;
				}
			}
		}else if('#' == ch){
			isEscaped = false;
			if(!inSingleQuote && !inDoubleQuote){
				oneLine.erase(pos);
				break;
			}
		}else{
			isEscaped = false;
		}
	}
}

//
// Load and Parse Prompt Command List from file
//
// The Prompt Command file is read line by line and parsed.
// Each parsed line is stored sequentially in CmdList.
// In the Prompt Command file, any text following "#" is treated as a comment.
//
bool PromptCommandBase::LoadPromptCommandList(const std::string& strFile, PromptCommandList& PromptCommands)
{
	// Open promt command file
	std::ifstream	ifs(strFile);
	if(!ifs.is_open()){
		ERR("Could not open file(%s).", strFile.c_str());
		return false;
	}

	// Loop
	std::string	oneLine;
	while(std::getline(ifs, oneLine)){
		// Cut comment
		PromptCommandBase::RemoveComment(oneLine);

		// Trim
		const auto	first	= oneLine.find_first_not_of(OPT_DELIMITERS);
		const auto	last	= oneLine.find_last_not_of(OPT_DELIMITERS);
		if(std::string::npos != first){
			std::string	trimedLine = oneLine.substr(first, (last - first + 1));
			if(!trimedLine.empty()){
				// Parse one line
				std::vector<std::string>	Argvs;
				Argvs.emplace_back(trimedLine);
				if(!PromptCommandBase::MakePromptCommandList(Argvs, false, PromptCommands)){	// Parse a line and add commands to the PromptCommands.
					return false;
				}
			}
		}
	}

	if(PromptCommands.empty()){
		ERR("%s prompt command file does not have any command line.", strFile.c_str());
		return false;
	}

	// Check all Prompt Command List
	if(!CheckPromptCommandList(false, PromptCommands)){
		return false;
	}
	return true;
}

//
// Make Prompt Command Object List from Arguments
//
// [NOTE]
// If specify comand line arguments, set and pass the "argc" and "argv" after "--" option, or strings
// which is read from "run" command line option.
//
bool PromptCommandBase::MakePromptCommandList(const std::vector<std::string>& Argvs, bool isdirect, PromptCommandList& PromptCommands)
{
	PromptCommandArgs	PromptArgs;
	if(!PromptCommandBase::ParsePromptArgs(Argvs, PromptArgs)){
		ERR("Invalid command arguments(%s), so cannot be parsed.", CombineString(Argvs).c_str());
		return false;
	}

	// Parse options
	for(auto citer = PromptArgs.begin(); citer != PromptArgs.end(); ++citer){
		const PromptArgList&	arglist = *citer;
		if(arglist.empty()){
			// why?
			continue;
		}

		const OptionDefinition*	pOption	= FindOption(arglist[0], PromptOptionList);
		if(!pOption){
			// unknown prompt command
			ERR("Unknown prompt command: \"%s\"", CombineString(arglist).c_str());
			return false;
		}

		// Check option conditions
		if(isdirect && !pOption->direct_command){
			ERR("Command \"%s\" cannot be specified directly from the command line: \"%s\"", arglist[0].c_str(), CombineString(arglist).c_str());
			return false;
		}
		if(arglist.size() < static_cast<size_t>(pOption->min_param_cnt + 1) || static_cast<size_t>(pOption->max_param_cnt + 1) < arglist.size()){
			ERR("The number of parameters for command \"%s\" is incorrect: \"%s\"", arglist[0].c_str(), CombineString(arglist).c_str());
			return false;
		}

		switch(pOption->id){
			case	CommandId::CMDID_PROMPT_HELP:
				{
					CommandId	id = CommandId::CMDID_UNKNOWN;
					if(static_cast<size_t>(pOption->max_param_cnt + 1) == arglist.size()){
						id = FindCommandId(arglist[1], PromptOptionList);
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandHelp>(arglist, id));
				}
				break;

			case	CommandId::CMDID_PROMPT_QUIT:
				PromptCommands.emplace_back(std::make_unique<PromptCommandQuit>(arglist));
				break;

			case	CommandId::CMDID_PROMPT_SHELL:
				PromptCommands.emplace_back(std::make_unique<PromptCommandShell>(arglist));
				break;

			case	CommandId::CMDID_PROMPT_ECHO:
				PromptCommands.emplace_back(std::make_unique<PromptCommandEcho>(arglist, PromptArgList(arglist.begin() + 1, arglist.end())));
				break;

			case	CommandId::CMDID_PROMPT_SLEEP:
				{
					int	sec = 0;
					if(!StringToInt(arglist[1], sec)){
						ERR("Fisrt parameter(%s) for Command \"%s\" is not decimal integer", arglist[1].c_str(), arglist[0].c_str());
						return false;
					}else if(sec <= 0 || (24 * 60 * 60) < sec){
						ERR("Fisrt parameter(%s) for Command \"%s\" is not positive integer or over 24H(24*60*60).", arglist[1].c_str(), arglist[0].c_str());
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandSleep>(arglist, sec));
				}
				break;

			case	CommandId::CMDID_PROMPT_RUN:
				PromptCommands.emplace_back(std::make_unique<PromptCommandRun>(arglist, arglist[1]));
				break;

			case	CommandId::CMDID_PROMPT_BREAK:
				PromptCommands.emplace_back(std::make_unique<PromptCommandBreak>(arglist, true));
				break;

			case	CommandId::CMDID_PROMPT_NOBREAK:
				PromptCommands.emplace_back(std::make_unique<PromptCommandBreak>(arglist, false));
				break;

			case	CommandId::CMDID_PROMPT_HISTORYMAX:
				{
					int	max = 0;
					if(!StringToInt(arglist[1], max)){
						ERR("Fisrt parameter(%s) for Command \"%s\" is not decimal integer", arglist[1].c_str(), arglist[0].c_str());
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandHistoryMax>(arglist, max));
				}
				break;

			case	CommandId::CMDID_PROMPT_HISTORY:
				PromptCommands.emplace_back(std::make_unique<PromptCommandHistory>(arglist));
				break;

			case	CommandId::CMDID_PROMPT_SAVEHIS:
				PromptCommands.emplace_back(std::make_unique<PromptCommandSaveHistory>(arglist, arglist[1]));
				break;

			case	CommandId::CMDID_PROMPT_LIBVER:
				PromptCommands.emplace_back(std::make_unique<PromptCommandLibVersion>(arglist));
				break;

			case	CommandId::CMDID_PROMPT_LAPTIME:
				{
					// parse parameters
					bool	toggle 		= true;
					bool	enable 		= false;
					bool	message 	= false;
					bool	isset_mode	= false;
					bool	isset_msg	= false;
					size_t	argcnt		= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::IsOnParameter(arglist[cnt])){
							if(isset_mode){
								ERR("Already specify \"on\" or \"off\" parameter.");
								return false;
							}
							toggle 		= false;
							enable 		= true;
							isset_mode	= true;

						}else if(PromptCommandBase::IsOffParameter(arglist[cnt])){
							if(isset_mode){
								ERR("Already specify \"on\" or \"off\" parameter.");
								return false;
							}
							toggle 		= false;
							enable 		= false;
							isset_mode	= true;

						}else if(PromptCommandBase::IsMessageParameter(arglist[cnt])){
							if(isset_msg){
								ERR("Already specify \"message\"(msg, m) parameter.");
								return false;
							}
							message 	= true;
							isset_msg	= true;

						}else{
							// unknown argument
							ERR("Unknown parameter(%s). Specify either \"on\", \"off\" or \"messgae\"(msg, m) for this command.", arglist[1].c_str());
							return false;
						}
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandLapTime>(arglist, toggle, enable, message));
				}
				break;

			case	CommandId::CMDID_PROMPT_INFO:
				{
					bool	pm = true;
					if(PromptCommandBase::IsProjectionMatrixParameter(arglist[1])){
						pm = true;
					}else if(PromptCommandBase::IsEmbeddingDataParameter(arglist[1])){
						pm = false;
					}else{
						ERR("Fisrt parameter(%s) for Command \"%s\" is unknown, it must be \"pm\"(ProjectionMmatrix, Projection, Matrix) or \"em\"(EmbeddingData, Embedding).", arglist[1].c_str(), arglist[0].c_str());
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandInfo>(arglist, pm));
				}
				break;

			case	CommandId::CMDID_PROMPT_DUMP:
				{
					// parse parameters
					bool		pm			= true;
					std::string	strOutputFile("");
					bool		isset_pm	= false;
					bool		isset_output= false;
					size_t		argcnt		= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::IsProjectionMatrixParameter(arglist[cnt])){
							if(isset_pm){
								ERR("Already specify \"pm\"(ProjectionMmatrix, Projection, Matrix) or \"em\"(EmbeddingData, Embedding) parameter.");
								return false;
							}
							pm			= true;
							isset_pm	= true;

						}else if(PromptCommandBase::IsEmbeddingDataParameter(arglist[cnt])){
							if(isset_pm){
								ERR("Already specify \"pm\"(ProjectionMmatrix, Projection, Matrix) or \"em\"(EmbeddingData, Embedding) parameter.");
								return false;
							}
							pm			= false;
							isset_pm	= true;

						}else if(PromptCommandBase::GetOutputfileParameter(arglist[cnt], strOutputFile)){
							if(isset_output){
								ERR("Already specify \"output\"(output_file, outputfile, ofile, of) parameter.");
								return false;
							}
							isset_output = true;

						}else{
							// unknown argument
							ERR("Unknown parameter(%s). Specify either \"pm\"(ProjectionMmatrix, Projection, Matrix), \"em\"(EmbeddingData, Embedding) or \"output\"(output_file, outputfile, ofile, of) for this command.", arglist[1].c_str());
							return false;
						}
					}
					if(!isset_pm){
						ERR("\"pm\"(ProjectionMmatrix, Projection, Matrix) or \"em\"(EmbeddingData, Embedding) paramter muest be specified.");
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandDump>(arglist, pm, strOutputFile));
				}
				break;

			case	CommandId::CMDID_PROMPT_DBG:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// print information
						PromptCommands.emplace_back(std::make_unique<PromptCommandDebugLevel>(arglist));

					}else{																					// parameter: one...
						// parse parameters
						K2hvecDbgMode	level		= K2hvecDbgMode::VECDBG_SILENT;
						bool			with_k2hash	= false;
						//bool			isset_level	= false;
						bool			isset_wk2h	= false;
						size_t		argcnt		= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::IsWithK2hashParameter(arglist[cnt])){
								if(isset_wk2h){
									ERR("Already specify \"with_k2hash\"(k2hash, k2h, wk2h) parameter.");
									return false;
								}
								with_k2hash	= true;
								isset_wk2h	= true;

							}else{
								if(IsDbgModeString(arglist[cnt], K2hvecDbgMode::VECDBG_SILENT)){
									level = K2hvecDbgMode::VECDBG_SILENT;
								}else if(IsDbgModeString(arglist[cnt], K2hvecDbgMode::VECDBG_ERR)){
									level = K2hvecDbgMode::VECDBG_ERR;
								}else if(IsDbgModeString(arglist[cnt], K2hvecDbgMode::VECDBG_WARN)){
									level = K2hvecDbgMode::VECDBG_WARN;
								}else if(IsDbgModeString(arglist[cnt], K2hvecDbgMode::VECDBG_MSG)){
									level = K2hvecDbgMode::VECDBG_MSG;
								}else if(IsDbgModeString(arglist[cnt], K2hvecDbgMode::VECDBG_DUMP)){
									level = K2hvecDbgMode::VECDBG_DUMP;
								}else{
									ERR("Unknown parameter(%s). Specify K2hVec debug level(silent(slt), error(err), warning(warn, wan), message(msg, information, info), dump(dmp)), or with_k2hash(k2hash, k2h, wk2h) for this command.", arglist[cnt].c_str());
									return false;
								}
								//isset_level = true;
							}
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandDebugLevel>(arglist, level, with_k2hash));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_DBGFILE:
				{
					// parse parameters
					std::string		strFile("");
					bool			with_k2hash	= false;
					bool			isset_file	= false;
					bool			isset_wk2h	= false;
					size_t		argcnt		= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::IsWithK2hashParameter(arglist[cnt])){
							if(isset_wk2h){
								ERR("Already specify \"with_k2hash\"(k2hash, k2h, wk2h) parameter.");
								return false;
							}
							with_k2hash	= true;
							isset_wk2h	= true;

						}else{
							if(isset_file){
								ERR("Already specify \"log file\" parameter.");
								return false;
							}
							strFile		= arglist[cnt];
							isset_file	= true;
						}
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandDebugFile>(arglist, strFile, with_k2hash));
				}
				break;

			case	CommandId::CMDID_PROMPT_LASTSTATUS:
				{
					// parse parameters
					bool			IsLong		= false;
					bool			isset_long	= false;
					size_t		argcnt		= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::IsLongParameter(arglist[cnt])){
							if(isset_long){
								ERR("Already specify \"long\"(l) parameter.");
								return false;
							}
							IsLong		= true;
							isset_long	= true;

						}else{
							ERR("Unknown parameter(%s). Specify \"long\"(l) for this command.", arglist[1].c_str());
							return false;
						}
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandLastStatus>(arglist, IsLong));
				}
				break;

			case	CommandId::CMDID_PROMPT_COMATTR:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// print all common attributes
						PromptCommands.emplace_back(std::make_unique<PromptCommandCommonAttribute>(arglist));

					}else{																					// parameter: one or two
						// parse parameters
						time_t		expire		= 0;
						std::string	passphrase("");
						bool		isset_expire= false;
						bool		isset_pass	= false;
						size_t		argcnt		= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetExpireParameter(arglist[cnt], expire)){
								if(isset_expire){
									ERR("Already specify \"expire\"(exp, e) parameter.");
									return false;
								}
								isset_expire = true;

							}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], passphrase)){
								if(isset_pass){
									ERR("Already specify \"pass\"(passphrase, p) parameter.");
									return false;
								}
								isset_pass = true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"expire\"(exp, e) or \"pass\"(passphrase, p) for this command.", arglist[1].c_str());
								return false;
							}
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandCommonAttribute>(arglist, isset_expire, expire, isset_pass, passphrase));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_COMMON_CONF:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// print information
						PromptCommands.emplace_back(std::make_unique<PromptCommandCommonConfig>(arglist));
					}else{																					// parameter: one...
						// parse parameters
						int		bitsize				= -1;
						size_t	neighborkeylimit	= 0;
						size_t	directlimit			= 0;
						bool	isset_bitsize		= false;
						bool	isset_nkeylimit		= false;
						bool	isset_directlimit	= false;
						size_t	argcnt				= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetBinaryHashBitSizeParameter(arglist[cnt], bitsize)){
								if(isset_bitsize){
									ERR("Already specify binary hash bit count parameter.");
									return false;
								}
								isset_bitsize = true;

							}else if(PromptCommandBase::GetNeighborKeyLimitParameter(arglist[cnt], neighborkeylimit)){
								if(isset_nkeylimit){
									ERR("Already specify \"neighbor_key_limit\"(neighbor_limit, neighbor_key, key_limit, nkl, kl) parameter.");
									return false;
								}
								isset_nkeylimit = true;

							}else if(PromptCommandBase::GetContentDirectLimitParameter(arglist[cnt], directlimit)){
								if(isset_directlimit){
									ERR("Already specify \"content_direct_limit\"(direct_limit, content_direct, content_limit, cdl, dl) parameter.");
									return false;
								}
								isset_directlimit = true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify decimal string or \"default\" for this command.", arglist[1].c_str());
								return false;
							}
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandCommonConfig>(arglist, isset_bitsize, bitsize, isset_nkeylimit, neighborkeylimit, isset_directlimit, directlimit));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_CREATEK2HKEY:
				{
					std::string	input("");
					if(static_cast<size_t>(pOption->max_param_cnt + 1) == arglist.size()){
						input = arglist[1];
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandK2hKey>(arglist, input));
				}
				break;

			case	CommandId::CMDID_PROMPT_SET:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// set Preloaded embedding data
						PromptCommands.emplace_back(std::make_unique<PromptCommandSet>(arglist));

					}else{																					// parameter: one or two
						// parse parameters
						time_t		expire		= 0;
						std::string	passphrase("");
						std::string	strEmFile("");
						std::string	strValueFile("");
						std::string	strValue("");
						bool		isset_expire	= false;
						bool		isset_pass		= false;
						bool		isset_emfile	= false;
						bool		isset_valfile	= false;
						bool		isset_value		= false;
						size_t		argcnt		= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetExpireParameter(arglist[cnt], expire)){
								if(isset_expire){
									ERR("Already specify \"expire\"(exp, e) parameter.");
									return false;
								}
								isset_expire = true;

							}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], passphrase)){
								if(isset_pass){
									ERR("Already specify \"pass\"(passphrase, p) parameter.");
									return false;
								}
								isset_pass = true;

							}else if(PromptCommandBase::GetEmfileParameter(arglist[cnt], strEmFile)){
								if(isset_emfile){
									ERR("Already specify \"emfile\"(embedding file) parameter.");
									return false;
								}
								isset_emfile = true;

							}else if(PromptCommandBase::GetValuefileParameter(arglist[cnt], strValueFile)){
								if(isset_valfile || isset_value){
									ERR("Already specify \"valuefile\" or \"value\" parameter.");
									return false;
								}
								isset_valfile = true;

							}else if(PromptCommandBase::GetValueParameter(arglist[cnt], strValue)){
								if(isset_valfile || isset_value){
									ERR("Already specify \"valuefile\" or \"value\" parameter.");
									return false;
								}
								isset_value = true;
							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"expire\"(exp, e), \"pass\"(passphrase, p), \"emfile\"(emf), \"valuefile\"(vf) or \"value\"(v) for this command.", arglist[1].c_str());
								return false;
							}
						}
						// Check and set value as string
						if(isset_valfile){
							// read all text from file
							std::ifstream ifs(strEmFile, std::ios::binary);		// binary mode
							if(!ifs.is_open()){
								ERR("Could not open value file(%s).", strEmFile.c_str());
								return false;
							}
							std::ostringstream	ss;
							ss << ifs.rdbuf();
							strValue = ss.str();
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandSet>(arglist, isset_expire, isset_pass, expire, passphrase, strEmFile, strValue));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_GET:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// get Preloaded embedding data
						PromptCommands.emplace_back(std::make_unique<PromptCommandGet>(arglist));

					}else{																					// parameter: one or two
						// parse parameters
						std::string	strKey("");
						bool		usePreloadedEm	= false;
						std::string	strEmFile("");
						bool		isNoAttr		= false;
						std::string	strPass("");
						std::string	strOutputFile("");
						bool		isset_key		= false;
						bool		isset_noattr	= false;
						bool		isset_pass		= false;
						bool		isset_file		= false;
						size_t		argcnt			= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetK2HashKeyParameter(arglist[cnt], strKey, usePreloadedEm, strEmFile)){
								if(isset_key){
									ERR("Already specify \"key\"(k) or \"input\"(i) parameter.");
									return false;
								}
								isset_key = true;

							}else if(PromptCommandBase::GetNoAttributeCheckParameter(arglist[cnt], isNoAttr)){
								if(isset_noattr){
									ERR("Already specify \"noattrcheck\"(noattr, noa) parameter.");
									return false;
								}
								isset_noattr = true;

							}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], strPass)){
								if(isset_pass){
									ERR("Already specify \"pass\"(passphrase, p) parameter.");
									return false;
								}
								isset_pass = true;

							}else if(PromptCommandBase::GetOutputfileParameter(arglist[cnt], strOutputFile)){
								if(isset_file){
									ERR("Already specify \"output\"(output_file, outputfile, ofile, of) parameter.");
									return false;
								}
								isset_file		= true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"key\"(k), \"emfile\"(emf), \"noattrcheck\"(noattr, noa), \"pass\"(passphrase, p) or \"output\"(ooutputfile, of) for this command.", arglist[1].c_str());
								return false;
							}
						}
						if(isset_noattr && isNoAttr && isset_pass && !strPass.empty()){
							ERR("If \"noattrcheck\" (noattr, noa) is specified, \"pass\"(passphrase, p) cannot be specified, or can only be set to \"unset\"(no).");
							return false;
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandGet>(arglist, isset_key, strKey, usePreloadedEm, strEmFile, isNoAttr, isset_pass, strPass, strOutputFile));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_REMOVE:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// remove Preloaded embedding data
						PromptCommands.emplace_back(std::make_unique<PromptCommandRemove>(arglist));

					}else{																					// parameter: one or two
						// parse parameters
						std::string	strKey("");
						bool		usePreloadedEm	= false;
						std::string	strEmFile("");
						bool		isMainOnly		= false;
						bool		isset_key		= false;
						bool		isset_only		= false;
						size_t		argcnt			= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetK2HashKeyParameter(arglist[cnt], strKey, usePreloadedEm, strEmFile)){
								if(isset_key){
									ERR("Already specify \"key\"(k) or \"input\"(i) parameter.");
									return false;
								}
								isset_key = true;

							}else if(PromptCommandBase::GetMainkeyOnlyCheckParameter(arglist[cnt], isMainOnly)){
								if(isset_only){
									ERR("Already specify \"mainkey-only\"(mainkey, only) parameter.");
									return false;
								}
								isset_only = true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"key\"(k), \"emfile\"(emf) or \"mainkey-only\"(mainkey, only) for this command.", arglist[1].c_str());
								return false;
							}
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandRemove>(arglist, isset_key, strKey, usePreloadedEm, strEmFile, isMainOnly));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_READSUMMARY:
				{
					// parse parameters
					std::string	strKey("");
					bool		isNoAttr	= false;
					std::string	strPass("");
					bool		isset_key	= false;
					bool		isset_noattr= false;
					bool		isset_pass	= false;
					size_t		argcnt		= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::GetNoAttributeCheckParameter(arglist[cnt], isNoAttr)){
							if(isset_noattr){
								ERR("Already specify \"noattrcheck\"(noattr, noa) parameter.");
								return false;
							}
							isset_noattr = true;

						}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], strPass)){
							if(isset_pass){
								ERR("Already specify \"pass\"(passphrase, p) parameter.");
								return false;
							}
							isset_pass = true;

						}else{
							if(isset_key){
								ERR("Already specify \"<k2hash key>\" parameter.");
								return false;
							}
							strKey		= arglist[cnt];
							isset_key	= true;
						}
					}
					if(!isset_key){
						ERR("The \"<k2hash key>\" parameter must be specified.");
						return false;
					}
					if(isset_noattr && isNoAttr && isset_pass && !strPass.empty()){
						ERR("If \"noattrcheck\" (noattr, noa) is specified, \"pass\"(passphrase, p) cannot be specified, or can only be set to \"unset\"(no).");
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandReadSummary>(arglist, strKey, isNoAttr, isset_pass, strPass));
				}
				break;

			case	CommandId::CMDID_PROMPT_SEARCH:
				{
					// parse parameters
					std::string	strScore("");
					float		Score			= 0.0f;
					bool		isNoAttr		= false;
					std::string	strPass("");
					bool		usePreloadedEm	= false;
					std::string	strEmFile("");
					std::string	strOutputFile("");
					bool		isset_score		= false;
					bool		isset_noattr	= false;
					bool		isset_pass		= false;
					bool		isset_em		= false;
					bool		isset_output	= false;
					size_t		argcnt			= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::GetScoreParameter(arglist[cnt], strScore)){
							if(isset_score){
								ERR("Already specify \"score\"(s) parameter.");
								return false;
							}
							if(!PromptCommandBase::CvtScoreStringToFloat(strScore, Score)){
								ERR("The \"score\"(s) parameter value(%s), it must be float value string and \"1.0f\" as upper limit.", strScore.c_str());
								return false;
							}
							isset_score = true;

						}else if(PromptCommandBase::GetNoAttributeCheckParameter(arglist[cnt], isNoAttr)){
							if(isset_noattr){
								ERR("Already specify \"noattrcheck\"(noattr, noa) parameter.");
								return false;
							}
							isset_noattr = true;

						}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], strPass)){
							if(isset_pass){
								ERR("Already specify \"pass\"(passphrase, p) parameter.");
								return false;
							}
							isset_pass = true;

						}else if(PromptCommandBase::GetEmbeddingParameter(arglist[cnt], usePreloadedEm, strEmFile)){
							if(isset_em){
								ERR("Already specify \"key=em\"(EmbeddingData, Embedding) or \"emfile\"(emf) parameter.");
								return false;
							}
							isset_em = true;

						}else if(PromptCommandBase::GetOutputfileParameter(arglist[cnt], strOutputFile)){
							if(isset_output){
								ERR("Already specify \"output\"(output_file, outputfile, ofile, of) parameter.");
								return false;
							}
							isset_output = true;

						}else{
							// unknown argument
							ERR("Unknown parameter(%s). Specify either \"key=em\"(EmbeddingData, Embedding) or \"output\"(output_file, outputfile, ofile, of) for this command.", arglist[1].c_str());
							return false;
						}
					}
					if(isset_noattr && isNoAttr && isset_pass && !strPass.empty()){
						ERR("If \"noattrcheck\" (noattr, noa) is specified, \"pass\"(passphrase, p) cannot be specified, or can only be set to \"unset\"(no).");
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandSearch>(arglist, isset_score, Score, strEmFile, strOutputFile, isNoAttr, isset_pass, strPass));
				}
				break;

			case	CommandId::CMDID_PROMPT_LOADPM:
				PromptCommands.emplace_back(std::make_unique<PromptCommandLoadProjection>(arglist, arglist[1]));
				break;

			case	CommandId::CMDID_PROMPT_LOADEM:
				PromptCommands.emplace_back(std::make_unique<PromptCommandLoadEmbedding>(arglist, arglist[1]));
				break;

			case	CommandId::CMDID_PROMPT_CREATEPM:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// create with default and set as Preloaded data
						PromptCommands.emplace_back(std::make_unique<PromptCommandCreateProjection>(arglist, 0, 0, "", "", ""));

					}else{
						// parse parameters
						int			nDimension	= 0;
						int			nBitsize	= 0;
						std::string	strPmName;
						std::string	strPmVer;
						std::string	strPmFile;
						bool		isset_dim	= false;
						bool		isset_bs	= false;
						bool		isset_name	= false;
						bool		isset_ver	= false;
						bool		isset_file	= false;
						size_t		argcnt		= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetDimensionParameter(arglist[cnt], nDimension)){
								if(isset_dim){
									ERR("Already specify \"dimension\"(dim, d) parameter.");
									return false;
								}
								isset_dim = true;

							}else if(PromptCommandBase::GetBitSizeParameter(arglist[cnt], nBitsize)){
								if(isset_bs){
									ERR("Already specify \"bitsize\"(bit_size, bit_count, bitcount, bit_cnt, bitcnt, bit, b) parameter.");
									return false;
								}
								isset_bs = true;

							}else if(PromptCommandBase::GetNameParameter(arglist[cnt], strPmName)){
								if(isset_name){
									ERR("Already specify \"name\"(n) parameter.");
									return false;
								}
								isset_name = true;

							}else if(PromptCommandBase::GetVersionParameter(arglist[cnt], strPmVer)){
								if(isset_ver){
									ERR("Already specify \"version\"(ver, v) parameter.");
									return false;
								}
								isset_ver = true;

							}else if(PromptCommandBase::GetOutputfileParameter(arglist[cnt], strPmFile)){
								if(isset_file){
									ERR("Already specify \"output\"(output_file, outputfile, ofile, of) parameter.");
									return false;
								}
								isset_file	= true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"dimension\"(dim, d), \"bitsize\"(bit_size, bit_count, bitcount, bit_cnt, bitcnt, bit, b), \"name\"(n), \"version\"(ver, v) or \"output\"(output_file, outputfile, ofile, of) for this command.", arglist[1].c_str());
								return false;
							}
						}
						// check
						if(!isset_file){
							if(isset_name){
								ERR("The \"name\"(n) parameter can be specified only when \"output=<output file>\" is specified.");
								return false;
							}
							if(isset_ver){
								ERR("The \"version\"(ver, v) parameter can be specified only when \"output=<output file>\" is specified.");
								return false;
							}
						}

						PromptCommands.emplace_back(std::make_unique<PromptCommandCreateProjection>(arglist, nDimension, nBitsize, strPmFile, strPmName, strPmVer));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_CREATEEM:
				{
					if(static_cast<size_t>(pOption->min_param_cnt + 1) == arglist.size()){					// parameter: <no>
						// create with default and set as Preloaded data
						PromptCommands.emplace_back(std::make_unique<PromptCommandCreateEmbedding>(arglist, 0, ""));

					}else{
						// parse parameters
						int			nDimension	= 0;
						std::string	strEmFile;
						bool		isset_dim	= false;
						bool		isset_file	= false;
						size_t		argcnt		= arglist.size() - pOption->min_param_cnt;
						for(size_t cnt = 1; cnt < argcnt; ++cnt){
							if(PromptCommandBase::GetDimensionParameter(arglist[cnt], nDimension)){
								if(isset_dim){
									ERR("Already specify \"dimension\"(dim, d) parameter.");
									return false;
								}
								isset_dim = true;

							}else if(PromptCommandBase::GetOutputfileParameter(arglist[cnt], strEmFile)){
								if(isset_file){
									ERR("Already specify \"output\"(output_file, outputfile, ofile, of) parameter.");
									return false;
								}
								isset_file	= true;

							}else{
								// unknown argument
								ERR("Unknown parameter(%s). Specify either \"dimension\"(dim, d) or \"output\"(output_file, outputfile, ofile, of) for this command.", arglist[1].c_str());
								return false;
							}
						}
						PromptCommands.emplace_back(std::make_unique<PromptCommandCreateEmbedding>(arglist, nDimension, strEmFile));
					}
				}
				break;

			case	CommandId::CMDID_PROMPT_CALBH:
				{
					// parse parameters
					std::string	strEmFile("");
					std::string	strPmFile("");
					bool		usePreloadedEm	= false;
					bool		isset_em		= false;
					bool		isset_pm		= false;
					size_t		argcnt			= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::GetEmbeddingParameter(arglist[cnt], usePreloadedEm, strEmFile)){
							if(isset_em){
								ERR("Already specify \"key=em\"(EmbeddingData, Embedding) or \"emfile\"(emf) parameter.");
								return false;
							}
							isset_em = true;

						}else if(PromptCommandBase::GetPmfileParameter(arglist[cnt], strPmFile)){
							if(isset_pm){
								ERR("Already specify \"pmfile\"(pmff) parameter.");
								return false;
							}
							isset_pm = true;

						}else{
							// unknown argument
							ERR("Unknown parameter(%s). Specify either \"key=em\"(EmbeddingData, Embedding), \"emfile\"(emf) or \"pmfile\"(pmff) for this command.", arglist[1].c_str());
							return false;
						}
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandCalBinaryHash>(arglist, strEmFile, strPmFile));
				}
				break;

			case	CommandId::CMDID_PROMPT_CALCOS:
				{
					// parse parameters
					std::string	strK2hKey("");
					bool		isNoAttr		= false;
					std::string	strPass("");
					std::string	strEmFile("");
					bool		usePreloadedEm	= false;
					bool		isSimple		= false;
					bool		isset_key		= false;
					bool		isset_noattr	= false;
					bool		isset_pass		= false;
					bool		isset_em		= false;
					bool		isset_spl		= false;
					size_t		argcnt			= arglist.size();
					for(size_t cnt = 1; cnt < argcnt; ++cnt){
						if(PromptCommandBase::GetNoAttributeCheckParameter(arglist[cnt], isNoAttr)){
							if(isset_noattr){
								ERR("Already specify \"noattrcheck\"(noattr, noa) parameter.");
								return false;
							}
							isset_noattr = true;

						}else if(PromptCommandBase::GetPassphraseParameter(arglist[cnt], strPass)){
							if(isset_pass){
								ERR("Already specify \"pass\"(passphrase, p) parameter.");
								return false;
							}
							isset_pass = true;

						}else if(PromptCommandBase::GetEmbeddingParameter(arglist[cnt], usePreloadedEm, strEmFile)){
							if(isset_em){
								ERR("Already specify \"key=em\"(EmbeddingData, Embedding) or \"emfile\"(emf) parameter.");
								return false;
							}
							isset_em = true;

						}else if(PromptCommandBase::IsSimpleParameter(arglist[cnt])){
							if(isset_spl){
								ERR("Already specify \"simple\"(spl) parameter.");
								return false;
							}
							isSimple	= true;
							isset_spl	= true;

						}else{
							if(isset_key){
								ERR("Already specify \"<k2hash key>\" parameter.");
								return false;
							}
							strK2hKey = arglist[cnt];
							isset_key = true;
						}
					}
					// check
					if(!isset_key){
						ERR("The \"<k2hash key>\" argument must be specified.");
						return false;
					}
					if(isset_noattr && isNoAttr && isset_pass && !strPass.empty()){
						ERR("If \"noattrcheck\" (noattr, noa) is specified, \"pass\"(passphrase, p) cannot be specified, or can only be set to \"unset\"(no).");
						return false;
					}
					PromptCommands.emplace_back(std::make_unique<PromptCommandCalCosineSimilarity>(arglist, strK2hKey, isNoAttr, isset_pass, strPass, strEmFile, isSimple));
				}
				break;

			default:
				ERR("Unknown prompt command/option (command id=%d is specified.", static_cast<int>(pOption->id));
				return false;
		}
	}

	// Check all Prompt Command List
	if(!CheckPromptCommandList(isdirect, PromptCommands)){
		return false;
	}
	return true;
}

//
// Check prompt command list
//
bool PromptCommandBase::CheckPromptCommandList(bool isdirect, const PromptCommandList& PromptCommands)
{
	bool	foundQuit = false;
	for(const auto& pCmdObj: PromptCommands){
		if(!pCmdObj){
			ERR("Internal error: Found empty command object in list.");
			return false;
		}
		if(CommandId::CMDID_UNKNOWN == pCmdObj->GetCommandId()){
			ERR("Internal error: Found unknown command object in list.");
			return false;
		}
		if(foundQuit){
			ERR("There are commands following the \"quit(q, exit)\" command. If you use the \"quit(q, exit)\" command, please place it at the end.");
			return false;
		}
		if(CommandId::CMDID_PROMPT_QUIT == pCmdObj->GetCommandId()){
			foundQuit = true;
		}
	}
	if(isdirect && PromptCommands.empty()){
		ERR("There are no prompt commands after \"--\" option.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// PromptCommandBase Methods : Execute
//---------------------------------------------------------
std::string PromptCommandBase::GetCommandName() const
{
	return FindCommandName(command_id, PromptOptionList);
}

std::string PromptCommandBase::GetCommandLineArgs() const
{
	return CombineString(command_args);
}

bool PromptCommandBase::Execute(bool& isTerminate)
{
	LapTime	duration(true);

	// Execute
	bool	result = RawExecute(isTerminate);

	// Print laptime
	if(PromptCommandBase::useLapTime){
		std::string	strDuration = duration.GetDurationString();
		PRN("[Command : %s]: Execution time - %s\n", GetCommandName().c_str(), strDuration.c_str());
	}

	return result;
}

//---------------------------------------------------------
// Inherit Classes Variables
//---------------------------------------------------------
//
// PromptCommandRun
//
int	PromptCommandRun::CurrentNestDepth = 0;

//---------------------------------------------------------
// Inherit Classes Methods : Execute
//---------------------------------------------------------
bool PromptCommandHelp::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	switch(CmdId){
		case	CommandId::CMDID_UNKNOWN:
			PromptCommandBase::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_HELP:
			PromptCommandHelp::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_QUIT:
			PromptCommandQuit::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_SHELL:
			PromptCommandShell::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_ECHO:
			PromptCommandEcho::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_SLEEP:
			PromptCommandSleep::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_RUN:
			PromptCommandRun::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_BREAK:
			PromptCommandBreak::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_NOBREAK:
			PromptCommandBreak::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_HISTORYMAX:
			PromptCommandHistoryMax::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_HISTORY:
			PromptCommandHistory::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_SAVEHIS:
			PromptCommandSaveHistory::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_LIBVER:
			PromptCommandLibVersion::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_LAPTIME:
			PromptCommandLapTime::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_INFO:
			PromptCommandInfo::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_DUMP:
			PromptCommandDump::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_DBG:
			PromptCommandDebugLevel::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_DBGFILE:
			PromptCommandDebugFile::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_LASTSTATUS:
			PromptCommandLastStatus::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_COMATTR:
			PromptCommandCommonAttribute::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_COMMON_CONF:
			PromptCommandCommonConfig::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_CREATEK2HKEY:
			PromptCommandK2hKey::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_SET:
			PromptCommandSet::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_GET:
			PromptCommandGet::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_REMOVE:
			PromptCommandRemove::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_READSUMMARY:
			PromptCommandReadSummary::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_SEARCH:
			PromptCommandSearch::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_LOADPM:
			PromptCommandLoadProjection::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_LOADEM:
			PromptCommandLoadEmbedding::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_CREATEPM:
			PromptCommandCreateProjection::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_CREATEEM:
			PromptCommandCreateEmbedding::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_CALBH:
			PromptCommandCalBinaryHash::PrintHelp();
			break;
		case	CommandId::CMDID_PROMPT_CALCOS:
			PromptCommandCalCosineSimilarity::PrintHelp();
			break;
		default:
			ERR("Unknown command id(%d).", static_cast<int>(CmdId));
			return false;
	}
	return true;
}

bool PromptCommandQuit::RawExecute(bool& isTerminate)
{
	isTerminate = true;

	return true;
}

bool PromptCommandShell::RawExecute(bool& isTerminate)
{
	static constexpr const char* pDefaultShell = "/bin/sh";		// default

	isTerminate = false;

	// Check if shell is available on the system
	if(0 == ::system(nullptr)){
		ERR("Could not find a subshell to launch.");
		return false;
	}

	// Check emvronment
	const char*	pShell = ::std::getenv("SHELL");
	if(!pShell){
		pShell = pDefaultShell;
	}

	// Execute shell
	const int result = ::system(pShell);
	if(-1 == result){
		ERR("Failed to execute shell(%s) : errno(%d).", pShell, errno);
		return false;
	}

	return true;
}

bool PromptCommandEcho::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	PRN("%s", CombineString(printStrings).c_str());

	return true;
}

bool PromptCommandSleep::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(0 < second){
		sleep(std::min(second, (24 * 60 * 60)));	// maxmum 24H
	}

	return true;
}

bool PromptCommandRun::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// Check nest limit
	if(PromptCommandRun::RUNCMD_NEST_DEPTH_LIMIT <= PromptCommandRun::CurrentNestDepth){
		ERR("Reached maximum \"run\" command nesting depth(%d)", PromptCommandRun::RUNCMD_NEST_DEPTH_LIMIT);
		return false;
	}

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// Read command file
	std::ifstream	ifs(strFile);
	if(!ifs.is_open()){
		ERR("Could not open file: %s", strFile.c_str());
		return false;
	}
	std::vector<std::string>	cmdlines;
	std::string					line;
	while(std::getline(ifs, line)){
		// check ignore line
		auto	first_pos = line.find_first_not_of(OPT_DELIMITERS);
		if(std::string::npos == first_pos){
			// empty line
			continue;
		}
		if('#' == line[first_pos]){
			// comment line
			continue;
		}
		// set command line arguments
		line += ";";
		cmdlines.emplace_back(std::move(line));
	}
	ifs.close();

	// Make prompt commands
	PromptCommandList	RunPromptCommands;
	if(!PromptCommandBase::MakePromptCommandList(cmdlines, false, RunPromptCommands)){
		// Something error
		return false;
	}

	// Execute commands
	PRN("[START] Run %s command file", strFile.c_str());
	++(PromptCommandRun::CurrentNestDepth);

	for(const auto& pPromptCmdObj: RunPromptCommands){
		// Execute one command
		if(!pPromptCmdObj->Execute(isTerminate)){
			// failed but continue...
			ERR("Failed to run \"%s\" command: \"%s\"", pPromptCmdObj->GetCommandName().c_str(), pPromptCmdObj->GetCommandLineArgs().c_str());
			pK2hvecCmdObj->SetLastCommandStatus(false);
		}else{
			pK2hvecCmdObj->SetLastCommandStatus(true);
		}

		// Check break for teminate
		if(isTerminate){
			break;
		}
	}

	--(PromptCommandRun::CurrentNestDepth);
	PRN("[FINISH] Run %s command file", strFile.c_str());

	return true;
}

bool PromptCommandBreak::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}
	if(CommandId::CMDID_PROMPT_BREAK == command_id){
		pK2hvecCmdObj->SetErrorBreakMode(true);
	}else if(CommandId::CMDID_PROMPT_NOBREAK == command_id){
		pK2hvecCmdObj->SetErrorBreakMode(false);
	}else{
		ERR("Internal error: Command id is unknown.");
		return false;
	}
	std::string	strMode = pK2hvecCmdObj->GetErrorBreakMode() ? "break" : "no break";
	PRNSUCCESS("Set prompt command break mode to \"%s\"", strMode.c_str());

	return true;
}

bool PromptCommandHistoryMax::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pConsoleObj){
		ERR("Internal error: console input object is null.");
		return false;
	}
	PromptCommandBase::pConsoleObj->SetMax(history_max);

	PRNSUCCESS("Set history max: %lld", PromptCommandBase::pConsoleObj->GetMax());

	return true;
}

bool PromptCommandHistory::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pConsoleObj){
		ERR("Internal error: console input object is null.");
		return false;
	}
	PromptCommandBase::pConsoleObj->PrintAllHistory();

	return true;
}

bool PromptCommandSaveHistory::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pConsoleObj){
		ERR("Internal error: console input object is null.");
		return false;
	}

	// lastest history is this command line, thus it shoud not be put to file.
	if(!PromptCommandBase::pConsoleObj->PutAllHistory(strFile, false)){
		return false;
	}

	PRNSUCCESS("Save history to file: %s", strFile.c_str());

	return true;
}

bool PromptCommandLibVersion::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	k2hvec_print_version(stdout);

	return true;
}

bool PromptCommandLapTime::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(isToggle){
		PromptCommandBase::ToggleLapTime();
	}else if(toEnable){
		PromptCommandBase::EnableLapTime();
	}else{
		PromptCommandBase::DisableLapTime();
	}
	if(isMeaage){
		PRNSUCCESS("Set lap time to \"%s\"", PromptCommandBase::useLapTime ? "ON" : "OFF");
	}
	return true;
}

bool PromptCommandInfo::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(isPM){
		// Print information for Projection Matrix
		std::string	strName		= ProjectionMatrix::GetProjectionMatrixName();
		std::string strVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
		int 		nBitsize	= ProjectionMatrix::GetProjectionMatrixBitsize();
		int			nDimension	= ProjectionMatrix::GetProjectionMatrixDimension();
		std::string	strChecksum	= ProjectionMatrix::GetChecksumString();
		std::string	str2ndHash	= ProjectionMatrix::GetSecondHashString();

		PRN("Projection Matrix information:");
		PRN("    Name            : %s",		strName.c_str());
		PRN("    Version         : %s",		strVersion.c_str());
		PRN("    Dimension size  : %d",		nDimension);
		PRN("    Bit size(count) : %d bit",	nBitsize);
		PRN("    Checksum        : %s",		strChecksum.c_str());
		PRN("    FNV-1a hash     : %s",		str2ndHash.c_str());

	}else{
		// Print information for Embedding
		if(!PromptCommandBase::pK2hvecCmdObj){
			ERR("Internal error: k2hveccommand object is null.");
			return false;
		}

		int	dimension = pK2hvecCmdObj->GetPreloadedEmbeddingDimension();
		if(dimension <= 0){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
		std::string	strChecksum	= pK2hvecCmdObj->GetPreloadedEmbeddingChecksum();
		std::string	str2ndHash	= pK2hvecCmdObj->GetPreloadedEmbeddingSecondHash();

		PRN("Preloaded Embedding data information:");
		PRN("    Dimension size  : %d", dimension);
		PRN("    Checksum        : %s", strChecksum.c_str());
		PRN("    FNV-1a hash     : %s",	str2ndHash.c_str());
	}

	return true;
}

bool PromptCommandDump::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(isPM){
		if(strFile.empty()){
			// Dump Projection Matrix
			std::string	strPmName		= ProjectionMatrix::GetProjectionMatrixName();
			std::string strPmVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
			int 		PmBitsize		= ProjectionMatrix::GetProjectionMatrixBitsize();
			int			PmDimension		= ProjectionMatrix::GetProjectionMatrixDimension();
			std::string	strPmChecksum	= ProjectionMatrix::GetChecksumString();
			std::string	str2ndHash		= ProjectionMatrix::GetSecondHashString();
			std::string	strPmData		= MakeStringFromFloatArray(ProjectionMatrix::GetProjectionMatrix(), "    ");

			PRN("Projection Matrix summary:");
			PRN("    Name            : %s",		strPmName.c_str());
			PRN("    Version         : %s",		strPmVersion.c_str());
			PRN("    Dimension size  : %d",		PmDimension);
			PRN("    Bit size(count) : %d bit",	PmBitsize);
			PRN("    Checksum        : %s",		strPmChecksum.c_str());
			PRN("    FNV-1a hash     : %s",		str2ndHash.c_str());
			PRN("");
			PRN("Projection Matrix data = %s", strPmData.c_str());

		}else{
			// Put Projection Matrix to file
			if(!ProjectionMatrix::ExportProjectionMatrixFileAsCArray(strFile.c_str(), false)){
				ERR("Failed to output Projection Matrix to file(%s).", strFile.c_str());
				return false;
			}
			PRNSUCCESS("Output Projection Matrix to file(%s).", strFile.c_str());
		}
	}else{
		if(!PromptCommandBase::pK2hvecCmdObj){
			ERR("Internal error: k2hveccommand object is null.");
			return false;
		}

		int	dimension = pK2hvecCmdObj->GetPreloadedEmbeddingDimension();
		if(dimension <= 0){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}

		if(strFile.empty()){
			// Dump Embedding data
			std::string	strChecksum	= pK2hvecCmdObj->GetPreloadedEmbeddingChecksum();
			std::string	str2ndHash	= pK2hvecCmdObj->GetPreloadedEmbeddingSecondHash();
			std::string	strEmData	= MakeStringFromFloatArray(pK2hvecCmdObj->GetPreloadedEmbedding(), "    ");

			PRN("Preloaded Embedding data summary:");
			PRN("    Dimension size  : %d", dimension);
			PRN("    Checksum        : %s", strChecksum.c_str());
			PRN("    FNV-1a hash     : %s",	str2ndHash.c_str());
			PRN("");
			PRN("Preloaded Embedding data = %s", strEmData.c_str());

		}else{
			// Put Embedding data to file
			if(!CreateEmbeddingFile(strFile, pK2hvecCmdObj->GetPreloadedEmbedding(), EMBEDDING_ARRAY_NAME)){
				ERR("Failed to output Preloaded Embedding data to file(%s).", strFile.c_str());
				return false;
			}
			PRNSUCCESS("Ooutput Preloaded Embedding data to file(%s).", strFile.c_str());
		}
	}

	return true;
}

bool PromptCommandDebugLevel::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	std::string	strPrefix;
	if(!displayMode){
		K2hvecCommand::SetupDebugLevel(dbglevel, withk2hash);
		if(withk2hash){
			strPrefix = "Set debug level to K2hVec and K2Hash: ";
		}else{
			strPrefix = "Set debug level to K2hVec: ";
		}
	}

	std::string	strInfo = K2hvecCommand::GetDebugLevelInformation();
	PRNSUCCESS("%s%s", strPrefix.c_str(), strInfo.c_str());

	return true;
}

bool PromptCommandDebugFile::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!K2hvecCommand::SetupDebugFile(strFile, withk2hash)){
		if(strFile.empty()){
			ERR("Filed to unset debug log file.");
		}else{
			ERR("Filed to set debug log file(%s).", strFile.c_str());
		}
		return false;
	}

	// Print message
	std::string	strTarget = "K2hVec";
	if(withk2hash){
		strTarget += " and K2Hash";
	}
	if(!strFile.empty()){
		PRNSUCCESS("Set debug log for %s to %s file.", strTarget.c_str(), strFile.c_str());
	}else{
		PRNSUCCESS("Reset debug log for %s to stdout.", strTarget.c_str(), strFile.c_str());
	}

	return true;
}

bool PromptCommandLastStatus::RawExecute(bool& isTerminate)
{
	static const char* CDEF = ::isatty(STDOUT_FILENO) ? "\033[0m"  : "";		// To default

	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}
	if(pK2hvecCmdObj->GetLastCommandStatus()){
		static const char* CGRN = ::isatty(STDOUT_FILENO) ? "\033[32m" : "";	// Color Green
		if(IsLong){
			PRN("Last command result : %sSucceed%s(0)", CGRN, CDEF);
		}else{
			PRN("%s0%s", CGRN, CDEF);
		}
	}else{
		static const char* CRED = ::isatty(STDOUT_FILENO) ? "\033[31m" : "";	// Color Red
		if(IsLong){
			PRN("Last command result : %sFailed%s(1)", CRED, CDEF);
		}else{
			PRN("%s1%s", CRED, CDEF);
		}
	}
	return true;
}

bool PromptCommandCommonAttribute::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(isPrint){
		PRN("Current Common attributes:");
	}else{
		// set/unset attributes
		if(isExpire){
			if(0 == Expire){
				K2hVecDb::ResetExpire();
			}else{
				if(!K2hVecDb::SetExpire(Expire)){
					ERR("Failed to set expire period time(%lld sec).", static_cast<long long>(Expire));
					return false;
				}
			}
		}
		if(isPass){
			if(strPass.empty()){
				K2hVecDb::ResetCryptPass();
			}else{
				K2hVecDb::SetCryptPass(strPass);
			}
		}
		PRNSUCCESS("Set Common attributes:");
	}

	// print attributes
	time_t		expire_time	= K2hVecDb::GetExpire();
	std::string	pass		= K2hVecDb::GetCryptPass();

	if(K2hVecDb::DEFAULT_NOTSET_EXPIRE == expire_time){
		PRN("    Expire period time: not set");
	}else{
		PRN("    Expire period time: %lld sec", static_cast<long long>(expire_time));
	}
	if(pass.empty()){
		PRN("    Passphrase:         not set");
	}else{
		PRN("    Passphrase:         %s", pass.c_str());
	}

	return true;
}

bool PromptCommandCommonConfig::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	if(isPrint){
		PRN("Common configuration(Binary hash / Index keys / Direct Content Limits):");
	}else{
		// set/unset attributes
		if(isBitSize){
			if(0 == nBitSize){
				K2hVecDb::ResetBinHashBitSize();
			}else if(0 < nBitSize){
				if(!K2hVecDb::SetBinHashSize(nBitSize)){
					ERR("Failed to set binary hash bit size(%d bit).", nBitSize);
					return false;
				}
			}
		}

		if(isNeighborKeyLimit){
			if(0 == NeighborKeyLimit){
				K2hVecDb::ResetNeighborKeysLimit();
			}else{
				if(!K2hVecDb::SetNeighborKeysLimit(NeighborKeyLimit)){
					ERR("Failed to set neighbor key upper limit(%zu).", NeighborKeyLimit);
					return false;
				}
			}
		}

		if(isDirectLimit){
			if(0 == DirectLimit){
				K2hVecDb::ResetContentDirectLimit();
			}else{
				if(!K2hVecDb::SetContentDirectLimit(DirectLimit)){
					ERR("Failed to set upper limit for saving content text directly to the main key(%zu).", DirectLimit);
					return false;
				}
			}
		}
		PRNSUCCESS("Set Common configuration(Binary hash / Index keys / Direct Content Limits):");
	}

	// print information
	size_t	cur_direct_limit = K2hVecDb::GetContentDirectLimit();

	PRN("    Binary Hash bit size                : %d bit",		K2hVecDb::GetBinHashSize());
	PRN("    Search neighbor key upper limit     : %zu",		K2hVecDb::GetNeighborKeysLimit());

	if(K2hVecDb::CONTENT_ONLY_DIRECT == cur_direct_limit){
		PRN("    Saving content directly upper limit : no limit");
	}else{
		PRN("    Saving content directly upper limit : %zu",	cur_direct_limit);
	}

	return true;
}

bool PromptCommandK2hKey::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	std::vector<float>	embedding;
	if(strFile.empty()){
		// Preloaded Embedding data
		if(!PromptCommandBase::pK2hvecCmdObj){
			ERR("Internal error: k2hveccommand object is null.");
			return false;
		}

		embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
		if(embedding.empty()){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
	}else{
		// Embedding data file
		if(!LoadEmbeddingFile(strFile, EMBEDDING_ARRAY_NAME, embedding)){
			ERR("Failed to load embedding data from file: %s", strFile.c_str());
			return false;
		}
		if(embedding.empty()){
			ERR("The embedding data loaded from file(%s) is empty.", strFile.c_str());
			return false;
		}
	}

	// Get K2hash key and index keys
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, embedding)){
		ERR("Failed to generate k2hash key.");
		return false;
	}

	// Print
	PRNSUCCESS("Generate k2hash key:");
	PRN("    Embedding data type    : %s%s", (strFile.empty() ? "Preloaded" : "File - "), strFile.c_str());
	PRN("    K2hash main key        : %s", k2hkey.c_str());

	return true;
}

bool PromptCommandSet::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// passphrase
	const std::string*	pPassphrase = nullptr;
	if(isPass){
		if(!strPass.empty()){
			pPassphrase = &strPass;
		}
	}

	// expire option
	time_t*	pExpire	= nullptr;
	if(isExpire){
		if(0 != Expire){
			pExpire	= &Expire;
		}
	}

	// embedding data
	std::vector<float>	embedding;
	if(strEmFile.empty()){
		// Preloaded Embedding data
		embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
		if(embedding.empty()){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
	}else{
		// Embedding data file
		if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
			ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
			return false;
		}
		if(embedding.empty()){
			ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
			return false;
		}
	}

	// Get k2hash keys
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, embedding)){
		ERR("Failed to generate k2hash key.");
		return false;
	}

	// Set embedding data to k2hash
	if(!pK2hvecCmdObj->Set(embedding, strValue, nullptr, nullptr, pPassphrase, pExpire)){
		ERR("Failed to set embedding data to k2hash: key=%s", k2hkey.c_str());
		return false;
	}

	PRNSUCCESS("Set embedding data to k2hash");
	if(pExpire && 0 != *pExpire){
		PRN("    Expire period time : %lld sec", *pExpire);
	}else{
		PRN("    Expire period time : not set");
	}
	if(pPassphrase && !pPassphrase->empty()){
		PRN("    Passphrase         : %s", pPassphrase->c_str());
	}else{
		PRN("    Passphrase         : not set");
	}
	PRN("    K2hash main key    : %s", k2hkey.c_str());

	return true;
}

bool PromptCommandGet::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// passphrase
	const std::string*	pPassphrase = nullptr;
	if(isPass){
		if(!strPass.empty()){
			pPassphrase = &strPass;
		}
	}

	// Get content text
	std::string	content;
	std::string	strK2hKey;
	if(!strKey.empty()){
		// specify k2hash key
		if(!pK2hvecCmdObj->Get(strKey, content, !isNoAttr, pPassphrase)){
			ERR("The value for the %s key was not found in K2Hash.", strKey.c_str());
			return false;
		}
		strK2hKey = strKey;

	}else{
		// set embedding data from Preloaded or file
		std::vector<float>	embedding;
		if(usePreloadedEm){
			// Preloaded Embedding data
			embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
			if(embedding.empty()){
				ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
				return false;
			}

		}else if(!strEmFile.empty()){
			// Embedding data file
			if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
				ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
				return false;
			}
			if(embedding.empty()){
				ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
				return false;
			}
		}else{
			ERR("Not found the K2Hash key name/type in parameter.");
			return false;
		}

		// Get k2hash keys (for only print information)
		if(!K2hVecDb::GetK2hashKey(strK2hKey, embedding)){
			ERR("Failed to generate k2hash key.");
			return false;
		}

		// Get K2Hash data
		if(!pK2hvecCmdObj->Get(embedding, content, nullptr, nullptr, !isNoAttr, pPassphrase)){
			ERR("The value for the embedding data(key=%s) was not found in K2Hash.", strK2hKey.c_str());
			return false;
		}
	}

	// Put file
	if(!strOutputFile.empty()){
		std::ofstream	ofs(strOutputFile, std::ios::out | std::ios::binary);
		if(!ofs.is_open()){
			ERR("Could not create/open file(%s).", strOutputFile.c_str());
			return false;
		}

		ofs << content;
		if(!ofs.good()){
			ERR("Failed to write the value to file(%s).", strOutputFile.c_str());
			return false;
		}
	}

	// Summary
	size_t		ContentLen	= content.size();
	uint64_t	ContentHash	= CalFnvA1Hash(content.c_str());
	if(36 < ContentLen){
		content.resize(36);
		content += "...";
	}
	PRNSUCCESS("Got %s value:",						strK2hKey.c_str());
	PRN("    Value length         : %ld",			ContentLen);
	PRN("    Value FNV-1a hash    : 0x%016" PRIX64,	ContentHash);
	PRN("    Value Content prefix : %s",			content.empty() ? "<empty>" : content.c_str());

	return true;
}

bool PromptCommandRemove::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// Remove
	if(!strKey.empty()){
		// specify k2hash key
		if(!pK2hvecCmdObj->Remove(strKey, isMainKeyOnly)){
			return false;
		}
	}else{
		// set embedding data from Preloaded or file
		std::vector<float>	embedding;
		if(usePreloadedEm){
			// Preloaded Embedding data
			embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
			if(embedding.empty()){
				ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
				return false;
			}

		}else if(!strEmFile.empty()){
			// Embedding data file
			if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
				ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
				return false;
			}
			if(embedding.empty()){
				ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
				return false;
			}
		}else{
			ERR("Not found the K2Hash key name/type in parameter.");
			return false;
		}

		// Get k2hash key (for only print information)
		if(!K2hVecDb::GetK2hashKey(strKey, embedding)){
			ERR("Failed to generate k2hash key.");
			return false;
		}

		// Remove keys
		if(!pK2hvecCmdObj->Remove(embedding, isMainKeyOnly, nullptr, nullptr)){
			return false;
		}
	}

	// Print messages
	if(isMainKeyOnly){
		PRNSUCCESS("Remove only key(not remove index keys): %s", strKey.c_str());
	}else{
		std::string	strNeighborKey;
		if(K2hVecValue::DeriveMainNeighborKey(strKey, strNeighborKey)){
			PRNSUCCESS("Remove main key and neighbor key: %s / %s", strKey.c_str(), strNeighborKey.c_str());
		}else{
			PRNSUCCESS("Remove main key and neighbor key: %s / (not found)", strKey.c_str());
		}
	}

	return true;
}

bool PromptCommandReadSummary::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// passphrase
	const std::string*	pPassphrase = nullptr;
	if(isPass){
		if(!strPass.empty()){
			pPassphrase = &strPass;
		}
	}

	// get value
	std::vector<float>	embedding;
	std::vector<float>	sysembedding;
	std::string			content;
	std::string			embeddingmodel;
	if(!pK2hvecCmdObj->Get(strK2hKey, embedding, sysembedding, content, embeddingmodel, !isNoAttr, pPassphrase)){
		ERR("The value for the %s key was not found in K2Hash.", strK2hKey.c_str());
		return false;
	}

	// Summary
	PRN("Found the \"%s\" key in K2Hash:",	strK2hKey.c_str());
	PRN("    K2hash main key             : %s",	strK2hKey.c_str());

	std::string	strNeighborKey;
	if(!K2hVecValue::DeriveMainNeighborKey(strK2hKey, strNeighborKey)){
		PRN("    K2hash main neighbor key    : <something error in paesing main key>");
	}else{
		PRN("    K2hash main neighbor key    : %s", strNeighborKey.c_str());
	}

	std::string	strEmCS		= GetChecksumString(embedding);
	std::string	strEmHash	= MakeFlatStringFromUint64(CalSecondHash(embedding), false);
	std::string	strSysEmCS	= GetChecksumString(sysembedding);
	std::string	strSysEmHash= MakeFlatStringFromUint64(CalSecondHash(sysembedding), false);

	PRN("    Embedding        count      : %lld",	embedding.size());
	PRN("                     checksum   : %s",		strEmCS.c_str());
	PRN("                     2nd hash   : %s",		strEmHash.c_str());
	PRN("    System Embedding count      : %lld",	sysembedding.size());
	PRN("                     checksum   : %s",		strSysEmCS.c_str());
	PRN("                     2nd hash   : %s",		strSysEmHash.c_str());
	PRN("    Embedding model             : %s",		embeddingmodel.c_str());

	size_t		ContentLen	= content.size();
	uint64_t	ContentHash	= CalFnvA1Hash(content.c_str());
	if(36 < ContentLen){
		content.resize(36);
		content += "...";
	}
	PRN("    Value length                : %ld",			ContentLen);
	PRN("    Value FNV-1a hash           : 0x%016" PRIX64,	ContentHash);
	PRN("    Value Content prefix        : %s",				content.empty() ? "<empty>" : content.c_str());

	return true;
}

bool PromptCommandSearch::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// set embedding data from Preloaded or file
	std::vector<float>	embedding;
	if(strEmFile.empty()){
		// Preloaded Embedding data
		embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
		if(embedding.empty()){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
	}else{
		// Embedding data file
		if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
			ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
			return false;
		}
		if(embedding.empty()){
			ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
			return false;
		}
	}

	// passphrase
	const std::string*	pPassphrase = nullptr;
	if(isPass){
		if(!strPass.empty()){
			pPassphrase = &strPass;
		}
	}

	// Score
	float	set_score = (isScore ? Score : K2hVecDb::USE_COMMON_SCORE_LIMIT);

	// Print information for embedding data
	std::string	strChecksum	= GetChecksumString(embedding);
	std::string	str2ndHash	= MakeFlatStringFromUint64(CalSecondHash(embedding), false);
	std::string	strSetScore	= MakeFlatStringFromFloat(set_score);

	PRN("Search Embedding data:");
	if(strEmFile.empty()){
		PRN("    Data source               : Preloaded embedding data");
	}else{
		PRN("    Data source               : File=%s", strEmFile.c_str());
	}
	PRN("    Dimension size            : %lld",	embedding.size());
	PRN("    Checksum                  : %s",	strChecksum.c_str());
	PRN("    FNV-1a hash               : %s",	str2ndHash.c_str());
	PRN("Search Score Limit            : %s",	strSetScore.c_str());
	PRN("");

	// Search
	std::string			found_key;
	std::vector<float>	found_em;
	std::vector<float>	found_sysem;
	std::string			found_content;
	std::string			found_model;
	if(!pK2hvecCmdObj->Search(embedding, found_key, found_em, found_sysem, found_content, found_model, set_score, nullptr, nullptr, !isNoAttr, pPassphrase)){
		PRN("-> Search Result              : Not found");
		return false;
	}

	// Found: Calacurate Cosine Similarity
	std::string	strCosResult = MakeFlatStringFromFloat(CosineSimilarity(found_em, embedding));

	if(!strOutputFile.empty()){
		// Put Embedding data to file
		if(!CreateEmbeddingFile(strOutputFile, found_em, EMBEDDING_ARRAY_NAME)){
			ERR("Failed to output found Embedding data to file(%s).", strOutputFile.c_str());
			return false;
		}
	}

	// Print result
	std::string	strEmCS		= GetChecksumString(found_em);
	std::string	strEmHash	= MakeFlatStringFromUint64(CalSecondHash(found_em), false);
	size_t		ContentLen	= found_content.size();
	uint64_t	ContentHash	= CalFnvA1Hash(found_content.c_str());
	std::string	ContentText;
	if(36 < ContentLen){
		ContentText = found_content.substr(0, 36);
		ContentText += "...";
	}else{
		ContentText	= found_content;
	}

	PRN("-> Search Result              : Found");
	PRN("    Cosine Similarity value   : %s",			strCosResult.c_str());
	if(!strOutputFile.empty()){
		PRN("    Saved embedding data      : %s",		strOutputFile.c_str());
	}
	PRN("");
	PRN("Found embedding data information:");
	PRN("    K2hash main key           : %s",			found_key.c_str());
	PRN("    Embedding        count    : %lld",			found_em.size());
	PRN("                     checksum : %s",			strEmCS.c_str());
	PRN("                     2nd hash : %s",			strEmHash.c_str());
	PRN("    Embedding model           : %s",			found_model.c_str());
	PRN("    Value length              : %ld",			ContentLen);
	PRN("          FNV-1a hash         : 0x%016" PRIX64,ContentHash);
	PRN("          Content prefix      : %s",			ContentText.empty() ? "<empty>" : ContentText.c_str());

	return true;
}

bool PromptCommandLoadProjection::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// Load and Set Projection Matrix
	if(!ProjectionMatrix::LoadCustomProjectionMatrix(strPmFile.c_str())){
		ERR("Failed to load Projection Matrix file: %s", strPmFile.c_str());
		return false;
	}

	// Print information for Projection Matrix
	std::string	strName		= ProjectionMatrix::GetProjectionMatrixName();
	std::string strVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
	int 		nBitsize	= ProjectionMatrix::GetProjectionMatrixBitsize();
	int			nDimension	= ProjectionMatrix::GetProjectionMatrixDimension();
	std::string	strChecksum	= ProjectionMatrix::GetChecksumString();
	std::string	str2ndHash	= ProjectionMatrix::GetSecondHashString();

	PRNSUCCESS("Load Projection Matrix in K2HVEC library.");
	PRN("    File path       : %s",		strPmFile.c_str());
	PRN("    Name            : %s",		strName.c_str());
	PRN("    Version         : %s",		strVersion.c_str());
	PRN("    Dimension size  : %d",		nDimension);
	PRN("    Bit size(count) : %d bit",	nBitsize);
	PRN("    Checksum        : %s",		strChecksum.c_str());
	PRN("    FNV-1a hash     : %s",		str2ndHash.c_str());

	return true;
}

bool PromptCommandLoadEmbedding::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// Load embedding data from file
	std::vector<float>	embedding;
	if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
		ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
		return false;
	}

	// Set Preloaded embedding data
	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}
	if(!pK2hvecCmdObj->SetPreloadedEmbedding(embedding)){
		ERR("Failed to load and set embedding data from file as Preloaded embedding data.");
		return false;
	}

	// Print information for Preloaded embedding data
	int			dimension	= pK2hvecCmdObj->GetPreloadedEmbeddingDimension();
	std::string	strChecksum	= pK2hvecCmdObj->GetPreloadedEmbeddingChecksum();
	std::string	str2ndHash	= pK2hvecCmdObj->GetPreloadedEmbeddingSecondHash();

	PRNSUCCESS("Load Preloaded embedding data.");
	PRN("    File path       : %s", strEmFile.c_str());
	PRN("    Dimension size  : %d", dimension);
	PRN("    Checksum        : %s", strChecksum.c_str());
	PRN("    FNV-1a hash     : %s",	str2ndHash.c_str());

	return true;
}

bool PromptCommandCreateProjection::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// Parameters for generating Projection Matrix
	std::string	strFile = strOutputFile;
	if(strFile.empty()){
		// Temporary file path
		strFile = K2hvecCommand::MakeTemporaryFilepath("pmtemp", "pm");
	}
	if(0 == dimension){
		dimension = DEFAULT_DIMENSION;
	}
	if(0 == bitsize){
		bitsize = DEFAULT_BITSIZE;
	}
	if(strName.empty()){
		strName = DEFAULT_CUSTOM_NAME;
	}
	if(strVersion.empty()){
		strVersion = DEFAULT_VERSION;
	}

	// Create Projection Matrix file
	if(!ProjectionMatrix::CreateProjectionMatrixFileAsCArray(strFile.c_str(), bitsize, dimension, strName.c_str(), strVersion.c_str(), false)){
		ERR("Failed to generate Projection Matrix, and save it to file: %s", strFile.c_str());
		if(strFile != strOutputFile){
			if(0 != std::remove(strFile.c_str())){
				ERR("Failed to remove temporary file: %s", strFile.c_str());
			}
		}
		return false;
	}

	if(strFile == strOutputFile){
		// Print information for Projection Matrix file
		PRNSUCCESS("Generated Projection Matrix and save it.");
		PRN("    File path       : %s",		strFile.c_str());
		PRN("    Name            : %s",		strName.c_str());
		PRN("    Version         : %s",		strVersion.c_str());
		PRN("    Dimension size  : %d",		dimension);
		PRN("    Bit size(count) : %d bit",	bitsize);
	}else{
		// Load Projection Matrix temporary file
		if(!ProjectionMatrix::LoadCustomProjectionMatrix(strFile.c_str())){
			ERR("Failed to load generated Projection Matrix from temporary file: %s", strFile.c_str());
			return false;
		}
		if(0 != std::remove(strFile.c_str())){
			ERR("Failed to remove temporary file: %s", strFile.c_str());
		}

		// Print information for internal Projection Matrix
		std::string	strPmName		= ProjectionMatrix::GetProjectionMatrixName();
		std::string strPmVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
		int 		PmBitsize		= ProjectionMatrix::GetProjectionMatrixBitsize();
		int			PmDimension		= ProjectionMatrix::GetProjectionMatrixDimension();
		std::string	strPmChecksum	= ProjectionMatrix::GetChecksumString();
		std::string	strPm2ndHash	= ProjectionMatrix::GetSecondHashString();

		PRNSUCCESS("Generated and load Projection Matrix in K2HVEC library.");
		PRN("    Name            : %s",		strPmName.c_str());
		PRN("    Version         : %s",		strPmVersion.c_str());
		PRN("    Dimension size  : %d",		PmDimension);
		PRN("    Bit size(count) : %d bit",	PmBitsize);
		PRN("    Checksum        : %s",		strPmChecksum.c_str());
		PRN("    FNV-1a hash     : %s",		strPm2ndHash.c_str());
	}

	return true;
}

bool PromptCommandCreateEmbedding::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// Parameters for generating embedding data
	std::string	strFile = strOutputFile;
	if(strFile.empty()){
		// Temporary file path
		strFile = K2hvecCommand::MakeTemporaryFilepath("emtemp", "em");
	}
	if(0 == dimension){
		dimension = DEFAULT_DIMENSION;
	}

	// Generate embedding data
	if(strFile == strOutputFile){
		// Create embedding data file
		if(!GenerateEmbeddingFile(strFile, EMBEDDING_ARRAY_NAME, dimension)){
			ERR("Failed to create generate embedding data file: file=%s, dimension=%d", strFile.c_str(), dimension);
			return false;
		}

		// Print information for Embedding data file
		PRNSUCCESS("Generate Embedding data and save it.");
		PRN("    File path       : %s", strFile.c_str());
		PRN("    Array Name      : %s", EMBEDDING_ARRAY_NAME);
		PRN("    Dimension size  : %d", dimension);

	}else{
		// Generate Embedding data
		std::vector<float>	embedding = GenerateEmbedding(dimension);
		if(embedding.empty()){
			ERR("Failed to generate embedding data: dimension=%d", dimension);
			return false;
		}

		// Set Preloaded embedding data
		if(!PromptCommandBase::pK2hvecCmdObj){
			ERR("Internal error: k2hveccommand object is null.");
			return false;
		}
		if(!pK2hvecCmdObj->SetPreloadedEmbedding(embedding)){
			ERR("Failed to set generated embedding data as Preloaded embedding data.");
			return false;
		}

		// Print information for Preloaded embedding data
		int			nDimension	= pK2hvecCmdObj->GetPreloadedEmbeddingDimension();
		std::string	strChecksum	= pK2hvecCmdObj->GetPreloadedEmbeddingChecksum();
		std::string	str2ndHash	= pK2hvecCmdObj->GetPreloadedEmbeddingSecondHash();

		PRNSUCCESS("Generate and set it as Preloaded embedding data.");
		PRN("    Dimension size  : %d", nDimension);
		PRN("    Checksum        : %s",	strChecksum.c_str());
		PRN("    FNV-1a hash     : %s",	str2ndHash.c_str());
	}

	return true;
}

bool PromptCommandCalBinaryHash::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	// set embedding data from Preloaded or file
	std::vector<float>	embedding;
	if(strEmFile.empty()){
		// Preloaded Embedding data
		if(!PromptCommandBase::pK2hvecCmdObj){
			ERR("Internal error: k2hveccommand object is null.");
			return false;
		}
		embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
		if(embedding.empty()){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
	}else{
		// Embedding data file
		if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
			ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
			return false;
		}
		if(embedding.empty()){
			ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
			return false;
		}
	}

	// Load and Set Projection Matrix
	if(!strPmFile.empty()){
		if(!ProjectionMatrix::LoadCustomProjectionMatrix(strPmFile.c_str())){
			ERR("Failed to load Projection Matrix file: %s", strPmFile.c_str());
			return false;
		}
		PRN("Loaded Projection Matrix from %s file.", strPmFile.c_str());
	}

	// Print information for Projection Matrix
	PRN("Projection Matrix information:");
	PRN("    Name            : %s",		ProjectionMatrix::GetProjectionMatrixName().c_str());
	PRN("    Version         : %s",		ProjectionMatrix::GetProjectionMatrixVersion().c_str());
	PRN("    Dimension size  : %d",		ProjectionMatrix::GetProjectionMatrixDimension());
	PRN("    Bit size(count) : %d bit",	ProjectionMatrix::GetProjectionMatrixBitsize());
	PRN("    Checksum        : %s",		ProjectionMatrix::GetChecksumString().c_str());
	PRN("    FNV-1a hash     : %s",		ProjectionMatrix::GetSecondHashString().c_str());
	PRN("");

	// Print information for embedding data
	std::string	strChecksum	= GetChecksumString(embedding);
	std::string	str2ndHash	= MakeFlatStringFromUint64(CalSecondHash(embedding), false);

	PRN("Embedding data information:");
	if(strEmFile.empty()){
		PRN("    Data source     : Preloaded embedding data");
	}else{
		PRN("    Data source     : File=%s", strEmFile.c_str());
	}
	PRN("    Dimension size  : %lld",	embedding.size());
	PRN("    Checksum        : %s",		strChecksum.c_str());
	PRN("    FNV-1a hash     : %s",		str2ndHash.c_str());
	PRN("");

	// Calacuration
	std::string	strBinHash = MakeFlatStringFromUint8Array(ProjectionMatrix::GetBinaryHash(embedding), false);

	// Print result
	PRNSUCCESS("Calculate Binary hash:");
	PRN("    Binary hash     : %s", strBinHash.c_str());

	return true;
}

bool PromptCommandCalCosineSimilarity::RawExecute(bool& isTerminate)
{
	isTerminate = false;

	if(!PromptCommandBase::pK2hvecCmdObj){
		ERR("Internal error: k2hveccommand object is null.");
		return false;
	}

	// passphrase
	const std::string*	pPassphrase = nullptr;
	if(isPass){
		if(!strPass.empty()){
			pPassphrase = &strPass;
		}
	}

	// get embedding data in k2hash
	std::vector<float>	k2h_embedding;
	if(!pK2hvecCmdObj->Get(strK2hKey, k2h_embedding, !isNoAttr, pPassphrase)){
		ERR("The value for the %s key was not found in K2Hash.", strK2hKey.c_str());
		return false;
	}

	// set target embedding data from Preloaded or file
	std::vector<float>	embedding;
	if(strEmFile.empty()){
		// Preloaded Embedding data
		embedding = pK2hvecCmdObj->GetPreloadedEmbedding();			// copy
		if(embedding.empty()){
			ERR("The embedding data has not been preloaded, use \"load_embedding(em)\" or \"create_embedding(cem)\" to load the data.");
			return false;
		}
	}else{
		// Embedding data file
		if(!LoadEmbeddingFile(strEmFile, EMBEDDING_ARRAY_NAME, embedding)){
			ERR("Failed to load embedding data from file: %s", strEmFile.c_str());
			return false;
		}
		if(embedding.empty()){
			ERR("The embedding data loaded from file(%s) is empty.", strEmFile.c_str());
			return false;
		}
	}

	// Print information for k2hash embedding data
	std::string	strChecksum	= GetChecksumString(k2h_embedding);
	std::string	str2ndHash	= MakeFlatStringFromUint64(CalSecondHash(k2h_embedding), false);

	PRN("Embedding data in K2Hash:");
	PRN("    K2hash key        : %s",	strK2hKey.c_str());
	PRN("    Dimension size    : %lld",	k2h_embedding.size());
	PRN("    Checksum          : %s",	strChecksum.c_str());
	PRN("    FNV-1a hash       : %s",	str2ndHash.c_str());
	PRN("");

	// Print information for embedding data
	strChecksum	= GetChecksumString(embedding);
	str2ndHash	= MakeFlatStringFromUint64(CalSecondHash(embedding), false);

	PRN("Target Embedding data:");
	if(strEmFile.empty()){
		PRN("    Data source       : Preloaded embedding data");
	}else{
		PRN("    Data source       : File=%s", strEmFile.c_str());
	}
	PRN("    Dimension size    : %lld",	embedding.size());
	PRN("    Checksum          : %s",	strChecksum.c_str());
	PRN("    FNV-1a hash       : %s",	str2ndHash.c_str());
	PRN("");

	// Calacuration
	std::string	strCosResult = MakeFlatStringFromFloat(CosineSimilarity(k2h_embedding, embedding, isSimple));

	// Print result
	PRNSUCCESS("Calculate Cosine Similarity:");
	PRN("    Cosine Similarity : %s", isSimple ? "simple" : "standard");
	PRN("    Result            : %s", strCosResult.c_str());

	return true;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
