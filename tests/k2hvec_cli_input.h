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

#ifndef	K2HVEC_CLI_INPUT_H
#define	K2HVEC_CLI_INPUT_H

#include <string>
#include <vector>
#include <cstddef>
#include <sys/types.h>
#include <termios.h>

//---------------------------------------------------------
// ConsoleInput Class
//---------------------------------------------------------
//
// This class handles console input, so this assists with input from the prompt.
// It also manages input history.
//
class ConsoleInput
{
	protected:
		static constexpr size_t 	DEFAULT_HISTORY_MAX = 500;

		std::string					prompt{"> "};							// Command prompt string
		size_t						history_max{DEFAULT_HISTORY_MAX};		// History size
		std::vector<std::string>	history{};								// History list
		ssize_t						history_pos{-1};						// Position in history list(for up/down key)
		std::string					input{""};								// One command line
		size_t						input_pos{0};							// Cursor position
		struct termios				tty_backup{};							// Backup for termios attribute
		bool						is_set_terminal{false};					// Status of set/upset termios attriutes
		int							last_errno{0};							// Lastest errno

	protected:
		bool Clean();
		bool SetTerminal();
		bool UnsetTerminal();
		bool ReadByte(char& cInput);
		void ClearInput();
		void ClearLine();

		bool GetInputLine();
		bool PutHistory(const std::string& commandline);
		bool RemoveLastHistory();

		int LastErrno() const { return last_errno; }

	public:
		ConsoleInput() = default;
		virtual ~ConsoleInput();

		size_t SetMax(size_t max);
		size_t GetMax(void) const { return history_max; }
		bool SetPrompt(const std::string& strPrompt);

		bool GetCommandLine(std::string& line);

		void PrintAllHistory() const;
		bool PutAllHistory(const std::string& strFile, bool put_last_line = true) const;
};

#endif	// K2HVEC_CLI_INPUT_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
