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

#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdarg>

#include "k2hvecmath.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"
#include "k2hvec_tools_common.h"

//---------------------------------------------------------
// Option parser etc
//---------------------------------------------------------
//
// K2HVecTestBhOpt class
//
class K2HVecTestBhOpt
{
	private:
		static constexpr const char*	OPT_LONG_NAME_HELP		= "--help";
		static constexpr const char*	OPT_LONG_NAME_DEBUG		= "--debug";
		static constexpr const char*	OPT_LONG_NAME_SIMPLE	= "--simple";

		static constexpr const char*	OPT_SHORT_NAME_HELP		= "-h";
		static constexpr const char*	OPT_SHORT_NAME_DEBUG	= "-d";
		static constexpr const char*	OPT_SHORT_NAME_SIMPLE	= "-s";

		bool Parse(int argc, const char** argv);

	public:
		bool		error;
		bool		help;
		bool		debug;
		bool		simple;

		K2HVecTestBhOpt(int argc, const char** argv);
		~K2HVecTestBhOpt() {}

		K2HVecTestBhOpt() = delete;
		K2HVecTestBhOpt(const K2HVecTestBhOpt&) = delete;
		K2HVecTestBhOpt& operator=(const K2HVecTestBhOpt&) = delete;
		K2HVecTestBhOpt& operator=(K2HVecTestBhOpt&&) = delete;
};

K2HVecTestBhOpt::K2HVecTestBhOpt(int argc, const char** argv): error(false), help(false), debug(false), simple(false)
{
	if(!Parse(argc, argv)){
		error = true;
	}
}

bool K2HVecTestBhOpt::Parse(int argc, const char** argv)
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

		}else if(OPT_LONG_NAME_DEBUG == oneArg || OPT_SHORT_NAME_DEBUG == oneArg){
			// --debug option
			if(debug){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			debug = true;

		}else if(OPT_LONG_NAME_SIMPLE == oneArg || OPT_SHORT_NAME_SIMPLE == oneArg){
			// --simple option
			if(simple){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			simple = true;

		}else{
			ERR("Unkown command or option(%s).", oneArg.c_str());
			return false;
		}
	}
	return true;
}

//
// Print Usage
//
static void Help(const std::string& progname)
{
	PRN("");
	PRN("Test for Cosine Similarity Calculation");
	PRN("");
	PRN("Usage: %s --simple(-s) --debug(-d)", progname.c_str());
	PRN("Usage: %s --help(-h)", progname.c_str());
	PRN("");
	PRN("Options:");
	PRN("  --help(-h)           display this usage.");
	PRN("  --debug(-d)          print debug messages");
	PRN("  --simple(-s)specify  specify calculation of simplified cosine similarity for normalized vector data");
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
	K2HVecTestBhOpt	opts(argc, const_cast<const char**>(&argv[0]));
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

	// Prepare embedding vector data
	int					length	= 4096;									// for test
	std::vector<float>	data1	= GenerateEmbedding(length);
	std::vector<float>	data2	= GenerateEmbedding(length);

	// Start measurement
	LapTime	duration(true);

	// Calculation of Cosine Similarity
	float				result	= CosineSimilarity(data1, data2, opts.simple);

	// Stop measurement
	std::string	strDuration	= duration.GetDurationString();

	// Print result
	PRN("-----------------------------------------------------------");
	PRN("Test Calculation of Cosine Similarity");
	PRN("-----------------------------------------------------------");
	PRN("Test Cosine Similarity     : %s", opts.simple ? "simple" : "standard");
	PRN("Test float data length     : %d", length);
	PRN("-----------------------------------------------------------");
	PRN("Result                     : %.10g", result);
	PRN("-----------------------------------------------------------");
	PRN("[SUCCESS] Calculation time : %s", strDuration.c_str());
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
