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
#include <iostream>
#include <cstdarg>
#include <cstring>
#include <stdexcept>

#include "k2hvecmath.h"
#include "k2hvecbuiltinpm.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"
#include "k2hvec_tools_common.h"

//---------------------------------------------------------
// Option parser etc
//---------------------------------------------------------
//
// K2HVecGenOpt class
//
class K2HVecGenOpt
{
	private:
		static constexpr const char*	OPT_LONG_NAME_HELP			= "--help";
		static constexpr const char*	OPT_LONG_NAME_BITSIZE		= "--bit_size";
		static constexpr const char*	OPT_LONG_NAME_DIMENSION		= "--dimension_size";
		static constexpr const char*	OPT_LONG_NAME_NAME			= "--name";
		static constexpr const char*	OPT_LONG_NAME_VERSION		= "--version";
		static constexpr const char*	OPT_LONG_NAME_OUTPUT		= "--output";
		static constexpr const char*	OPT_LONG_NAME_DEBUG			= "--debug";
		static constexpr const char*	OPT_LONG_NAME_SCMODE		= "--sourcecode";

		static constexpr const char*	OPT_SHORT_NAME_HELP			= "-h";
		static constexpr const char*	OPT_SHORT_NAME_BITSIZE		= "-bs";
		static constexpr const char*	OPT_SHORT_NAME_DIMENSION	= "-ds";
		static constexpr const char*	OPT_SHORT_NAME_NAME			= "-n";
		static constexpr const char*	OPT_SHORT_NAME_VERSION		= "-v";
		static constexpr const char*	OPT_SHORT_NAME_OUTPUT		= "-o";
		static constexpr const char*	OPT_SHORT_NAME_DEBUG		= "-d";
		static constexpr const char*	OPT_SHORT_NAME_SCMODE		= "-s";

		static constexpr const char*	DEFAULT_BUILTIN_NAME		= "builtin";
		static constexpr const char*	DEFAULT_CUSTOM_NAME			= "pm";
		static constexpr const char*	DEFAULT_VERSION				= "1.0";
		static constexpr int			DEFAULT_BITSIZE				= 48;
		static constexpr int			DEFAULT_DIMENSION			= 4096;

		bool Parse(int argc, const char** argv);

	public:
		bool		error;
		bool		help;
		int			bitsize;
		int			dimension;
		std::string	name;
		std::string	version;
		std::string	file;
		bool		debug;
		bool		builtin;

		K2HVecGenOpt(int argc, const char** argv);
		~K2HVecGenOpt() {}

		K2HVecGenOpt() = delete;
		K2HVecGenOpt(const K2HVecGenOpt&) = delete;
		K2HVecGenOpt& operator=(const K2HVecGenOpt&) = delete;
		K2HVecGenOpt& operator=(K2HVecGenOpt&&) = delete;
};

K2HVecGenOpt::K2HVecGenOpt(int argc, const char** argv): error(false), help(false), bitsize(-1), dimension(-1), name(""), version(""), file(""), debug(false), builtin(false)
{
	if(!Parse(argc, argv)){
		error = true;
	}
}

bool K2HVecGenOpt::Parse(int argc, const char** argv)
{
	if(argc < 1 || !argv){
		ERR("parameters(argc, argv) are wrong.");
		return false;
	}

	for(int pos = 1; pos < argc; ++pos){
		std::string	oneArg = VECEMPTYSTR(argv[pos]) ? "" : argv[pos];

		if(oneArg.empty()){
			continue;		// Why?

		}else if(OPT_LONG_NAME_HELP == oneArg || OPT_SHORT_NAME_HELP == oneArg){
			// --help option
			if(help){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			help = true;

		}else if(OPT_LONG_NAME_BITSIZE == oneArg || OPT_SHORT_NAME_BITSIZE == oneArg){
			// --bit_size option
			if(-1 != bitsize){
				ERR("Already specified %s option with value(%d).", oneArg.c_str(), bitsize);
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			try{
				int	paramval = std::stoi(strParam);
				if(paramval < 16 || 512 < paramval){
					ERR("Specified %s option parameter must be 16-512 range(%s).", oneArg.c_str(), strParam.c_str());
					return false;
				}
				bitsize = paramval;
			}catch(const std::invalid_argument& e){
				ERR("Specified %s option parameter is not number(%s).", oneArg.c_str(), strParam.c_str());
				return false;
			}catch(const std::out_of_range& e){
				ERR("Specified %s option parameter is over int range(%s).", oneArg.c_str(), strParam.c_str());
				return false;
			}

		}else if(OPT_LONG_NAME_DIMENSION == oneArg || OPT_SHORT_NAME_DIMENSION == oneArg){
			// --dimension_size option
			if(-1 != dimension){
				ERR("Already specified %s option with value(%d).", oneArg.c_str(), dimension);
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			try{
				int	paramval = std::stoi(strParam);
				if(paramval < 512 || 8192 < paramval){
					ERR("Specified %s option parameter must be 512-8192 range(%s).", oneArg.c_str(), strParam.c_str());
					return false;
				}
				dimension = paramval;
			}catch(const std::invalid_argument& e){
				ERR("Specified %s option parameter is not number(%s).", oneArg.c_str(), strParam.c_str());
				return false;
			}catch(const std::out_of_range& e){
				ERR("Specified %s option parameter is over int range(%s).", oneArg.c_str(), strParam.c_str());
				return false;
			}

		}else if(OPT_LONG_NAME_NAME == oneArg || OPT_SHORT_NAME_NAME == oneArg){
			// --name option
			if(!name.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), name.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			name = strParam;

		}else if(OPT_LONG_NAME_VERSION == oneArg || OPT_SHORT_NAME_VERSION == oneArg){
			// --output option
			if(!version.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), file.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			version = strParam;

		}else if(OPT_LONG_NAME_OUTPUT == oneArg || OPT_SHORT_NAME_OUTPUT == oneArg){
			// --output option
			if(!file.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), file.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			file = strParam;

		}else if(OPT_LONG_NAME_DEBUG == oneArg || OPT_SHORT_NAME_DEBUG == oneArg){
			// --debug option
			if(debug){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			debug = true;

		}else if(OPT_LONG_NAME_SCMODE == oneArg || OPT_SHORT_NAME_SCMODE == oneArg){
			// --sourcecode option
			if(builtin){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			builtin = true;

		}else{
			ERR("Unkown option(%s).", oneArg.c_str());
			return false;
		}
	}

	if(-1 == bitsize){
		bitsize = DEFAULT_BITSIZE;
	}
	if(-1 == dimension){
		dimension = DEFAULT_DIMENSION;
	}
	if(name.empty()){
		if(builtin){
			name = DEFAULT_BUILTIN_NAME;
		}else{
			name = DEFAULT_CUSTOM_NAME;
		}
	}
	if(version.empty()){
		version = DEFAULT_VERSION;
	}

	return true;
}

//
// Print Usage
//
static void Help(const std::string& progname)
{
	PRN("");
	PRN("Generate Projection Matrix data tool");
	PRN("");
	PRN("Usage: %s --bit_size(-bs) <number> --dimension_size(-ds) <number> --name(-n) <string> --version(-v) <string> --output(-o) <file path> --debug(-d) --sourcecode(-s)", progname.c_str());
	PRN("       %s --help(-h)", progname.c_str());
	PRN("");
	PRN("Options:");
	PRN("  --help(-h)                      display this usage.");
	PRN("  --bit_size(-bs) <number>        specify the number of bits(size) in the range of 16 to 512(default: 48)");
	PRN("  --dimension_size(-ds) <number>  specify the number of dimension in the range of 512 to 8192(default: 4096)");
	PRN("  --name(-n) <string>             specify Projection Matrix array name(default: \"pm\")");
	PRN("  --version(-v) <string>          specify Projection Matrix array version(default: \"1.0\")");
	PRN("  --output(-o) <file path>        specify output file(this option is not specified, put stdout)");
	PRN("  --debug(-d)                     print debug messages");
	PRN("  --sourcecode(-s)                specify output format for source code(use internal developer only)");
	PRN("");
}

//---------------------------------------------------------
// Main
//---------------------------------------------------------
int main(int argc, char** argv)
{
	// program name
	std::string	progname = programname(argv[0]);

	// parse parameters
	K2HVecGenOpt	opts(argc, const_cast<const char**>(&argv[0]));
	if(opts.error){
		exit(EXIT_FAILURE);
	}

	// help
	if(opts.help){
		Help(progname);
		exit(EXIT_SUCCESS);
	}

	// Set debug environments and options
	if(!LoadK2hvecDbgEnv()){
		exit(EXIT_FAILURE);
	}
	if(opts.debug){
		SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_DUMP);
	}

	// Print processing information
	PRN("-----------------------------------------------------------");
	PRN("Infomration : Generate Projection Matrix");
	PRN("-----------------------------------------------------------");
	PRN("Name                : %s",		opts.name.c_str());
	PRN("Version             : %s",		opts.version.c_str());
	PRN("Bit count(size)     : %d bit",	opts.bitsize);				// already validate(see. K2HVecGenOpt::Parse())
	PRN("Dimension size      : %d",		opts.dimension);			// already validate(see. K2HVecGenOpt::Parse())
	if(!opts.file.empty()){
		PRN("Output file path    : %s",		opts.file.c_str());
	}
	if(opts.builtin){
		PRN("C source code type  : yes");
	}
	PRN("-----------------------------------------------------------");
	PRN("");

	// Start measurement
	LapTime	duration(true);

	// Create Projection matrix
	if(opts.file.empty()){
		// stdout
		std::string	strProMatrix = ProjectionMatrix::CreateProjectionMatrixAsCArray(opts.bitsize, opts.dimension, opts.name.c_str(), opts.version.c_str(), opts.builtin);
		std::cout << strProMatrix << std::endl;
		PRN("-----------------------------------------------------------");
	}else{
		// file
		if(!ProjectionMatrix::CreateProjectionMatrixFileAsCArray(opts.file.c_str(), opts.bitsize, opts.dimension, opts.name.c_str(), opts.version.c_str(), opts.builtin)){
			ERR("Something error occurred.");
			exit(EXIT_FAILURE);
		}
	}

	// Print end message
	std::string	strDuration = duration.GetDurationString();
	PRN("[SUCCESS] Projection Matrix[%d * %d] - Calculation time : %s", opts.bitsize, opts.dimension, strDuration.c_str());
	PRN("-----------------------------------------------------------");

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
