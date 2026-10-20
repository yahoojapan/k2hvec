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
#include <climits>
#include <fstream>
#include <sstream>
#include <random>
#include <ctime>
#include <unistd.h>
#include <libgen.h>
#include <sys/random.h>

#include "k2hvecmath.h"
#include "k2hvecbuiltinpm.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"
#include "k2hvec_tools_common.h"

//---------------------------------------------------------
// Utility functions
//---------------------------------------------------------
//
// Get temporary file path under /tmp directory for custom
//
static std::string GenerateTempFilepath(const std::string& progname, bool filtered, bool original)
{
	// [NOTE]
	// strRandPart generates a portion of the file containing a single
	// random string for this process. Conversely, the value must remain
	// consistent. Therefore, it is initialized only once using "const".
	//
	static const std::string strRandPart = [](){
		char			szTmp[64];
		unsigned long	rand_val = 0;
		if(sizeof(rand_val) == getrandom(&rand_val, sizeof(rand_val), 0)){
			snprintf(szTmp, sizeof(szTmp), "_%lx", rand_val);
		}else{
			srandom(static_cast<unsigned int>(time(NULL)) ^ static_cast<unsigned int>(getpid()));
			snprintf(szTmp, sizeof(szTmp), "_%lx", random());
		}
		return std::string(szTmp);
	}();

	pid_t				pid		= getpid();
	std::ostringstream	ss;
	ss << "/tmp/." << progname << "_" << pid << strRandPart << (filtered ? "_filtered." : "." ) << (original ? "orginal" : "export");

	return ss.str();
}

//
// Get program base directory
//
static std::string GetExecuteBaseDir(const char* argv)
{
	char		resolved_path[PATH_MAX];

	// try to make directory path by realpath
	if(nullptr != realpath(argv, resolved_path)){
		return std::string(dirname(resolved_path));
	}else{
		// [NOTE]
		// Linux fallback for cases where realpath fails, such as when executed via PATH.
		// Reading the /proc/self/exe symbolic link.
		//
		ssize_t	length = readlink("/proc/self/exe", resolved_path, sizeof(resolved_path) - 1);
		if(-1 != length){
			resolved_path[length] = '\0';
			return std::string(dirname(resolved_path));
		}
	}
	return "";
}

//
// Get Projection Matrix source file path (lib/k2hvecbuiltinpm.cc)
//
static std::string GetProjectionMatrixSourceFilePath(const char* argv)
{
	std::string	filepath = GetExecuteBaseDir(argv);
	if(!filepath.empty()){
		// [NOTE]
		// Need to check last directory name.
		// The path might point to the `tests/.libs` directory because Libtool is being used.
		//
		if(5 <= filepath.size() && filepath.rfind(".libs") == (filepath.size() - 5)){
			filepath += "/..";
		}
		filepath += "/../lib/k2hvecbuiltinpm.cc";
    }
	return filepath;
}

//---------------------------------------------------------
// Option parser etc
//---------------------------------------------------------
//
// K2HVecTestPmOpt class
//
class K2HVecTestPmOpt
{
	private:
		static constexpr const char*	CMD_LONG_BUILTIN		= "builtin";
		static constexpr const char*	CMD_LONG_CUSTOM			= "custom";

		static constexpr const char*	CMD_SHORT_BUILTIN		= "b";
		static constexpr const char*	CMD_SHORT_CUSTOM		= "c";

		static constexpr const char*	OPT_LONG_NAME_HELP		= "--help";
		static constexpr const char*	OPT_LONG_NAME_DEBUG		= "--debug";

		static constexpr const char*	OPT_SHORT_NAME_HELP		= "-h";
		static constexpr const char*	OPT_SHORT_NAME_DEBUG	= "-d";

		bool Parse(int argc, const char** argv);

	public:
		bool		error;
		bool		help;
		bool		debug;
		bool		builtin;
		bool		custom;

		K2HVecTestPmOpt(int argc, const char** argv);
		~K2HVecTestPmOpt() {}

		K2HVecTestPmOpt() = delete;
		K2HVecTestPmOpt(const K2HVecTestPmOpt&) = delete;
		K2HVecTestPmOpt& operator=(const K2HVecTestPmOpt&) = delete;
		K2HVecTestPmOpt& operator=(K2HVecTestPmOpt&&) = delete;
};

K2HVecTestPmOpt::K2HVecTestPmOpt(int argc, const char** argv): error(false), help(false), debug(false), builtin(false), custom(false)
{
	if(!Parse(argc, argv)){
		error = true;
	}
}

bool K2HVecTestPmOpt::Parse(int argc, const char** argv)
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

		}else if(CMD_LONG_BUILTIN == oneArg || CMD_SHORT_BUILTIN == oneArg){
			// builtin command
			if(builtin){
				ERR("Already specified %s command.", oneArg.c_str());
				return false;
			}
			builtin = true;

		}else if(CMD_LONG_CUSTOM == oneArg || CMD_SHORT_CUSTOM == oneArg){
			// custom command
			if(custom){
				ERR("Already specified %s command.", oneArg.c_str());
				return false;
			}
			custom = true;

		}else{
			ERR("Unkown command or option(%s).", oneArg.c_str());
			return false;
		}
	}

	if(!help && !builtin && !custom){
		ERR("You must specify either the %s(%s) or %s(%s) command.", CMD_LONG_BUILTIN, CMD_SHORT_BUILTIN, CMD_LONG_CUSTOM, CMD_SHORT_CUSTOM);
		return false;
	}else if(!help && builtin && custom){
		ERR("The %s(%s) and %s(%s) commands cannot be specified together.", CMD_LONG_BUILTIN, CMD_SHORT_BUILTIN, CMD_LONG_CUSTOM, CMD_SHORT_CUSTOM);
		return false;
	}

	return true;
}

//
// Print Usage
//
static void Help(const std::string& progname)
{
	PRN("");
	PRN("Test for Projection Matrix loading");
	PRN("");
	PRN("Usage: %s [ builtin(b) | custom(c) ] --debug(-d)", progname.c_str());
	PRN("Usage: %s --help(-h)", progname.c_str());
	PRN("");
	PRN("Command:");
	PRN("   builtin(b)      test built-in Projection Matrix verifying");
	PRN("   custom(c)       test Custom Projection Matrix generating/loading/verifying");
	PRN("");
	PRN("Options:");
	PRN("  --help(-h)       display this usage.");
	PRN("  --debug(-d)      print debug messages");
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
	K2HVecTestPmOpt	opts(argc, const_cast<const char**>(&argv[0]));
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
	PRN("Infomration : Test for Projection Matrix loading");
	PRN("-----------------------------------------------------------");
	PRN("Target      : %s",		(opts.builtin ? "built-in Projection Matrix" : opts.custom ? "Custom Projection Matrix" : "(unknown)"));
	if(opts.builtin){
		PRN("Processing  : (1) Exports built-in Projection Matrix to temporary file");
		PRN("              (2) Compare(verify) temporary file and source code(k2hvecbuiltinpm.cc)");
	}else if(opts.custom){
		PRN("Processing  : (1) Create new Custom Projection Matrix file");
		PRN("              (2) Load Custom Projection Matrix file");
		PRN("              (3) Exports loaded Custom Projection Matrix to temporary file");
		PRN("              (4) Compare(verify) original and temporary Custom Projection Matrix file");
	}
	PRN("-----------------------------------------------------------");
	PRN("");

	// Start measurement
	LapTime	duration(true);

	// File paths
	std::string	org_file("");
	std::string	export_file				= GenerateTempFilepath(progname, false, false);		// .k2hvec_pmtest_<PID>_<rand>.export
	std::string	org_filtered_file		= GenerateTempFilepath(progname, true, true);		// .k2hvec_pmtest_<PID>_<rand>_filtered.original
	std::string	export_filtered_file	= GenerateTempFilepath(progname, true, false);		// .k2hvec_pmtest_<PID>_<rand>_filtered.export
	if(opts.custom){
		org_file = GenerateTempFilepath(progname, false, true);								// .k2hvec_pmtest_<PID>_<rand>.original
	}else{
		// C source code file path (k2hvecbuiltinpm.cc)
		org_file = GetProjectionMatrixSourceFilePath(argv[0]);								// k2hvecbuiltinpm.cc
		if(org_file.empty()){
			ERR("Could not make Projection Matrix source file(k2hvecbuiltinpm.cc).");
			exit(EXIT_FAILURE);
		}

		// Check file exists
		std::ifstream	ifs(org_file.c_str());
		if(!ifs.is_open()){
			ERR("Not found Projection Matrix source file(%s).", org_file.c_str());
			exit(EXIT_FAILURE);
		}
		ifs.close();
	}

	//
	// [Custom Mode] Create/Load Custom Projection Matrix
	//
	if(opts.custom){
		//
		// Get builtin Projection Matrix information for setting custom
		//
		//	name		= progname
		//	version		= 0.1
		//	bit size	= same as builtin Projection Matrix(ex. 48)
		//	dimension	= same as builtin Projection Matrix(ex. 4096)
		//	file path	= /tmp/...
		//
		int	bitsize		= ProjectionMatrix::GetProjectionMatrixBitsize();
		int dimension	= ProjectionMatrix::GetProjectionMatrixDimension();

		//
		// Create Projection Matrix
		//
		if(!ProjectionMatrix::CreateProjectionMatrixFileAsCArray(org_file.c_str(), bitsize, dimension, progname.c_str(), "0.1", !opts.custom)){
			ERR("Failed creating custom projection matrix.");
			exit(EXIT_FAILURE);
		}
		PRN("[SUCCESS] Create custom projection matrix to %s (bit size=%d, dimension=%d).", org_file.c_str(), bitsize, dimension);

		//
		// Load created custom Projection Matrix
		//
		if(!ProjectionMatrix::LoadCustomProjectionMatrix(org_file.c_str())){
			ERR("Failed loading custom projection matrix.");
			exit(EXIT_FAILURE);
		}
		PRN("[SUCCESS] Load custom projection matrix from %s.", org_file.c_str());
	}

	//
	// Export Projection Matrix
	//
	if(!ProjectionMatrix::ExportProjectionMatrixFileAsCArray(export_file.c_str(), opts.builtin)){
		ERR("Failed exporting %s projection matrix.", opts.builtin ? "builtin" : "custom");
		exit(EXIT_FAILURE);
	}
	PRN("[SUCCESS] Export %s projection matrix to %s.", opts.builtin ? "builtin" : "custom", export_file.c_str());

	//
	// Make filtered files
	//
	// [NOTE]
	// Run : "cat <org file path> | grep -v '^[[:space:]]*\*/' | grep -v '^[[:space:]]*\*' | grep -v '^.*/\*' | grep -v '^[[:space:]]*//' > <filtered file path> 2>&1"
	//
	std::string	strFilterCmd		= " | grep -v '^[[:space:]]*\\*/' | grep -v '^[[:space:]]*\\*' | grep -v '^.*/\\*' | grep -v '^[[:space:]]*//' | grep -v '^[[:space:]]*#' > ";
	std::string	strOrgFilterCmd		= "cat " + org_file		+ strFilterCmd + org_filtered_file		+ " 2>&1";
	std::string	strExportFilterCmd	= "cat " + export_file	+ strFilterCmd + export_filtered_file	+ " 2>&1";

	if(0 != std::system(strOrgFilterCmd.c_str())){
		ERR("Failed filtering orignal projection matrix file %s to %s.", org_file.c_str(), org_filtered_file.c_str());
		exit(EXIT_FAILURE);
	}
	PRN("[SUCCESS] Filter orignal projection matrix file %s to %s.", org_file.c_str(), org_filtered_file.c_str());

	if(0 != std::system(strExportFilterCmd.c_str())){
		ERR("Failed filtering exported projection matrix file %s to %s.", export_file.c_str(), export_filtered_file.c_str());
		exit(EXIT_FAILURE);
	}
	PRN("[SUCCESS] Filter exported projection matrix file %s to %s.", export_file.c_str(), export_filtered_file.c_str());

	//
	// Veriy(diff) Projection Matrix
	//
	std::string	strVerifyCmd = "diff " + org_filtered_file + " " + export_filtered_file + " >/dev/null 2>&1";

	if(0 != std::system(strVerifyCmd.c_str())){
		ERR("Failed verify projection matrix file %s and %s.", org_filtered_file.c_str(), export_filtered_file.c_str());
		exit(EXIT_FAILURE);
	}
	PRN("[SUCCESS] Verify projection matrix file %s and %s.", org_filtered_file.c_str(), export_filtered_file.c_str());

	//
	// Cleanup for temporary files
	//
	if(opts.custom){
		if(0 != std::remove(org_file.c_str())){
			PRN("[WARNING] Could not remove temporary file %s.", org_file.c_str());
	    }
	}
	if(0 != std::remove(export_file.c_str())){
		PRN("[WARNING] Could not remove temporary file %s.", export_file.c_str());
    }
	if(0 != std::remove(org_filtered_file.c_str())){
		PRN("[WARNING] Could not remove temporary file %s.", org_filtered_file.c_str());
    }
	if(0 != std::remove(export_filtered_file.c_str())){
		PRN("[WARNING] Could not remove temporary file %s.", export_filtered_file.c_str());
    }

	// Print end message
	std::string	strDuration	= duration.GetDurationString();
	PRN("");
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
