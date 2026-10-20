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

#ifndef	K2HVEC_CLI_CMD_H
#define	K2HVEC_CLI_CMD_H

#include <string>
#include <vector>
#include <cstddef>

#include "k2hvecdb.h"
#include "k2hvecdbg.h"

//---------------------------------------------------------
// Common Definition
//---------------------------------------------------------
//
// All Command Ids
//
enum class CommandId{
	CMDID_UNKNOWN				= -1,

	// execute line commands
	CMDID_EXEC_HELP				= 0,
	CMDID_EXEC_FILE,
	CMDID_EXEC_TEMPFILE,
	CMDID_EXEC_MEM,
	CMDID_EXEC_MASK,
	CMDID_EXEC_CMASK,
	CMDID_EXEC_ELCNT,
	CMDID_EXEC_PAGESIZE,
	CMDID_EXEC_FULL,
	CMDID_EXEC_RO,
	CMDID_EXEC_LIBVER,
	CMDID_EXEC_BH_BITSIZE,
	CMDID_EXEC_NEIGHBOR_LIMIT,
	CMDID_EXEC_DIRECT_LIMIT,
	CMDID_EXEC_HISMAX,
	CMDID_EXEC_DBGLEVEL,
	CMDID_EXEC_DBGFILE,
	CMDID_EXEC_LAPTIME,
	CMDID_EXEC_NOWELCOM,
	CMDID_EXEC_RUN,
	CMDID_EXEC_PROMPT,

	// prompt line commands
	CMDID_PROMPT_HELP			= 100,		// Prompt Command starts 100
	CMDID_PROMPT_QUIT,
	CMDID_PROMPT_SHELL,
	CMDID_PROMPT_ECHO,
	CMDID_PROMPT_SLEEP,
	CMDID_PROMPT_RUN,
	CMDID_PROMPT_BREAK,
	CMDID_PROMPT_NOBREAK,
	CMDID_PROMPT_HISTORYMAX,
	CMDID_PROMPT_HISTORY,
	CMDID_PROMPT_SAVEHIS,
	CMDID_PROMPT_LIBVER,
	CMDID_PROMPT_LAPTIME,
	CMDID_PROMPT_INFO,
	CMDID_PROMPT_DUMP,
	CMDID_PROMPT_DBG,
	CMDID_PROMPT_DBGFILE,
	CMDID_PROMPT_LASTSTATUS,
	CMDID_PROMPT_COMATTR,
	CMDID_PROMPT_COMMON_CONF,
	CMDID_PROMPT_CREATEK2HKEY,
	CMDID_PROMPT_SET,
	CMDID_PROMPT_GET,
	CMDID_PROMPT_REMOVE,
	CMDID_PROMPT_READSUMMARY,
	CMDID_PROMPT_SEARCH,
	CMDID_PROMPT_LOADPM,
	CMDID_PROMPT_LOADEM,
	CMDID_PROMPT_CREATEPM,
	CMDID_PROMPT_CREATEEM,
	CMDID_PROMPT_CALBH,
	CMDID_PROMPT_CALCOS
};

//
// Option Definition
//
struct OptionDefinition{
	CommandId					id				= CommandId::CMDID_UNKNOWN;
	std::vector<std::string>	options;
	int		 					min_param_cnt	= 0;
	int		 					max_param_cnt	= 0;
	bool						direct_command	= false;
};

//
// Common Functions
//
bool StringToInt(const std::string& str, int& val);
std::string CombineString(const std::vector<std::string>& strlist);
const OptionDefinition* FindOption(const std::string& param, const std::vector<OptionDefinition>& OptionList);
std::string FindCommandName(CommandId cmdid, const std::vector<OptionDefinition>& OptionList);
CommandId FindCommandId(const std::string& param, const std::vector<OptionDefinition>& OptionList);

// [NOTE]
// k2hvec_cli_promptcmd.h needs OptionDefinition before including.
//
#include "k2hvec_cli_promptcmd.h"

//---------------------------------------------------------
// K2hvecCommand Class
//---------------------------------------------------------
//
// This class provides parsing functionality for commands entered via the command
// line or a prompt.
// This is used to execute commands entered at the prompt.
//
class K2hvecCommand
{
	protected:
		// Delimiter
		static const char*							OptDelimiter;

		// Options
		static const std::vector<OptionDefinition>	ExecOptionList;

	protected:
		// Process execution mode
		bool				exec_help{false};
		bool				print_libver{false};
		bool				no_welcom_msg{false};
		size_t				history_count{0};
		bool				use_laptime{false};
		std::string			run_commandfile{""};
		bool				error_break_mode{true};
		bool				direct_prompt_mode{false};

		// K2hash option
		std::string			k2hfile{""};
		std::string			k2htempfile{""};
		bool				k2hmem{false};
		int					k2hmask{0};
		int					k2hcmask{0};
		int					k2helementcnt{0};
		size_t				k2hpagesize{0};
		bool				k2hfullmap{false};
		bool				k2hreadonly{false};

		// Binary Hash and Index keys Options
		bool				isset_bh_bitsize{false};
		bool				isset_neighbor_limit{false};
		bool				isset_direct_limit{false};

		// Objects
		K2hVecDb			k2hvecobj;					// K2hvec object
		std::vector<float>	PreloadedEm;				// Preloded embedding data
		PromptCommandList	PromptCommands;				// Prompt commands
		bool				LastCommandResult{true};	// Last command result

	protected:
		static void PrintHelp(const std::string& progname);
		static std::string MakeK2HashTemporaryFilepath(const std::string& progname);

		bool CheckExecArgs();

	public:
		// K2hVec Common variables
		static std::string GetDebugLevelInformation();
		static void SetupDebugLevel(const K2hvecDbgMode& dbglevel, bool k2hash = false);
		static bool SetupDebugFile(const std::string& strFile, bool k2hash = false);

		static std::string MakeTemporaryFilepath(const std::string& prefix, const std::string& suffix);

		K2hvecCommand() = default;
		~K2hvecCommand();

		// Attributes
		size_t GetHistorySize() const { return history_count; }
		bool IsLapTime() const { return use_laptime; }
		bool IsWelcomeMessage() const { return !no_welcom_msg; }
		bool IsDirectPromptMode() const { return direct_prompt_mode; }
		void SetErrorBreakMode(bool isbreak) { error_break_mode = isbreak; }
		bool GetErrorBreakMode() const { return error_break_mode; }

		// Parser / Loader
		bool ParseExecArgs(const std::vector<std::string>& Argvs, const std::string& progname);

		// K2Hash Setup / Information
		bool InitializeK2hVec();

		// K2hVec operations
		bool Set(const std::vector<float>& embedding, const std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, const std::string* pstrpass = nullptr, const time_t* pexpire = nullptr);
		bool Get(const std::vector<float>& embedding, std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::vector<float>& em, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::string& content, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Remove(const std::vector<float>& embedding, bool mainkeyonly, const std::string* pstrmodel, const std::vector<float>* psysembedding) const;
		bool Remove(const std::string& strKey, bool mainkeyonly) const;
		bool Search(const std::vector<float>& embedding, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Search(const std::vector<float>& embedding, std::string& res_key, std::string& res_content, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Search(const std::vector<float>& embedding, std::string& res_key, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;

		// Preloaded Embedding data
		bool SetPreloadedEmbedding(const std::vector<float>& embedding);
		int GetPreloadedEmbeddingDimension() const { return static_cast<int>(PreloadedEm.size()); }
		std::string GetPreloadedEmbeddingChecksum() const;
		std::string GetPreloadedEmbeddingSecondHash() const;
		const std::vector<float>& GetPreloadedEmbedding() const;

		// Run
		bool RunEarlyExitProcess(const std::string& progname);
		bool RunPromptCommands(bool& isTerminate);
		void SetLastCommandStatus(bool result) { LastCommandResult = result; }
		bool GetLastCommandStatus() const { return LastCommandResult; }

		// Information
		void PrintLaunchMessage();
};

#endif	// K2HVEC_CLI_CMD_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
