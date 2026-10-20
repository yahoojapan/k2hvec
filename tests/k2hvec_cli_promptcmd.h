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

#ifndef	K2HVEC_CLI_PROMPTCMD_H
#define	K2HVEC_CLI_PROMPTCMD_H

#include <memory>
#include <utility>

#include "k2hvec_cli_cmd.h"
#include "k2hvec_cli_input.h"

//---------------------------------------------------------
// PromptCommandBase Class
//---------------------------------------------------------
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
class K2hvecCommand;
class PromptCommandBase;
using PromptCommandList = std::vector<std::unique_ptr<PromptCommandBase>>;

class PromptCommandBase
{
	protected:
		// Types
		using PromptArgList		= std::vector<std::string>;
		using PromptCommandArgs	= std::vector<PromptArgList>;

		// Class const variables
		static constexpr int						DEFAULT_BITSIZE		= 48;
		static constexpr int						DEFAULT_DIMENSION	= 4096;
		static const char*							OPT_DELIMITERS;					// Delimiter for parser
		static const char*							DEFAULT_CUSTOM_NAME;
		static const char*							DEFAULT_VERSION;
		static const char*							EMBEDDING_ARRAY_NAME;

		// Class variables
		static K2hvecCommand*						pK2hvecCmdObj;					// K2hvecCommand object pointer(some inherit classes use)
		static ConsoleInput*						pConsoleObj;					// ConsoleInput object pointer(some inherit classes use)
		static bool									useLapTime;						// Lap time
	    static const std::vector<OptionDefinition>	PromptOptionList;				// All Command Line Options

		// Variables
		PromptArgList	command_args;												// Command line arguments
		CommandId		command_id{CommandId::CMDID_UNKNOWN};						// Command ID (override from subclasses)

	public:
		// RAII(Resource Acquisition Is Initialization) for pConsoleObj
		class ConsoleInputScope
		{
			public:
				explicit ConsoleInputScope(K2hvecCommand& cmdobj, ConsoleInput& input)
				{
					PromptCommandBase::pK2hvecCmdObj= &cmdobj;
					PromptCommandBase::pConsoleObj	= &input;
				}

				~ConsoleInputScope()
				{
					PromptCommandBase::pK2hvecCmdObj= nullptr;
					PromptCommandBase::pConsoleObj	= nullptr;
				}

				ConsoleInputScope(const ConsoleInputScope&)				= delete;
				ConsoleInputScope& operator=(const ConsoleInputScope&)	= delete;
		};

	protected:
		// Utilities for paesing parameters
		static bool IsProjectionMatrixParameter(const std::string& parameter);
		static bool IsEmbeddingDataParameter(const std::string& parameter);
		static bool IsSimpleParameter(const std::string& parameter);
		static bool IsWithK2hashParameter(const std::string& parameter);
		static bool IsLongParameter(const std::string& parameter);
		static bool IsOnParameter(const std::string& parameter);
		static bool IsOffParameter(const std::string& parameter);
		static bool IsMessageParameter(const std::string& parameter);
		static bool GetParameter(const std::string& parameter, const std::vector<std::string>& names, std::string& value);
		static bool GetExpireParameter(const std::string& parameter, time_t& expire);
		static bool GetPassphraseParameter(const std::string& parameter, std::string& passphrase);
		static bool GetNoAttributeCheckParameter(const std::string& parameter, bool& is_noattrs);
		static bool GetMainkeyOnlyCheckParameter(const std::string& parameter, bool& is_mainkeyonly);
		static bool GetDimensionParameter(const std::string& parameter, int& dimension);
		static bool GetBitSizeParameter(const std::string& parameter, int& bitsize);
		static bool GetNameParameter(const std::string& parameter, std::string& name);
		static bool GetVersionParameter(const std::string& parameter, std::string& version);
		static bool GetK2HashKeyParameter(const std::string& parameter, std::string& key, bool& usePreloadedEm, std::string& emfile);
		static bool GetEmbeddingParameter(const std::string& parameter, bool& usePreloadedEm, std::string& emfile);
		static bool GetOutputfileParameter(const std::string& parameter, std::string& file);
		static bool GetEmfileParameter(const std::string& parameter, std::string& file);
		static bool GetPmfileParameter(const std::string& parameter, std::string& file);
		static bool GetValuefileParameter(const std::string& parameter, std::string& file);
		static bool GetValueParameter(const std::string& parameter, std::string& contents);
		static bool GetScoreParameter(const std::string& parameter, std::string& contents);
		static bool CvtScoreStringToFloat(const std::string& str, float& value);
		static bool GetBinaryHashBitSizeParameter(const std::string& parameter, int& bitsize);
		static bool GetNeighborKeyLimitParameter(const std::string& parameter, size_t& neighborkeylimit);
		static bool GetContentDirectLimitParameter(const std::string& parameter, size_t& directlimit);

		static void RemoveComment(std::string& oneLine);
		static bool ParsePromptArgs(const std::vector<std::string>& Argvs, PromptCommandArgs& PromptArgs);
		static bool CheckPromptCommandList(bool isdirect, const PromptCommandList& PromptCommands);

		PromptCommandBase() = default;
		PromptCommandBase(const PromptArgList& args, CommandId id) : command_args(args), command_id(id) {}

		// Actual processing logic for each command
		virtual bool RawExecute(bool& isTerminate) = 0;

	public:
		static void SetLapTime(bool enable);
		static void ToggleLapTime();
		static void EnableLapTime();
		static void DisableLapTime();

		static void PrintHelp();						// see. NOTE

		static bool LoadPromptCommandList(const std::string& strFile, PromptCommandList& PromptCommands);
		static bool MakePromptCommandList(const std::vector<std::string>& Argvs, bool isdirect, PromptCommandList& PromptCommands);

		virtual ~PromptCommandBase() = default;

		CommandId GetCommandId() const { return command_id; }
		std::string GetCommandName() const;
		std::string GetCommandLineArgs() const;

		bool Execute(bool& isTerminate);
};

//---------------------------------------------------------
// Inherit Classes from PromptCommandBase
//---------------------------------------------------------
class PromptCommandHelp : public PromptCommandBase
{
	protected:
		CommandId	CmdId{CommandId::CMDID_UNKNOWN};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandHelp(const PromptArgList& args, CommandId id) : PromptCommandBase(args, CommandId::CMDID_PROMPT_HELP), CmdId(id) {}
		virtual ~PromptCommandHelp() override {}
};

class PromptCommandQuit : public PromptCommandBase
{
	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandQuit(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_QUIT) {}
		virtual ~PromptCommandQuit() override {}
};

class PromptCommandShell : public PromptCommandBase
{
	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandShell(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_SHELL) {}
		virtual ~PromptCommandShell() override {}
};

class PromptCommandEcho : public PromptCommandBase
{
	protected:
		PromptArgList	printStrings;

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandEcho(const PromptArgList& args, const PromptArgList& strings) : PromptCommandBase(args, CommandId::CMDID_PROMPT_ECHO), printStrings(strings) {}
		virtual ~PromptCommandEcho() override {}
};

class PromptCommandSleep : public PromptCommandBase
{
	protected:
		int	second{0};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandSleep(const PromptArgList& args, int sec) : PromptCommandBase(args, CommandId::CMDID_PROMPT_SLEEP), second(sec) {}
		virtual ~PromptCommandSleep() override {}
};

class PromptCommandRun : public PromptCommandBase
{
	protected:
		static constexpr char	RUNCMD_NEST_DEPTH_LIMIT = 10;
		static int				CurrentNestDepth;

		std::string	strFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandRun(const PromptArgList& args, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_RUN), strFile(file) {}
		virtual ~PromptCommandRun() override {}
};

class PromptCommandBreak : public PromptCommandBase
{
	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandBreak(const PromptArgList& args, bool is_break) : PromptCommandBase(args, (is_break ? CommandId::CMDID_PROMPT_BREAK : CommandId::CMDID_PROMPT_NOBREAK)) {}
		virtual ~PromptCommandBreak() override {}
};

class PromptCommandHistoryMax : public PromptCommandBase
{
	protected:
		int	history_max{0};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandHistoryMax(const PromptArgList& args, int max) : PromptCommandBase(args, CommandId::CMDID_PROMPT_HISTORYMAX), history_max(max) {}
		virtual ~PromptCommandHistoryMax() override {}
};

class PromptCommandHistory : public PromptCommandBase
{
	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandHistory(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_HISTORY) {}
		virtual ~PromptCommandHistory() override {}
};

class PromptCommandSaveHistory : public PromptCommandBase
{
	protected:
		std::string	strFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandSaveHistory(const PromptArgList& args, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_SAVEHIS), strFile(file) {}
		virtual ~PromptCommandSaveHistory() override {}
};

class PromptCommandLibVersion : public PromptCommandBase
{
	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandLibVersion(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_LIBVER) {}
		virtual ~PromptCommandLibVersion() override {}
};

class PromptCommandLapTime : public PromptCommandBase
{
	protected:
		bool	isToggle{false};
		bool	toEnable{false};	// [NOTE] If isToggle is true, the value of toEnable is ignored.
		bool	isMeaage{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandLapTime(const PromptArgList& args, bool toggle, bool enable, bool message) : PromptCommandBase(args, CommandId::CMDID_PROMPT_LAPTIME), isToggle(toggle), toEnable(toggle ? false : enable), isMeaage(message) {}
		virtual ~PromptCommandLapTime() override {}
};

class PromptCommandInfo : public PromptCommandBase
{
	protected:
		bool	isPM{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandInfo(const PromptArgList& args, bool pm) : PromptCommandBase(args, CommandId::CMDID_PROMPT_INFO), isPM(pm) {}
		virtual ~PromptCommandInfo() override {}
};

class PromptCommandDump : public PromptCommandBase
{
	protected:
		bool		isPM{false};
		std::string	strFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandDump(const PromptArgList& args, bool pm, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_DUMP), isPM(pm), strFile(file) {}
		virtual ~PromptCommandDump() override {}
};

class PromptCommandDebugLevel : public PromptCommandBase
{
	protected:
		bool			displayMode{true};
		K2hvecDbgMode	dbglevel{K2hvecDbgMode::VECDBG_SILENT};
		bool			withk2hash{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandDebugLevel(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_DBG), displayMode(true), dbglevel(K2hvecDbgMode::VECDBG_SILENT), withk2hash(false) {}
		explicit PromptCommandDebugLevel(const PromptArgList& args, K2hvecDbgMode level, bool k2hash) : PromptCommandBase(args, CommandId::CMDID_PROMPT_DBG), displayMode(false), dbglevel(level), withk2hash(k2hash) {}
		virtual ~PromptCommandDebugLevel() override {}
};

class PromptCommandDebugFile : public PromptCommandBase
{
	protected:
		std::string	strFile{""};
		bool		withk2hash{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandDebugFile(const PromptArgList& args, const std::string& file, bool k2hash) : PromptCommandBase(args, CommandId::CMDID_PROMPT_DBGFILE), strFile(file), withk2hash(k2hash) {}
		virtual ~PromptCommandDebugFile() override {}
};

class PromptCommandLastStatus : public PromptCommandBase
{
	protected:
		bool	IsLong{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandLastStatus(const PromptArgList& args, bool setlong) : PromptCommandBase(args, CommandId::CMDID_PROMPT_LASTSTATUS), IsLong(setlong) {}
		virtual ~PromptCommandLastStatus() override {}
};

class PromptCommandCommonAttribute : public PromptCommandBase
{
	protected:
		bool		isPrint{false};
		bool		isExpire{false};
		bool		isPass{false};
		time_t		Expire{0};
		std::string	strPass{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();
		explicit PromptCommandCommonAttribute(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMATTR), isPrint(true), isExpire(false), isPass(false), Expire(0), strPass("") {}
		explicit PromptCommandCommonAttribute(const PromptArgList& args, bool isset_expire, time_t expire_time, bool isset_pass, const std::string& passphrase) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMATTR), isPrint(false), isExpire(isset_expire), isPass(isset_pass), Expire(expire_time), strPass(passphrase) {}
		virtual ~PromptCommandCommonAttribute() override {}
};

class PromptCommandCommonConfig : public PromptCommandBase
{
	protected:
		bool	isPrint{false};
		bool	isBitSize{false};
		bool	isNeighborKeyLimit{false};
		bool	isDirectLimit{false};
		int		nBitSize{-1};
		size_t	NeighborKeyLimit{0};
		size_t	DirectLimit{0};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();
		explicit PromptCommandCommonConfig(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMMON_CONF), isPrint(true), isBitSize(false), isNeighborKeyLimit(false), isDirectLimit(false), nBitSize(-1), NeighborKeyLimit(0), DirectLimit(0) {}
		explicit PromptCommandCommonConfig(const PromptArgList& args, bool isset_bitsize, int bitsize, bool isset_neighborkeylimit, size_t neighbor, bool isset_directlimit, size_t direct) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMMON_CONF), isPrint(false), isBitSize(isset_bitsize), isNeighborKeyLimit(isset_neighborkeylimit), isDirectLimit(isset_directlimit), nBitSize(bitsize), NeighborKeyLimit(neighbor), DirectLimit(direct) {}
		virtual ~PromptCommandCommonConfig() override {}
};

class PromptCommandK2hKey : public PromptCommandBase
{
	protected:
		std::string	strFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandK2hKey(const PromptArgList& args, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_CREATEK2HKEY), strFile(file) {}
		virtual ~PromptCommandK2hKey() override {}
};

class PromptCommandSet : public PromptCommandBase
{
	protected:
		bool		isExpire{false};
		bool		isPass{false};
		time_t		Expire{0};
		std::string	strPass{""};
		std::string	strEmFile{""};
		std::string	strValue{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandSet(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMATTR), isExpire(false), isPass(false), Expire(0), strPass(""), strEmFile(""), strValue("") {}
		explicit PromptCommandSet(const PromptArgList& args, bool isset_expire, bool isset_pass, time_t expire_time, const std::string& passphrase, const std::string& emfile, const std::string& value) : PromptCommandBase(args, CommandId::CMDID_PROMPT_COMATTR), isExpire(isset_expire), isPass(isset_pass), Expire(expire_time), strPass(passphrase), strEmFile(emfile), strValue(value) {}
		virtual ~PromptCommandSet() override {}
};

class PromptCommandGet : public PromptCommandBase
{
	protected:
		std::string	strKey{""};
		bool		usePreloadedEm{false};
		std::string	strEmFile{""};
		bool		isNoAttr{false};
		bool		isPass{false};
		std::string	strPass{""};
		std::string	strOutputFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandGet(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_GET), strKey(""), usePreloadedEm(true), strEmFile(""), isNoAttr(false), isPass(false), strPass(""), strOutputFile("") {}
		explicit PromptCommandGet(const PromptArgList& args, bool isset_key, const std::string& keyname, bool useEm, const std::string& emfile, bool noattr, bool isset_pass, std::string& passphrase, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_GET), strKey(isset_key ? keyname : ""), usePreloadedEm(isset_key ? useEm : true), strEmFile(isset_key ? emfile : ""), isNoAttr(noattr), isPass(isset_pass), strPass(passphrase), strOutputFile(file) {}
		virtual ~PromptCommandGet() override {}
};

class PromptCommandRemove : public PromptCommandBase
{
	protected:
		std::string	strKey{""};
		bool		usePreloadedEm{false};
		std::string	strEmFile{""};
		bool		isMainKeyOnly{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandRemove(const PromptArgList& args) : PromptCommandBase(args, CommandId::CMDID_PROMPT_REMOVE), strKey(""), usePreloadedEm(true), strEmFile(""), isMainKeyOnly(false) {}
		explicit PromptCommandRemove(const PromptArgList& args, bool isset_key, const std::string& keyname, bool useEm, const std::string& emfile, bool mainkeyonly) : PromptCommandBase(args, CommandId::CMDID_PROMPT_GET), strKey(isset_key ? keyname : ""), usePreloadedEm(isset_key ? useEm : true), strEmFile(isset_key ? emfile : ""), isMainKeyOnly(mainkeyonly) {}
		virtual ~PromptCommandRemove() override {}
};

class PromptCommandReadSummary : public PromptCommandBase
{
	protected:
		std::string	strK2hKey{""};
		bool		isNoAttr{false};
		bool		isPass{false};
		std::string	strPass{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandReadSummary(const PromptArgList& args, const std::string& key, bool noattr, bool isset_pass, std::string& passphrase) : PromptCommandBase(args, CommandId::CMDID_PROMPT_READSUMMARY), strK2hKey(key), isNoAttr(noattr), isPass(isset_pass), strPass(passphrase) {}
		virtual ~PromptCommandReadSummary() override {}
};

class PromptCommandSearch : public PromptCommandBase
{
	protected:
		bool		isScore{false};
		float		Score{0.0f};
		std::string	strEmFile{""};				// [NOTE] If not set(empty), Preloaded embedding data is used.
		std::string	strOutputFile{""};
		bool		isNoAttr{false};
		bool		isPass{false};
		std::string	strPass{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandSearch(const PromptArgList& args, bool is_score, float score_value, const std::string& em, const std::string& output, bool noattr, bool isset_pass, std::string& passphrase) : PromptCommandBase(args, CommandId::CMDID_PROMPT_SEARCH), isScore(is_score), Score(is_score ? score_value : 0.0f), strEmFile(em), strOutputFile(output), isNoAttr(noattr), isPass(isset_pass), strPass(passphrase) {}
		virtual ~PromptCommandSearch() override {}
};

class PromptCommandLoadProjection : public PromptCommandBase
{
	protected:
		std::string	strPmFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandLoadProjection(const PromptArgList& args, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_LOADPM), strPmFile(file) {}
		virtual ~PromptCommandLoadProjection() override {}
};

class PromptCommandLoadEmbedding : public PromptCommandBase
{
	protected:
		std::string	strEmFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandLoadEmbedding(const PromptArgList& args, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_LOADEM), strEmFile(file) {}
		virtual ~PromptCommandLoadEmbedding() override {}
};

class PromptCommandCreateProjection : public PromptCommandBase
{
	protected:
		int			dimension{0};
		int			bitsize{0};
		std::string	strOutputFile{""};
		std::string	strName{""};
		std::string	strVersion{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandCreateProjection(const PromptArgList& args, int nDimension, int nBitsize, const std::string& strPmFile, const std::string& strPmName, const std::string& strPmVer) : PromptCommandBase(args, CommandId::CMDID_PROMPT_CREATEPM), dimension(nDimension), bitsize(nBitsize), strOutputFile(strPmFile), strName(strPmName), strVersion(strPmVer) {}
		virtual ~PromptCommandCreateProjection() override {}
};

class PromptCommandCreateEmbedding : public PromptCommandBase
{
	protected:
		int			dimension{0};
		std::string	strOutputFile{""};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandCreateEmbedding(const PromptArgList& args, int nDimension, const std::string& file) : PromptCommandBase(args, CommandId::CMDID_PROMPT_CREATEEM), dimension(nDimension), strOutputFile(file) {}
		virtual ~PromptCommandCreateEmbedding() override {}
};

class PromptCommandCalBinaryHash : public PromptCommandBase
{
	protected:
		std::string	strEmFile{""};				// [NOTE] If not set(empty), Preloaded embedding data is used.
		std::string	strPmFile{""};				// [NOTE] If not set(empty), Preloaded Projection Matrix is used.

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandCalBinaryHash(const PromptArgList& args, const std::string& em, const std::string& pm) : PromptCommandBase(args, CommandId::CMDID_PROMPT_CALBH), strEmFile(em), strPmFile(pm) {}
		virtual ~PromptCommandCalBinaryHash() override {}
};

class PromptCommandCalCosineSimilarity : public PromptCommandBase
{
	protected:
		std::string	strK2hKey{""};
		bool		isNoAttr{false};
		bool		isPass{false};
		std::string	strPass{""};
		std::string	strEmFile{""};				// [NOTE] If not set(empty), Preloaded embedding data is used.
		bool		isSimple{false};

	protected:
		bool RawExecute(bool& isTerminate) override;

	public:
		static void PrintHelp();

		explicit PromptCommandCalCosineSimilarity(const PromptArgList& args, const std::string& key, bool noattr, bool isset_pass, std::string& passphrase, const std::string& em, bool simple) : PromptCommandBase(args, CommandId::CMDID_PROMPT_CALCOS), strK2hKey(key), isNoAttr(noattr), isPass(isset_pass), strPass(passphrase), strEmFile(em), isSimple(simple) {}
		virtual ~PromptCommandCalCosineSimilarity() override {}
};

#endif	// K2HVEC_CLI_PROMPTCMD_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
