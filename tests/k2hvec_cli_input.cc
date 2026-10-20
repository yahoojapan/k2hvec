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
#include <iostream>
#include <fstream>
#include <cctype>
#include <cerrno>

#include <unistd.h>
#include <termios.h>

#include "k2hvec_tools_common.h"
#include "k2hvec_cli_input.h"
#include "k2hvec_cli_cmd.h"				// for StringToInt()
#include "k2hvecutil.h"

//---------------------------------------------------------
// ConsoleInput Methods : Constructor/Destructor
//---------------------------------------------------------
ConsoleInput::~ConsoleInput()
{
	UnsetTerminal();
	Clean();
}

//---------------------------------------------------------
// ConsoleInput Methods
//---------------------------------------------------------
bool ConsoleInput::Clean()
{
	history.clear();
	prompt.clear();
	input.clear();
	history_pos = -1;
	input_pos   = 0;
	return true;
}

size_t ConsoleInput::SetMax(size_t max)
{
	size_t	old = history_max;
	history_max = max;

	if(0 == history_max){
		history.clear();
	}else if(static_cast<size_t>(history_max) < history.size()){
		history.erase(history.begin(), history.end() - static_cast<size_t>(history_max));
	}
	return old;
}

bool ConsoleInput::SetPrompt(const std::string& strPrompt)
{
	if(strPrompt.empty()){
		return false;
	}
	prompt = strPrompt;
	return true;
}

bool ConsoleInput::SetTerminal()
{
	if(is_set_terminal){
		// already set
		return true;
	}

	// backup current termios attributes
	if(0 != tcgetattr(STDIN_FILENO, &tty_backup)){
		last_errno = errno;
		return false;
	}

	// setup new attribute
	//
	// [NOTE]
	// Allow(keep) Ctrl-Z(SIGTSTP) and Ctrl-\ (SIGQUIT)...
	//
	struct termios	tty_change	= tty_backup;
	tty_change.c_lflag			&= ~(ECHO | ICANON);
	tty_change.c_cc[VMIN]		= _POSIX_VDISABLE;
	tty_change.c_cc[VTIME]		= 1;
	tty_change.c_cc[VINTR]		= _POSIX_VDISABLE;		// Disable Ctrl-C (SIGINT)

	// set new termios
	if(0 != tcsetattr(STDIN_FILENO, TCSAFLUSH, &tty_change)){
		last_errno = errno;
		return false;
	}
	is_set_terminal = true;

	return true;
}

bool ConsoleInput::UnsetTerminal()
{
	if(!is_set_terminal){
		// already unset
		return true;
	}

	// unset temios from backup
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tty_backup);
	is_set_terminal = false;

	return true;
}

//
// If error occurred, return 0x00
//
bool ConsoleInput::ReadByte(char& cInput)
{
	// read one charactor
	cInput		= '\0';
	ssize_t	res = read(STDIN_FILENO, &cInput, sizeof(char));
	if(-1 == res){
		last_errno = errno;
		return false;
	}
	last_errno = 0;

	return true;
}

void ConsoleInput::ClearInput()
{
	history_pos = -1;
	input_pos   = 0;
	last_errno  = 0;
	input.clear();
}

void ConsoleInput::ClearLine()
{
	for(size_t pos = 0; pos < input_pos; ++pos){		// cursor to head
		std::putchar('\x08');
	}
	for(size_t pos = 0; pos < input.length(); ++pos){	// clear with space
		std::putchar(' ');
	}
	for(size_t pos = 0; pos < input.length(); ++pos){	// rewind to head
		std::putchar('\x08');
	}
	std::fflush(stdout);
}

bool ConsoleInput::GetInputLine()
{
	static const bool	isTerm	= ::isatty(STDOUT_FILENO) ? true : false;
	static const char*	CCYN	= "\033[36m";								// Color Cyan
	static const char*	CDEF	= "\033[0m";								// To default

	ClearInput();
	if(!SetTerminal()){
		return false;
	}

	// Print prompt
	if(isTerm){
		std::fputs(CCYN, stdout);
		std::fputs(prompt.c_str(), stdout);
		std::fputs(CDEF, stdout);
	}else{
		std::fputs(prompt.c_str(), stdout);
	}
	std::fflush(stdout);

	//
	// [Input key value]		[Processing]
	//	0x1b 0x5b 0x41			Up
	//	0x1b 0x5b 0x42			Down
	//	0x1b 0x5b 0x43			Right
	//	0x1b 0x5b 0x44			Left
	//	0x7f					Delete
	//	0x08					BackSpace
	//	0x01					Ctrl-A
	//	0x05					Ctrl-E
	//	0x1b 0x5b 0x31 0x7e		Home
	//	0x1b 0x5b 0x34 0x7e		End
	//	0x1b 0x5b 0x33 0x7e		BackSpace (on OSX)
	//
	char	input_char;
	while(true){
		// Read one character
		if(!ReadByte(input_char)){
			if(EINTR == last_errno){
				last_errno = 0;
				continue;
			}
			break;															// error or EOF
		}

		// Check control code
		if('\n' == input_char || '\r' == input_char){						// Line break
			// End of line
			std::putchar('\n');
			std::fflush(stdout);
			break;															// [NOTE] Not put history yet.

		}else if('\x1b' == input_char){										// Escape sequences(Arrow keys, Home/End keys) --> read next charactor
			if(!ReadByte(input_char)){
				break;
			}

			if('\x5b' == input_char){										// '[' key --> read more charactor
				if(!ReadByte(input_char)){
					break;
				}
				switch(input_char){
					case '\x41':											// Up key (backward history)
						if(0 != history_pos && !history.empty()){
							ClearLine();

							if(-1 == history_pos){
								history_pos = static_cast<ssize_t>(history.size() - 1);
							}else if(0 < history_pos){
								history_pos--;
							}
							input		= history[history_pos];
							input_pos	= input.length();

							std::fputs(input.c_str(), stdout);
							std::fflush(stdout);
						}
						break;

					case '\x42':											// Down key (forward history)
						if(-1 != history_pos){
							ClearLine();

							if(history.size() <= static_cast<size_t>(history_pos + 1)){
								history_pos	= -1;
								input.clear();
								input_pos	= 0;
							}else{
								history_pos++;
								input		= history[history_pos];
								input_pos	= input.length();

								std::fputs(input.c_str(), stdout);
								std::fflush(stdout);
							}
						}
						break;

					case '\x43':											// Right key (move cursor to right)
						if(input_pos < input.length()){
							std::putchar(input[input_pos]);
							std::fflush(stdout);
							input_pos++;
						}
						break;

					case '\x44':											// Left key (move cursor to left)
						if(0 < input_pos){
							input_pos--;
							std::putchar('\x08');
							std::fflush(stdout);
						}
						break;

					case '\x31':											// Home key (move cursor to head)
						if(ReadByte(input_char) && '\x7e' == input_char){
							for(size_t pos = 0; pos < input_pos; ++pos){
								std::putchar('\x08');
							}
							input_pos = 0;
							std::fflush(stdout);
						}
						break;

					case '\x34':											// End key (move cursor to end)
						if(ReadByte(input_char) && '\x7e' == input_char){
							for(size_t pos = input_pos; pos < input.length(); ++pos){
								std::putchar(input[pos]);
							}
							input_pos = input.length();
							std::fflush(stdout);
						}
						break;

					case '\x33':											// BackSpace key on OSX
						if(ReadByte(input_char) && '\x7e' == input_char){
							if(0 < input_pos){
								input.erase(input_pos - 1, 1);								// erase left charactor
								input_pos--;

								std::putchar('\x08');										// move cursor to left
								for(size_t pos = input_pos; pos < input.length(); ++pos){	// put characters following the deleted character
									std::putchar(input[pos]);
								}
								std::putchar(' ');											// overwrite lastest charactor pos with space
								for(size_t pos = input_pos; pos <= input.length(); ++pos){	// move cursor to the left using '\x08' by the distance advanced
									std::putchar('\x08');
								}
								std::fflush(stdout);
							}
						}
						break;
					}
			}

		}else if('\x7f' == input_char){										// Delete key
			if(input_pos < input.length()){
				input.erase(input_pos, 1);									// erase cursor charactor

				for(size_t pos = input_pos; pos < input.length(); ++pos){	// put characters following the deleted character
					std::putchar(input[pos]);
				}
				std::putchar(' ');											// overwrite lastest charactor pos with space
				for(size_t pos = input_pos; pos <= input.length(); ++pos){	// move cursor to the left using '\x08' by the distance advanced
					std::putchar('\x08');
				}
				std::fflush(stdout);
			}

		}else if('\x08' == input_char){										// Backspace key
			if(0 < input_pos){
				input.erase(input_pos - 1, 1);								// erase left charactor
				input_pos--;

				std::putchar('\x08');										// move cursor to left
				for(size_t pos = input_pos; pos < input.length(); ++pos){	// put characters following the deleted character
					std::putchar(input[pos]);
				}
				std::putchar(' ');											// overwrite lastest charactor pos with space
				for(size_t pos = input_pos; pos <= input.length(); ++pos){	// move cursor to the left using '\x08' by the distance advanced
					std::putchar('\x08');
				}
				std::fflush(stdout);
			}

		}else if('\x01' == input_char){										// Ctrl-A (move cursor to head)
			for(size_t pos = 0; pos < input_pos; ++pos){
				std::putchar('\x08');
			}
			input_pos = 0;
			std::fflush(stdout);

		}else if('\x03' == input_char){										// Ctrl-C (stop input current line, and start to input new line)
			// print Ctrl-C string and new line
			std::fputs("^C\n", stdout);
			std::fflush(stdout);

			// re-print prompt and continue to input new line
			if(isTerm){
				std::fputs(CCYN, stdout);
				std::fputs(prompt.c_str(), stdout);
				std::fputs(CDEF, stdout);
			}else{
				std::fputs(prompt.c_str(), stdout);
			}
			std::fflush(stdout);

			// clear
			input.clear();
			input_pos = input.length();

		}else if('\x04' == input_char){										// Ctrl-D (exit command)
			// print Ctrl-D string
			std::fputs("^D\n", stdout);
			std::fflush(stdout);

			// set "exit" command
			input		= "exit";
			input_pos	= input.length();
			break;

		}else if('\x05' == input_char){										// Ctrl-E (move cosor to end)
			for(size_t pos = input_pos; pos < input.length(); ++pos){
				std::putchar(input[pos]);
			}
			input_pos = input.length();
			std::fflush(stdout);

		}else if(std::isprint(static_cast<unsigned char>(input_char))){		// Normal charactor
			input.insert(input_pos, 1, input_char);
			for(size_t pos = input_pos; pos < input.length(); ++pos){
				std::putchar(input[pos]);
			}
			input_pos++;
			for(size_t pos = input_pos; pos < input.length(); ++pos){
				std::putchar('\x08');
			}
			std::fflush(stdout);
		}
	}
	UnsetTerminal();

	return (0 == last_errno);
}

bool ConsoleInput::GetCommandLine(std::string& line)
{
	if(!GetInputLine()){
		ERR("Something error occurred while reading stdin: err(%d).", LastErrno());
		return false;
	}

	// Check history keyword
	if(1 < input.size() && '!' == input[0]){
		// Start '!' charactor
		if("!!" == input){
			// '!!' is history(-1)
			history_pos = -1;
		}else{
			// Check '!<number>'
			std::string	strNum(input.begin() + 1, input.end());
			int			tmpNum = 0;
			if(!StringToInt(strNum, tmpNum) || 0 == tmpNum){
				ERR("A numeric value is required after the history command(\"!\"), anything other than a number results in an error.(allow only \"!!\"): %s", input.c_str());
				return false;
			}
			history_pos = static_cast<ssize_t>(tmpNum);
		}

		// Make history position
		ssize_t	history_list_pos = 0;
		if(0 == history_pos){
			ERR("There is no history for the specified position: %s", input.c_str());
			return false;
		}else if(history_pos < 0){
			history_list_pos = static_cast<ssize_t>(history.size()) - std::abs(history_pos);
		}else{
			history_list_pos = history_pos - 1;
		}
		if(history_list_pos < 0 || static_cast<ssize_t>(history.size()) <= history_list_pos){
			ERR("There is no history for the specified position: %ld(%s)", history_pos, input.c_str());
			return false;
		}

		// Replace command
		input = history[history_list_pos];
	}

	// push to history.
	PutHistory(input);

	// set command line
	line = input;

	return true;
}

bool ConsoleInput::PutHistory(const std::string& commandline)
{
	if(commandline.empty()){
		return false;
	}
	history.emplace_back(commandline);
	if(history_max < history.size()){
		history.erase(history.begin());
	}
	return true;
}

bool ConsoleInput::RemoveLastHistory()
{
	if(!history.empty()){
		history.pop_back();
	}
	return true;
}

void ConsoleInput::PrintAllHistory() const
{
	int	history_no = 1;						// history no. starts "1"
	for(const auto& oneline: history){
		PRN(" %d  %s", history_no++, oneline.c_str());
	}
}

bool ConsoleInput::PutAllHistory(const std::string& strFile, bool put_last_line) const
{
	if(strFile.empty()){
		ERR("Output file path is empty.");
		return false;
	}

	std::ofstream	ofs(strFile);
	if(!ofs){
		ERR("Could not create file: %s", strFile.c_str());
		return false;
	}
	size_t	his_count	= history.size() - 1;
	size_t	his_pos		= 0;
	for(const auto& line: history){
		if(!put_last_line && his_pos == his_count){
			break;
		}
		ofs << line << '\n';
		++his_pos;
	}
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
