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

#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cctype>
#include <cstddef>
#include <strings.h>
#include <unistd.h>

#include <k2hash/k2hash.h>

#include "k2hvecmath.h"
#include "k2hvec_tools_common.h"
#include "k2hvec_cli_cmd.h"
#include "k2hvec.h"
#include "k2hvecutil.h"

//---------------------------------------------------------
// Common Functions
//---------------------------------------------------------
//
// Convert string to variable
//
bool StringToInt(const std::string& str, int& val)
{
	try{
		size_t	index	= 0;					// used charactor count
		val				= std::stoi(str, &index);
		if(index != str.length()){
			// If there are characters that cannot be converted, the number of
			// characters consumed will not match the total character count.
			//
			return false;
		}
	}catch(const std::invalid_argument&){
		return false;							// Contains no characters interpreted as a numeric
	}catch(const std::out_of_range&){
		return false;							// The value of an integer exceeds the limit
	}
	return true;
}

//
// Convert string to variable
//
std::string CombineString(const std::vector<std::string>& strlist)
{
	std::string	result;
	for(const auto& str: strlist){
		if(!result.empty()){
			result += ' ';
		}
		result += str;
	}
	return result;
}

//
// Find option
//
const OptionDefinition* FindOption(const std::string& param, const std::vector<OptionDefinition>& OptionList)
{
	for(const auto& optarray: OptionList){
		for(const auto& opt: optarray.options){
			// cppcheck-suppress useStlAlgorithm
			if(0 == strcasecmp(opt.c_str(), param.c_str())){
				return &optarray;
			}
		}
	}
	return nullptr;
}

std::string FindCommandName(CommandId cmdid, const std::vector<OptionDefinition>& OptionList)
{
	for(const auto& optarray: OptionList){
		if(optarray.id == cmdid){
			if(optarray.options.empty()){
				return "";
			}
			return optarray.options[0];
		}
	}
	return "";
}

CommandId FindCommandId(const std::string& param, const std::vector<OptionDefinition>& OptionList)
{
	const OptionDefinition*	pOption = FindOption(param, OptionList);
	if(!pOption){
		return CommandId::CMDID_UNKNOWN;
	}
	return pOption->id;
}

//---------------------------------------------------------
// K2hvecCommand Class variables
//---------------------------------------------------------
const char*	K2hvecCommand::OptDelimiter = " \t\n\r\f\v";

//
// Command options for command line
//
const std::vector<OptionDefinition> K2hvecCommand::ExecOptionList = {
	// common
	{CommandId::CMDID_EXEC_HELP,			{"--help",														"-h"},			0,	0,		false},

	// k2hash parameter
	{CommandId::CMDID_EXEC_FILE,			{"--file",														"-f"},			1,	1,		false},
	{CommandId::CMDID_EXEC_TEMPFILE,		{"--temporary_file",		"--temporary",		"--temp",		"-t"},			0,	0,		false},
	{CommandId::CMDID_EXEC_MEM,				{"--memory",				"--mem",							"-m"},			0,	0,		false},
	{CommandId::CMDID_EXEC_MASK,			{"--mask",														"-mask"},		1,	1,		false},
	{CommandId::CMDID_EXEC_CMASK,			{"--collision_mask",		"--cmask",							"-cmask"},		1,	1,		false},
	{CommandId::CMDID_EXEC_ELCNT,			{"--element_count",			"--elementcnt",						"-elcnt"},		1,	1,		false},
	{CommandId::CMDID_EXEC_PAGESIZE,		{"--page_size",				"--pagesize",		"-pagesize",	"-page"},		1,	1,		false},
	{CommandId::CMDID_EXEC_FULL,			{"--full_mapping",			"--fullmap",		"-fullmap",		"-full"},		0,	0,		false},
	{CommandId::CMDID_EXEC_RO,				{"--read_only",				"--read",			"-ro",			"-r"},			0,	0,		false},
	{CommandId::CMDID_EXEC_LIBVER,			{"--library_version",		"--libversion",						"-libver"},		0,	0,		false},

	// Binary hash / Index keys settings
	{CommandId::CMDID_EXEC_BH_BITSIZE,		{"--binary_hash_bit_size",	"--bh_bitsize",		"-bitsize",		"-bs"},			1,	1,		false},
	{CommandId::CMDID_EXEC_NEIGHBOR_LIMIT,	{"--neighbor_key_limit",	"--neighbor_limit",	"-neighbor"		"-nkl"},		1,	1,		false},
	{CommandId::CMDID_EXEC_DIRECT_LIMIT,	{"--content_direct_limit",	"--direct_limit",	"-directlimit"	"-cdl"},		1,	1,		false},

	// setting for command
	{CommandId::CMDID_EXEC_HISMAX,			{"--history_max",			"--history_count",	"--hismax",		"-hmax"},		1,	1,		false},
	{CommandId::CMDID_EXEC_DBGLEVEL,		{"--debug_level",			"--debug",			"-d",			"-g"},			1,	1,		false},
	{CommandId::CMDID_EXEC_DBGFILE,			{"--debug_logfile",			"--debug_file",		"-dlog",		"-glog"},		1,	1,		false},
	{CommandId::CMDID_EXEC_LAPTIME,			{"--lap_time",				"--lap",			"-lap",			"-time"},		0,	0,		false},
	{CommandId::CMDID_EXEC_NOWELCOM,		{"--no_welcome_message",	"--no-banner",						"-no_welcome"},	0,	0,		false},

	// command control
	{CommandId::CMDID_EXEC_RUN,				{"--run_file",				"--run",							"-run"},		1,	1,		false},
	{CommandId::CMDID_EXEC_PROMPT,			{"--"},																			1,	9999,	false}
};

//---------------------------------------------------------
// K2hvecCommand Class methods
//---------------------------------------------------------
//
// Print help
//
void K2hvecCommand::PrintHelp(const std::string& progname)
{
	static const char format[] =
		"\n"
		"Usage: %s [options]\n"
		"       %s [options] -- <prompt commands>\n"
		"\n"
		"Process Options:\n"
		"    --help(-h)                            Print help\n"
		"    --library_version(-libver)            Displays the library version\n"
		"    --history_max(-hmax) <count>          Set maximum count for prompt command history\n"
		"    --lap_time(-lap)                      Enable the display of the elapsed time for command execution\n"
		"    --no_welcome_message(-no_welcome)     Suppress startup message\n"
		"    --run_file(-run) <file path>          Load a file containing prompt commands and execute those commands\n"
		"    -- <prompt commands...>               Parameters following this option are executed as prompt commands\n"
		"\n"
		"K2hash/K2hvec Options:\n"
		"    --file(-f) <file path>                Specify the K2Hash file\n"
		"    --temporary_file(-t)                  Create a K2Hash using a temporary file and use it\n"
		"    --memory(-m)                          Create a K2Hash in memory and use it.\n"
		"    --mask(-msk) <bit count>              Specifies the number of mask bits when creating a K2Hash\n"
		"    --collision_mask(-cmask) <bit count>  Specifies the number of collision mask(cmask) bits when creating a K2Hash\n"
		"    --element_count(-elcnt) <count>       Specify the element count when creating a K2Hash\n"
		"    --page_size(-page) <size>             Specifies the page size when creating a K2Hash\n"
		"    --full_mapping(-full)                 K2Hash is fully mapping mode(in-memory K2Hash is enabled this as default. otherwise, the default is not)\n"
		"    --read_only(-r)                       K2Hash file is used in read-only mode\n"
		"    --debug_level(-d) <debug mode>        Specifies the debug level: SILENT(SLT), ERROR(ERR), WARNING(WAN), MESSAGE(MSG), or DUMP(DMP)\n"
		"    --debug_logfile(-dlog) <file path>    Specifies the file for debug log output(default is stdout)\n"
		"\n"
		"Common configuration(Binary hash / Index keys / Direct Content Limits):\n"
		"    --binary_hash_bit_size(-bs) <bit>     Specifies the number of bits for the binary hash calculation result\n"
		"    --neighbor_key_limit(-nkl) <limit>    Specifies the maximum number of neighbor keys allowed for vector similarity searches\n"
		"    --content_direct_limit(-cdl) <limit>  Specifies the maximum size for saving content text directly to the main key\n";

	fprintf(stdout, format, progname.c_str(), progname.c_str());

	PromptCommandBase::PrintHelp();
}

//
// Get K2hVec debug information
//
std::string K2hvecCommand::GetDebugLevelInformation()
{
	std::string	strInfo;

	const char*	pLevel		= GetK2hvecDbgModeString();
	std::string	strLevel	= pLevel ? pLevel : "unknown";
	strInfo					+= "Debug Level = " + strLevel;

	const char*	pFile		= GetK2hvecDbgFile();
	std::string	strFile		= pFile ? pFile : "stdout";
	strInfo					+= ", Debug log = " + strFile;

	return strInfo;
}

//
// Setup K2hVec and K2Hash debug level
//
void K2hvecCommand::SetupDebugLevel(const K2hvecDbgMode& dbglevel, bool k2hash)
{
	if(k2hash){
		SetK2hvecDbgModeWithK2hash(dbglevel);
	}else{
		SetK2hvecDbgMode(dbglevel);
	}
}

//
// Setup K2hVec and K2Hash debug log file
//
bool K2hvecCommand::SetupDebugFile(const std::string& strFile, bool k2hash)
{
	// Set debug log file
	if(!strFile.empty()){
		if(k2hash){
			if(!SetK2hvecDbgFileWithK2hash(strFile.c_str())){
				ERR("Failed to set debug log file(%s) for k2hvec/k2hash library, but continue...", strFile.c_str());
				return false;
			}
		}else{
			if(!SetK2hvecDbgFile(strFile.c_str())){
				ERR("Failed to set debug log file(%s) for k2hvec library, but continue...", strFile.c_str());
				return false;
			}
		}
	}else{
		if(k2hash){
			if(!UnsetK2hvecDbgFileWithK2hash()){
				ERR("Failed to unset debug log file for k2hvec/k2hash library, but continue...");
				return false;
			}
		}else{
			if(!UnsetK2hvecDbgFile()){
				ERR("Failed to unset debug log file for k2hvec library, but continue...");
				return false;
			}
		}
	}
	return true;
}

//
// Make temporary filepath
//
std::string K2hvecCommand::MakeTemporaryFilepath(const std::string& prefix, const std::string& suffix)
{
	// Get temporary directory path
	const char*	envTmpDir = std::getenv("TMPDIR");
	if(VECEMPTYSTR(envTmpDir)){
		envTmpDir = std::getenv("TMP");
		if(VECEMPTYSTR(envTmpDir)){
			envTmpDir = std::getenv("TEMP");
			if(VECEMPTYSTR(envTmpDir)){
				envTmpDir = "/tmp";
			}
		}
	}
	std::string	strTmpDir = envTmpDir;
	if(strTmpDir.empty() || '/' != strTmpDir.back()){
		strTmpDir += '/';
	}

	// Make temporary file path ( /<dir>/<prefix>_XXXXXX<suffix> )
	std::string	randNumber;
	{
		std::random_device						rd;
		std::mt19937							gen(rd());
		std::uniform_int_distribution<uint32_t>	dis(0, 0xFFFFFF);
		std::ostringstream						oss;

		oss << std::hex << std::setw(6) << std::setfill('0') << dis(gen);
		randNumber = oss.str();
	}
    std::string	tmpFilepath	= strTmpDir + "." + prefix + "_" + randNumber + suffix;

    return tmpFilepath;
}

std::string K2hvecCommand::MakeK2HashTemporaryFilepath(const std::string& progname)
{
	return K2hvecCommand::MakeTemporaryFilepath(progname, ".k2h");
}

//---------------------------------------------------------
// K2hvecCommand Methods : Constructor/Destructor
//---------------------------------------------------------
K2hvecCommand::~K2hvecCommand()
{
}

//---------------------------------------------------------
// K2hvecCommand Methods : Parser
//---------------------------------------------------------
//
// Parse Command Line Arguments
//
// [NOTE]
// Please set and pass the "argc" and "argv" for the main function as shown below:
// (Do not include the program name as the first argument(argv[0]))
//		std::vector<std::string>	Argvs(argv + 1, argv + argc);
//
bool K2hvecCommand::ParseExecArgs(const std::vector<std::string>& Argvs, const std::string& progname)
{
	// Clear Prompt Command List
	PromptCommands.clear();

	// Parse options
	bool isset_dbgmode = false;
	bool isset_dbgfile = false;
	for(auto iter = Argvs.begin(); iter != Argvs.end(); ++iter){
		const std::string&		arg				= *iter;
		const OptionDefinition*	pOption			= FindOption(arg, ExecOptionList);

		if(!pOption){
			// unknown option
			ERR("Unknown option or paramter: %s", arg.c_str());
			return false;
		}

		switch(pOption->id){
			case	CommandId::CMDID_EXEC_HELP:
				exec_help = true;
				break;

			case	CommandId::CMDID_EXEC_FILE:
				if(!k2hfile.empty()){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the k2hash file path parameter(not found).", arg.c_str());
					return false;
				}
				k2hfile = *iter;
				break;

			case	CommandId::CMDID_EXEC_TEMPFILE:
				if(!k2htempfile.empty()){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				k2htempfile = MakeK2HashTemporaryFilepath(progname);
				break;

			case	CommandId::CMDID_EXEC_MEM:
				if(k2hmem){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				k2hmem = true;
				break;

			case	CommandId::CMDID_EXEC_MASK:
				if(0 != k2hmask){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
					return false;
				}
				if(!StringToInt(*iter, k2hmask)){
					ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
					return false;
				}
				break;

			case	CommandId::CMDID_EXEC_CMASK:
				if(0 != k2hcmask){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
					return false;
				}
				if(!StringToInt(*iter, k2hcmask)){
					ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
					return false;
				}
				break;

			case	CommandId::CMDID_EXEC_ELCNT:
				if(0 != k2helementcnt){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
					return false;
				}
				if(!StringToInt(*iter, k2helementcnt)){
					ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
					return false;
				}
				break;

			case	CommandId::CMDID_EXEC_PAGESIZE:
				if(0 != k2hpagesize){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
					return false;
				}else{
					int	tmpval = 0;
					if(!StringToInt(*iter, tmpval)){
						ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					if(tmpval < 0 || (1 << 20) < tmpval){
						ERR("The parameter for the %s option value(%s) must be over 0 to 2^20(1048576).", arg.c_str(), iter->c_str());
						return false;
					}
					k2hpagesize = static_cast<size_t>(tmpval);
				}
				break;

			case	CommandId::CMDID_EXEC_FULL:
				if(k2hfullmap){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				k2hfullmap = true;
				break;

			case	CommandId::CMDID_EXEC_RO:
				if(k2hreadonly){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				k2hreadonly = true;
				break;

			case	CommandId::CMDID_EXEC_LIBVER:
				if(print_libver){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				print_libver = true;
				break;

			case	CommandId::CMDID_EXEC_BH_BITSIZE:
				if(isset_bh_bitsize){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
					return false;
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the bit size parameter(not found).", arg.c_str());
					return false;
				}else{
					int	tmpval = 0;
					if(!StringToInt(*iter, tmpval)){
						ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					if(!K2hVecDb::SetBinHashSize(tmpval)){
						ERR("The parameter for the %s option is something wrong: %s", arg.c_str(), iter->c_str());
						return false;
					}
				}
				isset_bh_bitsize = true;
				break;

			case	CommandId::CMDID_EXEC_NEIGHBOR_LIMIT:
				if(isset_neighbor_limit){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
					return false;
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the limit size parameter(not found).", arg.c_str());
					return false;
				}else{
					int	tmpval = 0;
					if(!StringToInt(*iter, tmpval)){
						ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					if(!K2hVecDb::SetNeighborKeysLimit(static_cast<size_t>(tmpval))){
						ERR("The parameter for the %s option is something wrong: %s", arg.c_str(), iter->c_str());
						return false;
					}
				}
				isset_neighbor_limit = true;
				break;

			case	CommandId::CMDID_EXEC_DIRECT_LIMIT:
				if(isset_direct_limit){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
					return false;
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the limit size parameter(not found).", arg.c_str());
					return false;
				}else{
					int	tmpval = 0;
					if(!StringToInt(*iter, tmpval)){
						ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					if(!K2hVecDb::SetContentDirectLimit(static_cast<size_t>(tmpval))){
						ERR("The parameter for the %s option is something wrong: %s", arg.c_str(), iter->c_str());
						return false;
					}
				}
				isset_direct_limit = true;
				break;

			case	CommandId::CMDID_EXEC_HISMAX:
				if(0 != history_count){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
					return false;
				}else{
					int	tmpval = 0;
					if(!StringToInt(*iter, tmpval)){
						ERR("The parameter for the %s option is not a decimal value(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					if(tmpval < 0 || (1 << 16) < tmpval){
						ERR("The parameter for the %s option value(%s) must be over 0 to 2^16(65536).", arg.c_str(), iter->c_str());
						return false;
					}
					history_count = static_cast<size_t>(tmpval);
				}
				break;

			case	CommandId::CMDID_EXEC_DBGLEVEL:
				{
					if(isset_dbgmode){
						ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
					}
					if(++iter == Argvs.end()){
						ERR("The %s option requires the mask bit count parameter(not found).", arg.c_str());
						return false;
					}
					K2hvecDbgMode	newmode;
					if(IsDbgModeString(*iter, K2hvecDbgMode::VECDBG_SILENT)){
						newmode = K2hvecDbgMode::VECDBG_SILENT;
					}else if(IsDbgModeString(*iter, K2hvecDbgMode::VECDBG_ERR)){
						newmode = K2hvecDbgMode::VECDBG_ERR;
					}else if(IsDbgModeString(*iter, K2hvecDbgMode::VECDBG_WARN)){
						newmode = K2hvecDbgMode::VECDBG_WARN;
					}else if(IsDbgModeString(*iter, K2hvecDbgMode::VECDBG_MSG)){
						newmode = K2hvecDbgMode::VECDBG_MSG;
					}else if(IsDbgModeString(*iter, K2hvecDbgMode::VECDBG_DUMP)){
						newmode = K2hvecDbgMode::VECDBG_DUMP;
					}else{
						ERR("The parameter for the %s option is unknown(%s).", arg.c_str(), iter->c_str());
						return false;
					}
					SetK2hvecDbgMode(newmode);
					isset_dbgmode = true;
				}
				break;

			case	CommandId::CMDID_EXEC_DBGFILE:
				if(isset_dbgfile){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the debug output file path parameter(not found).", arg.c_str());
					return false;
				}
				if(!SetK2hvecDbgFile(iter->c_str())){
					ERR("Failed to set debug file(%s).", iter->c_str());
					return false;
				}
				isset_dbgfile = true;
				break;

			case	CommandId::CMDID_EXEC_LAPTIME:
				if(use_laptime){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				use_laptime = true;
				break;

			case	CommandId::CMDID_EXEC_NOWELCOM:
				if(no_welcom_msg){
					ERR("Already specified %s option. However, the process will continue as is.", arg.c_str());
				}
				no_welcom_msg = true;
				break;

			case	CommandId::CMDID_EXEC_RUN:
				if(!run_commandfile.empty()){
					ERR("Already specified %s option, it will be overwritten with the newly specified value.", arg.c_str());
				}
				if(++iter == Argvs.end()){
					ERR("The %s option requires the debug output file path parameter(not found).", arg.c_str());
					return false;
				}
				run_commandfile = *iter;
				break;

			case	CommandId::CMDID_EXEC_PROMPT:
				direct_prompt_mode = true;
				{
					// The strings after this command are all prompt command arguments.
					std::vector<std::string>	remainingArgvs(std::next(iter), Argvs.end());
					if(!PromptCommandBase::MakePromptCommandList(remainingArgvs, true, PromptCommands)){
						// Something error
						return false;
					}
				}
				// for end of loop
				iter = Argvs.end() - 1;
				break;

			default:
				ERR("Unknown option %s is specified.", arg.c_str());
				return false;
		}
	}

	// Check execute command options
	if(!CheckExecArgs()){
		return false;
	}

	// Load Prompt Command file
	if(!run_commandfile.empty()){
		// Load & Parse prompt command files
		if(!PromptCommandBase::LoadPromptCommandList(run_commandfile, PromptCommands)){
			return false;
		}
	}
	return true;
}

//
// Check execute commands
//
bool K2hvecCommand::CheckExecArgs()
{
	if(exec_help){
		// not check other options.
		return true;
	}

	if(!k2hfile.empty()){
		if(!k2htempfile.empty() || k2hmem){
			ERR("The options --file(-f), --temporary_file(-t), and --memory(-m) cannot be specified together. Please specify only one of them.");
			return false;
		}
	}else{
		if(!k2htempfile.empty() && k2hmem){
			ERR("The options \"--file(-f)\", \"--temporary_file(-t)\", and \"--memory(-m)\" cannot be specified together. Please specify only one of them.");
			return false;
		}else if(k2htempfile.empty() && !k2hmem){
			k2hmem = true;						// default k2hash memory mode
		}
		if(k2hreadonly){
			ERR("The \"--read_only(-r)\" option cannot be specified together with \"--temporary_file(-t)\" or \"--memory(-m)\". It is used with the \"--file(-f)\" option.");
			return false;
		}
		if(k2hmem && !k2hfullmap){
			k2hfullmap = true;					// default full mapping for memory mode
		}
	}
	if(!run_commandfile.empty() && direct_prompt_mode){
		ERR("Can not specify the \"--\" option(run directly prompt command)  and the \"--run_file(-run)\" option at the same time.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// K2hvecCommand Methods : Common Settings
//---------------------------------------------------------
//
// Initialize K2hVec object
//
bool K2hvecCommand::InitializeK2hVec()
{
	// Clean(detach)
	if(k2hvecobj.IsAttached()){
		k2hvecobj.Clean();
	}

	// Attach k2hash
	const char*	pK2hfile		= (k2hmem ? nullptr : !k2hfile.empty() ? k2hfile.c_str() : k2htempfile.c_str()); 
	bool		isK2hReadOnly	= k2hreadonly;
	bool		isK2hCreate		= (k2hreadonly			? false			: true);
	bool		isK2hTempFile	= (k2htempfile.empty()	? false			: true);
	bool		isK2hFullmap	= k2hfullmap;
	int			K2hMask			= (0 != k2hmask			? k2hmask		: K2HShm::DEFAULT_MASK_BITCOUNT);
	int			K2hCMask		= (0 != k2hcmask		? k2hcmask		: K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT);
	int			K2hElementCnt	= (0 != k2helementcnt	? k2helementcnt	: K2HShm::DEFAULT_MAX_ELEMENT_CNT);
	size_t		K2hPagesize		= (0 != k2hpagesize		? k2hpagesize	: K2HShm::MIN_PAGE_SIZE);

	if(!k2hvecobj.AttachK2hash(pK2hfile, isK2hReadOnly, isK2hCreate, isK2hTempFile, isK2hFullmap, K2hMask, K2hCMask, K2hElementCnt, K2hPagesize)){
		ERR("Failed to initialize/attach k2hash %s.", k2hmem ? "memory" : "file");
		return false;
	}

	return true;
}

//---------------------------------------------------------
// K2hvecCommand Methods : K2hVec operations
//---------------------------------------------------------
//
// Set data to k2hash
//
bool K2hvecCommand::Set(const std::vector<float>& embedding, const std::string& content, const std::string* pstrmodel, const std::vector<float>* psysembedding, const std::string* pstrpass, const time_t* pexpire)
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Set(embedding, content, pstrmodel, psysembedding, pstrpass, pexpire);
}

//
// Get content text(direct)
//
bool K2hvecCommand::Get(const std::vector<float>& embedding, std::string& content, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Get(embedding, content, pstrmodel, psysembedding, checkattr, pstrpass);

}

//
// Get all(direct) by key
//
bool K2hvecCommand::Get(const std::string& strKey, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Get(strKey, em, sysem, content, model, checkattr, pstrpass);
}

//
// Get embedding(direct) by key
//
bool K2hvecCommand::Get(const std::string& strKey, std::vector<float>& em, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Get(strKey, em, checkattr, pstrpass);
}

//
// Get content text(direct) by key
//
bool K2hvecCommand::Get(const std::string& strKey, std::string& content, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Get(strKey, content, checkattr, pstrpass);
}

//
// Remove key and index key
//
bool K2hvecCommand::Remove(const std::vector<float>& embedding, bool mainkeyonly, const std::string* pstrmodel, const std::vector<float>* psysembedding) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Remove(embedding, mainkeyonly, pstrmodel, psysembedding);

}

bool K2hvecCommand::Remove(const std::string& strKey, bool mainkeyonly) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Remove(strKey, mainkeyonly);
}

//
// Search(find) embedding data
//
bool K2hvecCommand::Search(const std::vector<float>& embedding, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Search(embedding, res_key, res_em, res_sysem, res_content, res_model, score, pstrmodel, psysembedding, checkattr, pstrpass);
}

bool K2hvecCommand::Search(const std::vector<float>& embedding, std::string& res_key, std::string& res_content, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Search(embedding, res_key, res_content, score, pstrmodel, psysembedding, checkattr, pstrpass);
}

bool K2hvecCommand::Search(const std::vector<float>& embedding, std::string& res_key, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	if(!k2hvecobj.IsAttached()){
		ERR("K2hVec object does not attach the K2Hash database yet.");
		return false;
	}
	return k2hvecobj.Search(embedding, res_key, score, pstrmodel, psysembedding, checkattr, pstrpass);
}

//---------------------------------------------------------
// K2hvecCommand Methods : Preloaded Embedding
//---------------------------------------------------------
//
// Set Preloaded Embedding data
//
bool K2hvecCommand::SetPreloadedEmbedding(const std::vector<float>& embedding)
{
	if(static_cast<size_t>(ProjectionMatrix::GetProjectionMatrixDimension()) != embedding.size()){
		ERR("Dimension size(%d) for internal Projection Matrix in this program and embedding data length(%lu) is different.", ProjectionMatrix::GetProjectionMatrixDimension(), embedding.size());
		return false;
	}
	PreloadedEm = embedding;

	return true;
}

std::string K2hvecCommand::GetPreloadedEmbeddingChecksum() const
{
	return GetChecksumString(PreloadedEm);
}

std::string K2hvecCommand::GetPreloadedEmbeddingSecondHash() const
{
	return MakeFlatStringFromUint64(CalSecondHash(PreloadedEm), false);
}

const std::vector<float>& K2hvecCommand::GetPreloadedEmbedding() const
{
	return PreloadedEm;
}

//---------------------------------------------------------
// K2hvecCommand Methods : Execution
//---------------------------------------------------------
//
// Run Early Exit Command
//
bool K2hvecCommand::RunEarlyExitProcess(const std::string& progname)
{
	if(exec_help){
		PrintHelp(progname);
		return true;
	}
	if(print_libver){
		k2hvec_print_version(stdout);
		return true;
	}
	return false;
}

//
// Run prompt commands
//
bool K2hvecCommand::RunPromptCommands(bool& isTerminate)
{
	for(const auto& CmdObj: PromptCommands){
		// Execute one command
		if(!CmdObj->Execute(isTerminate)){
			// failed
			ERR("Failed to run \"%s\" command: \"%s\"", CmdObj->GetCommandName().c_str(), CmdObj->GetCommandLineArgs().c_str());
			LastCommandResult = false;

			if(error_break_mode){
				return false;
			}
		}else{
			LastCommandResult = true;
		}
		if(isTerminate){
			break;
		}
	}
	return true;
}

//---------------------------------------------------------
// K2hvecCommand Methods : Information
//---------------------------------------------------------
//
// Setup K2Hash
//
void K2hvecCommand::PrintLaunchMessage()
{
	if(no_welcom_msg){
		return;
	}

	// Get K2Hash information
	std::string	strage;
	if(!k2hfile.empty()){
		strage = "Permanent file( " + k2hfile + " )";
	}else if(!k2htempfile.empty()){
		strage = "Temporary file( " + k2htempfile + " )";
	}else{
		strage = "On memory";
	}
	std::string	access		= (k2hreadonly 			? "Read only"	: "Read/Write");
	std::string	mapping		= (k2hfullmap 			? "Full mapping": "Index mapping only");
	int			mask		= (0 != k2hmask			? k2hmask		: K2HShm::DEFAULT_MASK_BITCOUNT);
	int			cmask		= (0 != k2hcmask		? k2hcmask		: K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT);
	int			element		= (0 != k2helementcnt	? k2helementcnt	: K2HShm::DEFAULT_MAX_ELEMENT_CNT);
	size_t		page		= (0 != k2hpagesize		? k2hpagesize	: K2HShm::MIN_PAGE_SIZE);

	// Get current Projection Matrix in library
	std::string	strName		= ProjectionMatrix::GetProjectionMatrixName();
	std::string strVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
	int 		nBitsize	= ProjectionMatrix::GetProjectionMatrixBitsize();
	int			nDimension	= ProjectionMatrix::GetProjectionMatrixDimension();

	// Get Common configuration(Binary hash / Index keys / Direct Content Limits)
	int			nBHBitsize		= K2hVecDb::GetBinHashSize();
	size_t		NeighborLimit	= K2hVecDb::GetNeighborKeysLimit();
	size_t		DirectLimit		= K2hVecDb::GetContentDirectLimit();

	// Print
	PRN("");
	PRN("-------------------------------------------------------------------------------");
	PRN("                  Command Line Tool for K2HVEC and K2HASH");
	PRN("");
	PRN("      K2HVEC is a high-performance vector database for fast vector search,     ");
	PRN("                  built on top of the K2HASH key-value store.                  ");
	PRN("-------------------------------------------------------------------------------");
	PRN("                           K2Hash Infomration");
	PRN("");
	PRN("    Strage type                             : %s", strage.c_str());
	PRN("    Access mode                             : %s", access.c_str());
	PRN("    Data mapping type                       : %s", mapping.c_str());
	PRN("    Key Index Mask count(mask)              : %d", mask);
	PRN("    Collision Key Index Mask count(cmask)   : %d", cmask);
	PRN("    Max Element count(element count)        : %d", element);
	PRN("    Page size                               : %lu", page);
	PRN("-------------------------------------------------------------------------------");
	PRN("                     Projection Matrix in K2HVEC library");
	PRN("");
	PRN("    Name                                    : %s",		strName.c_str());
	PRN("    Version                                 : %s",		strVersion.c_str());
	PRN("    Dimension size                          : %d",		nDimension);
	PRN("    Bit count(size)                         : %d bit",	nBitsize);
	PRN("-------------------------------------------------------------------------------");
	PRN("   Common configuration(Binary hash / Index keys / Direct Content Limits)");
	PRN("");
	PRN("    Calaculate Binary hash bit size         : %d",		nBHBitsize);
	PRN("    Search neighbor key upper limit         : %zu",	NeighborLimit);
	if(K2hVecDb::CONTENT_ONLY_DIRECT == DirectLimit){
		PRN("    Upper limit for saving content directly : no limit");
	}else{
		PRN("    Upper limit for saving content directly : %zu",	DirectLimit);
	}
	PRN("-------------------------------------------------------------------------------");
	PRN("");
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
