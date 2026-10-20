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
#include <cinttypes>
#include <unistd.h>
#include <libgen.h>

#include "k2hvecmath.h"
#include "k2hvecbuiltinpm.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"
#include "k2hvec_tools_common.h"

//---------------------------------------------------------
// Utility Functions : Put and Load Embedding float array and other items
//---------------------------------------------------------
//
// Keywords in file
//
static constexpr const char*	KEY_EMBEDDING			= "EMBEDDING_DATA";
static constexpr const char*	KEY_EMBEDDING_CHECKSUM	= "EMBEDDING_CHECKSUM";
static constexpr const char*	KEY_BINARY_HASH			= "BINARY_HASH";
static constexpr const char*	KEY_SECOND_HASH			= "SECOND_HASH";

//
// Put Embedding float array and other items
//
static bool PutDatafile(const char* file, const std::vector<float>& embedding, const std::string& strChecksum, const std::string& strHexBinHash, const std::string& strHexSecHash)
{
	// check file exists
	if(VECEMPTYSTR(file)){
		ERR("file parameter is null or empty.");
		return false;
	}
	std::ifstream	ifs(file);
	if(ifs.is_open()){
		PRN("[WARNING] The file(%s) exists, so this file will be overwritten.", file);
		ifs.close();
	}

	// Make file contents
	std::stringstream	ss;
	std::string			strCurDisp	= GetCurrentTimespecToDateString();

	// Put header
	ss <<	"#----------------------------------------------------------\n"
			"# The Embedding data file used by K2HVEC.\n"
			"#\n"
			"# This file was generated using the tools provided with K2HVEC.\n"
			"#\n"
			"# CREATE: " << strCurDisp << "\n"
			"#----------------------------------------------------------\n"
			"\n";

	// Put projection matrix data
	std::string	strFloatArray = MakeStringFromFloatArray(embedding, "    ");
	ss << KEY_EMBEDDING				<< " = " << strFloatArray	<< "\n\n";
	ss << KEY_EMBEDDING_CHECKSUM	<< " = " << strChecksum		<< "\n\n";
	ss << KEY_BINARY_HASH			<< " = " << strHexBinHash	<< "\n\n";
	ss << KEY_SECOND_HASH			<< " = " << strHexSecHash	<< "\n\n";

	// Put footer
	ss <<	"#\n"
			"# EOF\n"
			"#\n";

	// Output to file
	std::ofstream	ofs(file, std::ios::out | std::ios::trunc);
	if(!ofs.is_open()){
		ERR("Could not open file(%s).", file);
		return false;
	}
	ofs << ss.str();
	ofs.close();

	return true;
}

//
// Parse Embedding uint8_t array from file
//
static bool ParseAndSetUint8Values(const std::string& str, std::vector<uint8_t>& uintarray, bool& found_end_block)
{
	found_end_block = false;

	std::stringstream	ss(str);
	char				tmpch;

	while(ss >> std::ws){								// trim left
		// check end of stream
		int	next_char = ss.peek();
		if(next_char == EOF){
			break;										// reach to end
		}

		// check block end charactor
		if('}' == next_char){
			ss.get();									// skip '}'
			found_end_block = true;

			// check charactors after '}' word
			while(ss >> tmpch){
				if(!std::isspace(static_cast<unsigned char>(tmpch)) && ';' != tmpch){
					ERR("Found charactor after } word, so it is wrong format.");
					return false;						// wrong format
				}
			}
			break;										// reach to end
		}

		// read one data as unsigned int
		unsigned int	val;
		if(ss >> std::hex >> val){						// read as hex format
			if(0xFF < val){
				ERR("Value exceeds uint8_t range(0x00 - 0xFF).");
				return false;
			}
			uintarray.push_back(static_cast<uint8_t>(val));
		}else{
			ERR("Value string is not uint8_t format.");
			return false;
		}

		// check ',' and skip it
		ss >> std::ws;
		if(',' == ss.peek()){
			ss.get();									// skip ','
		}
	}

	return true;
}

//
// Put Embedding float array and other items
//
static bool LoadDatafile(const char* file, std::vector<float>& embedding, float& checksum, std::vector<uint8_t>& binhash, uint64_t& sechash)
{
	if(VECEMPTYSTR(file)){
		ERR("file parameter is null or empty.");
		return false;
	}
	std::ifstream	ifs(file);
	if(!ifs.is_open()){
		ERR("Could not open file(%s).", file);
		return false;
	}

	// clear
	embedding.clear();
	binhash.clear();

	// Loading
	std::string	line;
	bool		in_embedding_block	= false;
	bool		in_binhash_block	= false;
	bool		set_checksum		= false;
	while(std::getline(ifs, line)){
		if(line.empty()){
			continue;
		}

		if(in_embedding_block){
			// in {...} block
			bool	found_end_block = false;
			if(!ProjectionMatrix::ParseAndSetFloatValues(line, embedding, found_end_block)){
				ERR("Something wrong during reading %s keyword.", KEY_EMBEDDING);
				return false;
			}
			if(found_end_block){
				in_embedding_block = false;				// end of data block
			}

		}else if(in_binhash_block){
			// in {...} block
			bool	found_end_block = false;
			if(!ParseAndSetUint8Values(line, binhash, found_end_block)){
				ERR("Something wrong during reading %s keyword.", KEY_BINARY_HASH);
				return false;
			}
			if(found_end_block){
				in_binhash_block = false;				// end of data block
			}

		}else{
			// out of {..} block
			std::string	value("");

			if(ProjectionMatrix::GetKeywordValue(line, KEY_EMBEDDING, false, value)){
				// Maybe value is "{..."
				if('{' != value.front()){
					ERR("%s keyword first line is wrong format.", KEY_EMBEDDING);
					return false;
				}
				in_embedding_block = true;

				// trim left
				value = ProjectionMatrix::TrimWithoutComment(value.substr(1));
				if(!value.empty()){
					// The first line has float datas.
					bool	found_end_block = false;
					if(!ProjectionMatrix::ParseAndSetFloatValues(value, embedding, found_end_block)){
						ERR("Something wrong during reading %s keyword.", KEY_EMBEDDING);
						return false;
					}
					if(found_end_block){
						in_embedding_block = false;		// end of data block
					}
				}

			}else if(ProjectionMatrix::GetKeywordValue(line, KEY_EMBEDDING_CHECKSUM, false, value)){
				// value is string in one line
				if(value.empty()){
					ERR("%s keyword value is empty.", KEY_EMBEDDING_CHECKSUM);
					return false;
				}

				std::stringstream	ss(value);
				if(ss >> checksum){								// read string as float
					if('f' == ss.peek() || 'F' == ss.peek()){
						ss.get();								// skip 'f' word
					}
				}else{
					ERR("%s keyword value is something wrong.", KEY_EMBEDDING_CHECKSUM);
					return false;
				}
				set_checksum = true;

			}else if(ProjectionMatrix::GetKeywordValue(line, KEY_BINARY_HASH, false, value)){
				// Maybe value is "{..."
				if('{' != value.front()){
					ERR("%s keyword first line is wrong format.", KEY_BINARY_HASH);
					return false;
				}
				in_binhash_block = true;

				// trim left
				value = ProjectionMatrix::TrimWithoutComment(value.substr(1));
				if(!value.empty()){
					// The first line has uint8_t datas.
					bool	found_end_block = false;
					if(!ParseAndSetUint8Values(value, binhash, found_end_block)){
						ERR("Something wrong during reading %s keyword.", KEY_BINARY_HASH);
						return false;
					}
					if(found_end_block){
						in_binhash_block = false;		// end of data block
					}
				}

			}else if(ProjectionMatrix::GetKeywordValue(line, KEY_SECOND_HASH, false, value)){
				if(value.empty()){
					ERR("%s keyword value is empty.", KEY_SECOND_HASH);
					return false;
				}
				// value is uint64_t
				std::stringstream ss;
				ss << std::hex << value;
				ss >> sechash;

			}else{
				continue;								// not found keyword, so skip
			}
		}
	}
	ifs.close();

	if(embedding.empty() || !set_checksum || binhash.empty()){
		ERR("The load file is missing one of the keywords.");
		return false;
	}

	return true;
}

//---------------------------------------------------------
// Option parser etc
//---------------------------------------------------------
//
// K2HVecTestBhOpt class
//
class K2HVecTestBhOpt
{
	private:
		static constexpr const char*	CMD_LONG_CREATE			= "create";
		static constexpr const char*	CMD_LONG_VERIFY			= "verify";

		static constexpr const char*	CMD_SHORT_CREATE		= "c";
		static constexpr const char*	CMD_SHORT_VERIFY		= "v";

		static constexpr const char*	OPT_LONG_NAME_HELP		= "--help";
		static constexpr const char*	OPT_LONG_NAME_DEBUG		= "--debug";
		static constexpr const char*	OPT_LONG_NAME_PMFILE	= "--projection_matrix";
		static constexpr const char*	OPT_LONG_NAME_OUTPUT	= "--output";
		static constexpr const char*	OPT_LONG_NAME_INPUT		= "--input";

		static constexpr const char*	OPT_SHORT_NAME_HELP		= "-h";
		static constexpr const char*	OPT_SHORT_NAME_DEBUG	= "-d";
		static constexpr const char*	OPT_SHORT_NAME_PMFILE	= "-p";
		static constexpr const char*	OPT_SHORT_NAME_OUTPUT	= "-o";
		static constexpr const char*	OPT_SHORT_NAME_INPUT	= "-i";

		bool Parse(int argc, const char** argv);

	public:
		bool		error;
		bool		help;
		bool		debug;
		bool		create;
		bool		verify;
		std::string	strPmFile;
		std::string	strEmFile;

		K2HVecTestBhOpt(int argc, const char** argv);
		~K2HVecTestBhOpt() {}

		K2HVecTestBhOpt() = delete;
		K2HVecTestBhOpt(const K2HVecTestBhOpt&) = delete;
		K2HVecTestBhOpt& operator=(const K2HVecTestBhOpt&) = delete;
		K2HVecTestBhOpt& operator=(K2HVecTestBhOpt&&) = delete;
};

K2HVecTestBhOpt::K2HVecTestBhOpt(int argc, const char** argv): error(false), help(false), debug(false), create(false), verify(false), strPmFile(""), strEmFile("")
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

	std::string	tmpOutputFile;
	std::string	tmpInputFile;

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

		}else if(CMD_LONG_CREATE == oneArg || CMD_SHORT_CREATE == oneArg){
			// create command
			if(create){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			create = true;

		}else if(CMD_LONG_VERIFY == oneArg || CMD_SHORT_VERIFY == oneArg){
			// create command
			if(verify){
				ERR("Already specified %s option.", oneArg.c_str());
				return false;
			}
			verify = true;

		}else if(OPT_LONG_NAME_PMFILE == oneArg || OPT_SHORT_NAME_PMFILE == oneArg){
			// --projection_matrix option
			if(!strPmFile.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), strPmFile.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			strPmFile = strParam;

		}else if(OPT_LONG_NAME_OUTPUT == oneArg || OPT_SHORT_NAME_OUTPUT == oneArg){
			// --output option
			if(!tmpOutputFile.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), tmpOutputFile.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			tmpOutputFile = strParam;

		}else if(OPT_LONG_NAME_INPUT == oneArg || OPT_SHORT_NAME_INPUT == oneArg){
			// --input option
			if(!strEmFile.empty()){
				ERR("Already specified %s option with value(%s).", oneArg.c_str(), strEmFile.c_str());
				return false;
			}
			std::string	strParam = VECEMPTYSTR(argv[++pos]) ? "" : argv[pos];
			if(strParam.empty()){
				ERR("%s option parameter is empty.", oneArg.c_str());
				return false;
			}
			tmpInputFile = strParam;

		}else{
			ERR("Unkown command or option(%s).", oneArg.c_str());
			return false;
		}
	}

	// check
	if(!help && !create && !verify){
		ERR("You must specify either the %s(%s) or %s(%s) command.", CMD_LONG_CREATE, CMD_SHORT_CREATE, CMD_LONG_VERIFY, CMD_SHORT_VERIFY);
		return false;

	}else if(!help && create && verify){
		ERR("The %s(%s) and %s(%s) commands cannot be specified together.", CMD_LONG_CREATE, CMD_SHORT_CREATE, CMD_LONG_VERIFY, CMD_SHORT_VERIFY);
		return false;

	}else if(!help){
		if(strPmFile.empty()){
			ERR("You must specify %s(%s) option.", OPT_LONG_NAME_PMFILE, OPT_SHORT_NAME_PMFILE);
			return false;
		}

		if(create){
			if(tmpOutputFile.empty()){
				ERR("You must specify %s(%s) option.", OPT_LONG_NAME_OUTPUT, OPT_SHORT_NAME_OUTPUT);
				return false;
			}
			if(!tmpInputFile.empty()){
				ERR("Could not specify %s(%s) option for %s(%s) command.", OPT_LONG_NAME_INPUT, OPT_SHORT_NAME_INPUT, CMD_LONG_CREATE, CMD_SHORT_CREATE);
				return false;
			}
			strEmFile = tmpOutputFile;

		}else if(verify){
			if(tmpInputFile.empty()){
				ERR("You must specify %s(%s) option.", OPT_LONG_NAME_INPUT, OPT_SHORT_NAME_INPUT);
				return false;
			}
			if(!tmpOutputFile.empty()){
				ERR("Could not specify %s(%s) option for %s(%s) command.", OPT_LONG_NAME_OUTPUT, OPT_SHORT_NAME_OUTPUT, CMD_LONG_CREATE, CMD_SHORT_CREATE);
				return false;
			}
			strEmFile = tmpInputFile;
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
	PRN("Test for Binary Hash Calculation");
	PRN("");
	PRN("Usage: %s create(c) --projection_matrix(-p) <file path> --output(-o) <file path> --debug(-d)", progname.c_str());
	PRN("Usage: %s verify(v) --projection_matrix(-p) <file path> --input(-i) <file path> --debug(-d)", progname.c_str());
	PRN("Usage: %s --help(-h)", progname.c_str());
	PRN("");
	PRN("Command:");
	PRN("   create(c)                           create a verification file for dummy embedding float array and other items");
	PRN("   verify(v)                           calcurate binary hash and verify it");
	PRN("");
	PRN("Options:");
	PRN("  --help(-h)                           display this usage.");
	PRN("  --debug(-d)                          print debug messages");
	PRN("  --projection_matrix(-p) <file path>  specify custom projection matrix file");
	PRN("  --output(-o) <file path>             specify result output file(the result are embedding array, checksum value, and hash values)");
	PRN("  --input(-i) <file path>              specify input file expected values(it has embedding array, checksum value, and hash values)");
	PRN("");
	PRN("About:");
	PRN("This tool performs three operations:");
	PRN("(1) Creation of a verification file");
	PRN("    This tool creates a dummy embedding float array.");
	PRN("    And using a specified custom projection matrix file, this calculates");
	PRN("    a binary hash from the dummy embedding and computes its checksum.");
	PRN("    Then this outputs the dummy embedding float array and the checksum");
	PRN("    as an input file for verification.");
	PRN("    Additionally, loading the specified Custom Projection Matrix file,");
	PRN("    this generates a dummy embedding float array and calculates its");
	PRN("    checksum, finally computing a binary hash.");
	PRN("    Then this outputs the dummy embedding float array and the binary");
	PRN("    hash as verification input files.");
	PRN("");
	PRN("(2) Binary hash calculation verification");
	PRN("    Using the file created in step(1) as input it, this calculates the");
	PRN("    binary hash.");
	PRN("    This verifies the binary hash calculation performed by k2hvec.");
	PRN("");
	PRN("(3) Verification of Second Hash Calculation");
	PRN("    Calculate the second hash using the file created in step(1) as input.");
	PRN("    This verifies the second hash calculation performed by k2hvec.");
	PRN("");
	PRN("A custom projection matrix can be created using k2hvecpmtool.");
	PRN("    $ k2hvecpmtool --output <ex. %s_pm.data>", progname.c_str());
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

	// Start measurement
	LapTime	duration(true);

	// Load Custom Projection Matrix
	if(!ProjectionMatrix::LoadCustomProjectionMatrix(opts.strPmFile.c_str())){
		ERR("Failed to load custom projection matrix(%s)", opts.strPmFile.c_str());
		exit(EXIT_FAILURE);
	}

	// Get information of builtin Projection Matrix
	std::string	strPmName		= ProjectionMatrix::GetProjectionMatrixName();
	std::string	strPmVersion	= ProjectionMatrix::GetProjectionMatrixVersion();
	int			bitsize			= ProjectionMatrix::GetProjectionMatrixBitsize();
	int			dimension		= ProjectionMatrix::GetProjectionMatrixDimension();

	// Run by mode
	if(opts.create){
		// Create Dummy Embedding float array
		std::vector<float>		embedding		= GenerateEmbedding(dimension);
		std::string				strEmbedding	= MakeStringFromFloatArray(embedding, "    ");
		std::string				strChecksum		= GetChecksumString(embedding);

		// Calcurate Binary Hash
		std::vector<uint8_t>	binhash			= ProjectionMatrix::GetBinaryHash(embedding);
		std::string				strHexBinHash	= MakeStringFromUint8Array(binhash, false, "    ");
		std::string				strBitBinHash	= MakeStringFromUint8Array(binhash, true, "    ");
		std::string				strFHexBinHash	= MakeFlatStringFromUint8Array(binhash, false);
		std::string				strFBitBinHash	= MakeFlatStringFromUint8Array(binhash, true);

		// Calcurate Second Hash
		uint64_t				sechash			= CalSecondHash(embedding);
		std::string				strHexSecHash	= MakeFlatStringFromUint64(sechash, false);
		std::string				strBitSecHash	= MakeFlatStringFromUint64(sechash, true);

		// Put file
		if(!PutDatafile(opts.strEmFile.c_str(), embedding, strChecksum, strHexBinHash, strHexSecHash)){
			ERR("Failed to put file(%s)", opts.strEmFile.c_str());
			exit(EXIT_FAILURE);
		}

		// Print information
		std::string				strDuration		= duration.GetDurationString();

		PRN("===========================================================");
		PRN("Infomration : Create a Verification file");
		PRN("===========================================================");
		PRN("Output file                  : %s", opts.strEmFile.c_str());
		PRN("===========================================================");
		PRN("Projection Matrix file       : %s", opts.strPmFile.c_str());
		PRN("                  Name       : %s", strPmName.c_str());
		PRN("                  Version    : %s", strPmVersion.c_str());
		PRN("                  bit size   : %d", bitsize);
		PRN("                  dimension  : %d", dimension);
		PRN("===========================================================");
		PRN("Embedding float array size   : %d", dimension);
		PRN("                check sum    : %s", strChecksum.c_str());

		if(opts.debug){
			PRN("-----------------------------------------------------------");
			PRN("Embedding float array        : %s", strEmbedding.c_str());
		}

		PRN("===========================================================");
		PRN("Calcurate Binary Hash size   : %lu (%lu bit) = Projection Matrix bit size", binhash.size(), binhash.size() * 8);
		PRN("                      value  : h'%s", strFHexBinHash.c_str());
		PRN("                             : b'%s", strFBitBinHash.c_str());

		if(opts.debug){
			PRN("-----------------------------------------------------------");
			PRN("HEX                          : %s", strHexBinHash.c_str());
			PRN("");
			PRN("BIT                          : %s", strBitBinHash.c_str());
		}

		PRN("===========================================================");
		PRN("Calcurate Second Hash value  : h'%s", strHexSecHash.c_str());
		PRN("                             : b'%s", strBitSecHash.c_str());

		PRN("===========================================================");
		PRN("[SUCCESS] Calculation time : %s", strDuration.c_str());
		PRN("===========================================================");

	}else if(opts.verify){
		// Load Dummy Embedding float array
		std::vector<float>		embedding;
		float					verify_checksum	= 0.0f;
		std::vector<uint8_t>	verify_binhash;
		uint64_t				verify_sechash = 0;

		if(!LoadDatafile(opts.strEmFile.c_str(), embedding, verify_checksum, verify_binhash, verify_sechash)){
			ERR("Failed to load file(%s)", opts.strEmFile.c_str());
			exit(EXIT_FAILURE);
		}

		// Get loaded embedding data checksum
		float					checksum		= CalChecksum(embedding);
		std::string				strEmbedding	= MakeStringFromFloatArray(embedding, "    ");
		std::string				strChecksum		= GetChecksumString(embedding);

		// Calculate and Get binary hash
		std::vector<uint8_t>	binhash			= ProjectionMatrix::GetBinaryHash(embedding);
		std::string				strHexBinHash	= MakeStringFromUint8Array(binhash, false, "    ");
		std::string				strBitBinHash	= MakeStringFromUint8Array(binhash, true, "    ");
		std::string				strFHexBinHash	= MakeFlatStringFromUint8Array(binhash, false);
		std::string				strFBitBinHash	= MakeFlatStringFromUint8Array(binhash, true);

		// Calcurate Second Hash
		uint64_t				sechash			= CalSecondHash(embedding);
		std::string				strHexSecHash	= MakeFlatStringFromUint64(sechash, false);
		std::string				strBitSecHash	= MakeFlatStringFromUint64(sechash, true);

		// Verify
		bool		result = true;
		std::string	strResultChecksum;
		std::string	strResultBinhash;
		std::string	strResultSechash;
		if(checksum == verify_checksum){
			strResultChecksum	= "OK";
		}else{
			strResultChecksum	= "NG";
			result				= false;
		}
		if(binhash.size() == verify_binhash.size()){
			for(size_t pos = 0; pos < binhash.size(); ++pos){
				if(binhash[pos] != verify_binhash[pos]){
					strResultBinhash	= "NG";
					result				= false;
					break;
				}
			}
			if(result){
				strResultBinhash	= "OK";
			}
		}else{
			strResultBinhash	= "NG";
			result				= false;
		}
		if(sechash == verify_sechash){
			strResultSechash	= "OK";
		}else{
			strResultSechash	= "NG";
			result				= false;
		}

		// Print information
		std::string	strDuration	= duration.GetDurationString();

		PRN("===========================================================");
		PRN("Infomration : Calcurate Binary Hash and Verify");
		PRN("===========================================================");
		PRN("Input Expected result file   : %s", opts.strEmFile.c_str());
		PRN("===========================================================");
		PRN("Projection Matrix file       : %s", opts.strPmFile.c_str());
		PRN("                  Name       : %s", strPmName.c_str());
		PRN("                  Version    : %s", strPmVersion.c_str());
		PRN("                  bit size   : %d", bitsize);
		PRN("                  dimension  : %d", dimension);
		PRN("===========================================================");
		PRN("Embedding float array size   : %d", dimension);
		PRN("                check sum    : %s", strChecksum.c_str());

		if(opts.debug){
			PRN("-----------------------------------------------------------");
			PRN("Embedding float array        : %s", strEmbedding.c_str());
		}

		PRN("===========================================================");
		PRN("Calcurate Binary Hash size   : %lu (%lu bit)", binhash.size(), binhash.size() * 8);
		PRN("                      value  : h'%s", strFHexBinHash.c_str());
		PRN("                             : b'%s", strFBitBinHash.c_str());

		if(opts.debug){
			PRN("-----------------------------------------------------------");
			PRN("HEX                          : %s", strHexBinHash.c_str());
			PRN("");
			PRN("BIT                          : %s", strBitBinHash.c_str());
		}

		PRN("===========================================================");
		PRN("Calcurate Second Hash value  : h'%s", strHexSecHash.c_str());
		PRN("                             : b'%s", strBitSecHash.c_str());

		PRN("===========================================================");
		PRN("RESULT                       : %s", result ? "OK" : "NG");
		PRN("       Embedding checksum    : %s (%.10gf vs %.10gf)", strResultChecksum.c_str(), checksum, verify_checksum);
		PRN("       Binary Hash           : %s (length: %lu vs %lu)", strResultBinhash.c_str(), binhash.size(), verify_binhash.size());
		PRN("       Second Hash           : %s (0x%" PRIx64 " vs 0x%" PRIx64 ")", strResultSechash.c_str(), sechash, verify_sechash);
		PRN("===========================================================");
		PRN("[%s] Calculation time : %s", (result ? "SUCCESS" : "FAILURE"), strDuration.c_str());
		PRN("===========================================================");
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
