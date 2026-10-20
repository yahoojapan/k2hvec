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

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <utility>

#include <termios.h>
#include <unistd.h>

#include "k2hvecutil.h"
#include "k2hvec_tools_common.h"
#include "k2hvec_cli_input.h"
#include "k2hvec_cli_cmd.h"
#include "k2hvec_cli_promptcmd.h"

//---------------------------------------------------------
// Utilities
//---------------------------------------------------------
static std::string GetProgname(const char* argv)
{
	if(VECEMPTYSTR(argv)){
		return "k2hveclinetool";		// default
	}

	std::string	tmpPath(argv);
	size_t		pos = tmpPath.find_last_of('/');
	if(std::string::npos != pos){
		return tmpPath.substr(pos + 1);
	}
	return tmpPath;
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------
int main(int argc, char** argv)
{
	std::string					strProgname = GetProgname(argv[0]);
	std::vector<std::string>	Argvs(argv + 1, argv + argc);
	ConsoleInput				ConsoleObj;
	K2hvecCommand				CmdObj;

	// For laptime
	LapTime	duration(true);

	// Setup Console
	ConsoleObj.SetPrompt("K2HVEC > ");
	PromptCommandBase::ConsoleInputScope	ConsoleScope(CmdObj, ConsoleObj);	// RAII for K2hvecCommand and ConsoleInput pointer in PromptCommandBase

	// Parse command line
	if(!CmdObj.ParseExecArgs(Argvs, strProgname)){
		ERR("Invalid startup options or parameters. Please check the options using --help(-h).");
		exit(EXIT_FAILURE);
	}

	// Run command(--help, --version)
	if(CmdObj.RunEarlyExitProcess(strProgname)){
		exit(EXIT_SUCCESS);
	}

	// Set process configuration(for console)
	if(0 < CmdObj.GetHistorySize()){
		ConsoleObj.SetMax(CmdObj.GetHistorySize());
	}
	if(CmdObj.IsLapTime()){
		PromptCommandBase::EnableLapTime();
	}

	// Setup debug mode/file from environment
	k2h_load_debug_env();
	LoadK2hvecDbgEnv();

	// Initialize K2hVec object
	if(!CmdObj.InitializeK2hVec()){
		exit(EXIT_SUCCESS);
	}

	// Execute prompt command on startup
	bool isTerminate		= false;
	bool precommand_result	= CmdObj.RunPromptCommands(isTerminate);

	// Exit at direct prompt mode or Wait console input
	if(isTerminate || CmdObj.IsDirectPromptMode()){
		exit(precommand_result ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	// Print welcom message
	CmdObj.PrintLaunchMessage();

	//
	// Prompt Command Loop
	//
	PromptCommandList	PromptCommands;
	do{
		// Get command line on console
		std::string		CmdLine;
		if(!ConsoleObj.GetCommandLine(CmdLine)){
			continue;				// not break
		}

		// Set arguments
		Argvs.clear();
		Argvs.emplace_back(std::move(CmdLine));

		// Make prompt commands
		PromptCommands.clear();
		if(!PromptCommandBase::MakePromptCommandList(Argvs, false, PromptCommands)){
			continue;				// not break
		}

		// Execute commands
		for(const auto& pPromptCmdObj: PromptCommands){
			// Execute one command
			if(!pPromptCmdObj->Execute(isTerminate)){
				// failed but continue...
				ERR("Failed to run \"%s\" command: \"%s\"", pPromptCmdObj->GetCommandName().c_str(), pPromptCmdObj->GetCommandLineArgs().c_str());
			}
			// Check break for teminate
			if(isTerminate){
				break;
			}
		}
	}while(!isTerminate);

	// Goodbye message
	if(CmdObj.IsWelcomeMessage()){
		if(CmdObj.IsLapTime()){
			std::string	strDuration = duration.GetDurationString();
			PRN("Exiting : Execution time - %s\n", strDuration.c_str());
		}else{
			PRN("Exiting...");
		}
	}

	exit(EXIT_SUCCESS);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
