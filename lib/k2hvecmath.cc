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

#include <vector>
#include <random>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <numeric>

#include "k2hvecmath.h"
#include "k2hvecbuiltinpm.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"

//---------------------------------------------------------
// About K2HVEC Vector Search
//---------------------------------------------------------
// K2HVEC utilizes a two-stage approach to search vector data stored within the
// Key-Value Store (KVS).
//
// In the first stage, it applies Locality Sensitive Hashing(LSH), an Approximate
// Nearest Neighbor (ANN) search technique, to filter the candidates. Specifically,
// multi-dimensional embedding vectors(such as embeddings or coordinates generated
// by OpenAI) are converted into binary hash values of 64 bits or more, which are
// then used to narrow down the target scope.
//
// In the second stage, it calculates and compares the cosine similarity between
// the query vector and each of the filtered candidate vectors to pinpoint the
// final targets.
//
// A Projection Matrix is used to compute these binary hash values.
// To map the high-dimensional embeddings(ex. 1537 dimensions) into a bit array,
// this projection matrix of "float[dimension]" is required for each respective
// bit position.
//
// About Projection Matrix:
//   To calculate the binary hash values, a Projection Matrix is required.
//   To transform the embedding dimensions(ex. 1537 dimensions) into a bit array,
//   this projection matrix of "float[dimension]" must be provided for each bit
//   position.
//
//   For instance, if the dimension of the embedding vector is defined as dimension
//   and the number of bits for the binary hash value is bitsize, a Projection
//   Matrix structured as a two-dimensional array of "float[bitsize][dimension]" is
//   needed.
//   While it conceptually requires a 2-D array of "float[bitsize][dimension]", a
//   flattened one-dimensional array of "float[bitsize * dimension]" is also
//   acceptable.
//
//   So that, this utility utilizes a one-dimensional float array.
//
// About compile options:
//   Please specify the following options when compiling:
//       -O3 -fno-math-errno -freciprocal-math -fno-trapping-math
//
//   If these options are not specified, the desired performance will not be achieved.
//
//   Notes regarding each option are summarized below. 
//     -ffast-math:
//         This may affect other linked files (object files).
//         It impacts calculations involving types like float and double(including
//         within internal libraries), so side effects are possible. 
//
//         -> Time-related calculations, among others, could be affected.
//
//     -fno-math-errno -freciprocal-math -fno-trapping-math:
//         Since -ffast-math is a bundled option, we replace it with these individual
//         component options. 
//
//         -> This should be fine. 
//
//     -march=native:
//         You must absolutely NOT specify this, as the software will be packaged and
//         run on CPUs different from the build environment.
//         It is to boost speed, but it must not be used on this product.
//         It may be slightly slower without it, but the performance remains
//         sufficiently fast.
//

//---------------------------------------------------------
// Utility for Printable string
//---------------------------------------------------------
//
// Make display string from float array
//
std::string MakeStringFromFloatArray(const std::vector<float>& farray, const std::string& indent)
{
	std::stringstream	ss;

	ss << "{\n";
	for(size_t pos = 0; pos < farray.size(); ++pos){
		if(0 == pos % 8){
			if(0 != pos){
				ss << std::endl;
			}
			ss << indent;
		}

		//
		// Precision setting
		//
		// Setting to ensure 100% prevention of information loss.
		// Automatically sets the number of digits required to fully restore the bit information
		// held by the float type(typically 9 digits).
		//
		std::stringstream	ssFloat;
		ssFloat << std::setprecision(std::numeric_limits<float>::max_digits10);
		if(0 <= farray[pos]){
			ssFloat << " ";
		}
		ssFloat << farray[pos] << "f";

		std::string	strFloat = ssFloat.str();
		if((pos + 1) < farray.size()){
			strFloat += ",";
			if(0 != (pos + 1) % 8){
				// fill to 18-22 charactor with space
				size_t	length = 18;
				if(0 == (pos + 1) % 4){
					length = 22;
				}
				while(strFloat.size() < length){
					strFloat += " ";
				}
			}
		}
		ss << strFloat;
	}
	ss << "\n}";

	return ss.str();
}

//
// Make display string from uint8_t array
//
std::string MakeStringFromUint8Array(const std::vector<uint8_t>& uintarray, bool printbit, const std::string& indent)
{
	std::stringstream	ss;

	ss << "{\n";
	for(size_t pos = 0; pos < uintarray.size(); ++pos) {
		if(0 == pos % 8){
			if(0 != pos){
				ss << std::endl;
			}
			ss << indent;
		}

		if(printbit){
			for(size_t bitpos = 0; bitpos < (sizeof(uint8_t) * 8); ++bitpos){
				ss << ((uintarray[pos] & (1ULL << ((sizeof(uint8_t) * 8) - bitpos - 1))) ? "1" : "0");
			}
		}else{
			ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(uintarray[pos]);
		}

		if(0 != (pos + 1) % 8){
			if(0 != (pos % 8) && 0 == (pos + 1) % 4){
				ss << "  ";
			}else{
				ss << " ";
			}
		}
	}
	ss << std::endl;
	ss << "}";

	return ss.str();
}

//
// Make flat string from uint8_t array
//
std::string MakeFlatStringFromUint8Array(const std::vector<uint8_t>& uintarray, bool printbit, int bitsize)
{
	std::stringstream	ss;
	int					putbitcnt = 0;
	for(size_t pos = 0; pos < uintarray.size(); ++pos) {
		if(printbit){
			for(size_t bitpos = 0; bitpos < (sizeof(uint8_t) * 8); ++bitpos){
				ss << ((uintarray[pos] & (1ULL << ((sizeof(uint8_t) * 8) - bitpos - 1))) ? "1" : "0");
				if(0 < bitsize && bitsize <= ++putbitcnt){
					break;
				}
			}
		}else{
			ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(uintarray[pos]);
			if(0 < bitsize){
				putbitcnt += sizeof(uint8_t) * 8;
			}
		}
		if(0 < bitsize && bitsize <= putbitcnt){
			break;
		}
	}
	return ss.str();
}

//
// Make flat string from uint64_t
//
std::string MakeFlatStringFromUint64(uint64_t value, bool printbit)
{
	std::stringstream	ss;

	if(printbit){
		for(size_t bitpos = 0; bitpos < (sizeof(uint64_t) * 8); ++bitpos){
			ss << (((value >> (((sizeof(uint64_t) * 8) - 1) - bitpos)) & 1ULL) ? "1" : "0");
		}
	}else{
		ss << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << value;
	}
	return ss.str();
}

//
// Make flat string from float
//
std::string MakeFlatStringFromFloat(float value)
{
	// [NOTE]
	// Using max_digits10 causes the value to be rounded, resulting in a larger discrepancy, so digits10 is used instead.
	//
	std::stringstream	ss;
	ss << std::setprecision(std::numeric_limits<float>::digits10) << value;

	std::string	str = ss.str();
	if(std::string::npos == str.find('.')){
		str += ".0f";
	}else{
		str += "f";
	}
	return str;
}

//---------------------------------------------------------
// Utility for Checksum calculation
//---------------------------------------------------------
//
// Calculate check-sum for float array
//
std::string GetChecksumString(const std::vector<float>& farray)
{
	return MakeFlatStringFromFloat(CalChecksum(farray));
}

// [NOTE]
// // This checksum is calculated as the squared L2 norm of the vector(the sum of
// the squares of each element).
// Therefore, by definition, the sum of squares for an L2-normalized array will
// always be about 1.0f.
// If you need a checksum to distinguish between normalized arrays, calculate a
// hash value instead.
//
float CalChecksum(const std::vector<float>& farray)
{
	return std::inner_product(farray.begin(), farray.end(), farray.begin(), 0.0f);
}

//---------------------------------------------------------
// Utility for Mathematics and calculation
//---------------------------------------------------------
//
// Calculation of Cosine Similarity
//
// [NOTE]
// There are two functions, CosineSimilarity, CosineSimilarityStandard and
// CosineSimilaritySimple.
// Function CosineSimilarityStandard is the full version.
// Function CosineSimilaritySimple assumes the input float array is normalized
// to a length of 1.0. This function skips the square root calculation and
// casting to double, performing only the multiply-accumulate operation.
// (This is suitable for use with embeddings such as those from OpenAI.)
// However, for a simple, one-off calculation, there is no significant difference
// in execution time.
//
float CosineSimilarity(const std::vector<float>& data1, const std::vector<float>& data2, bool simple)
{
	if(simple){
		return CosineSimilaritySimple(data1, data2);
	}else{
		return CosineSimilarityStandard(data1, data2);
	}
}

float CosineSimilarityStandard(const std::vector<float>& data1, const std::vector<float>& data2)
{
	if(data1.size() != data2.size()){
		WAN_VECPRN("The lengths of the vector<float> is differ: %lu vs %lu", data1.size(), data2.size());
		return 0.0f;
	}
	if(data1.empty()){
		WAN_VECPRN("The lengths of the vector<float> is 0");
		return 0.0f;
	}

	// [NOTE]
	// If the vector data is not normalized to a length of 1.0, internal
	// calculations are performed using `double` precision to minimize
	// loss of significance and rounding errors associated with floating-point
	// numbers.
	//
	double	dot_product	= 0.0;
	double	norm_data1	= 0.0;
	double	norm_data2	= 0.0;

	// Simultaneously calculate the inner product and the respective norms(squared lengths).
	for(size_t pos = 0; pos < data1.size(); ++pos){
		dot_product	+= static_cast<double>(data1[pos]) * static_cast<double>(data2[pos]);
		norm_data1	+= static_cast<double>(data1[pos]) * static_cast<double>(data1[pos]);
		norm_data2	+= static_cast<double>(data2[pos]) * static_cast<double>(data2[pos]);
	}

	// Prevention of division by zero
	if(0.0 == norm_data1 || 0.0 == norm_data2){
		MSG_VECPRN("Case division by zero.");
		return 0.0f;
	}

	// Calculate the denominator and numerator separately
	double	denominator	= std::sqrt(norm_data1) * std::sqrt(norm_data2);
	double	similarity	= dot_product / denominator;

	// [NOTE]
	// Prevent values from exceeding 1.0 or falling below -1.0 due to floating errors.
	//
	if(1.0 < similarity){
		similarity = 1.0;
	}else if(similarity < -1.0){
		similarity = -1.0;
	}

	return static_cast<float>(similarity);
}

float CosineSimilaritySimple(const std::vector<float>& data1, const std::vector<float>& data2)
{
	if(data1.size() != data2.size()){
		WAN_VECPRN("The lengths of the vector<float> is differ: %lu vs %lu", data1.size(), data2.size());
		return 0.0f;
	}
	if(data1.empty()){
		WAN_VECPRN("The lengths of the vector<float> is 0");
		return 0.0f;
	}

	// [NOTE]
	// Assuming both vector data are normalized to a length of 1.0, simplify
	// the calculation to just a sum of products.
	//
	float			dot_product	= 0.0f;
	const size_t	length		= data1.size();

	for(std::size_t pos = 0; pos < length; ++pos) {
		dot_product += data1[pos] * data2[pos];
	}
	return dot_product;
}

//
// Calculates a binary hash value from an embedding vector using a projection matrix.
//
// embedding:
//		The input embedding vector. The number of elements must be less than or equal
//      to 'dimension'. Any missing elements up to 'dimension' are treated as 0.0f
//      (resulting in a calculation of 0).
//
// projection_matrix:
//		The projection matrix used to compute the binary hash.
//      It must be a 1-D array representing a "float[bitsize][dimension]" structure,
//      with a total size of "float[bitsize * dimension]".
//		(Note: The element count of 'embedding' must be less than 'dimension'.)
//
// bitsize:
//		The number of bits for the generated binary hash.
//		Recommended to specify it as a multiple of uint8_t(8 bits).
//
// dimension:
//		The base dimension of the embedding vector.
//		Cases where the embedding dimension is smaller than this value are permitted.
//
// Result:
//		A byte array(uint8_t[]) containing the created binary hash.
//		If 'bitsize' is not a multiple of 8, the padding will be left-aligned(MSB-first).
//		Returns an empty(zero-size) result in case of an error.
//
std::vector<uint8_t> CalBinaryHash(const std::vector<float>& embedding, const std::vector<float>& projection_matrix, int bitsize, int dimension)
{
	int	EmbeddingSize	= embedding.size();
	int	ProMatrixSize	= projection_matrix.size();
	if(EmbeddingSize <= 0 || bitsize <= 0 || (1 << 16) < bitsize || dimension <= 0 || dimension < EmbeddingSize || ProMatrixSize < (bitsize * dimension)){
		ERR_VECPRN("Parameters are wrong : embedding(size=%lu), bitsize(%d), dimension(%d), projection_matrix(size=%u)", embedding.size(), bitsize, dimension, ProMatrixSize);
		return {};
	}

	LapTime	duration(IS_VECDBG_MESSAGE());

	//
	// Dot product calculation
	//
	// [NOTE]
	// Calculates the dot product(multiply-accumulate) of the embedding array
	// and the projection_matrix array(bit sequence), then sets the result at
	// the index corresponding to the BitResult bit position.
	// This operation populates the float data for the requested bit position.
	//
	// Optimizations:
	//   If the number of elements in the embedding array is less than or equal
	//   to 'dimension', calculations beyond the element count are skipped
	//   (since multiplying by 0.0f makes addition unnecessary).
	//
	const float*	pEmbedding	= embedding.data();
	const float*	pProMatrix	= projection_matrix.data();
	float			BitResult[bitsize];						// Result values for earch bit number

	for(int bitno = 0; bitno < bitsize; ++bitno){
		int				OffsetProMatrix	= bitno * dimension;
		const float*	pRowProMatrix	= &pProMatrix[OffsetProMatrix];
		float			sum				= 0.0f;

		for(int data_pos = 0; data_pos < EmbeddingSize; ++data_pos){
			sum += pEmbedding[data_pos] * pRowProMatrix[data_pos];
		}
		BitResult[bitno] = sum;
	}

	//
	// Bit assembly and setting into a byte array(determination processing)
	//
	// [NOTE]
	// Iterates through the float array to evaluate the sign(positive/negative)
	// of each element, converting them into 1 or 0 bits to pack into a byte
	// array.
	// Each element of the byte array is packed in MSB-first order(left-aligned
	// / big-endian bit order) based on the calculation results.
	//
	// PositiveBits[]
	//   An array where the bit is set to 1 at the position corresponding to
	//   the bit index (thereby eliminating the need for shift operations).
	// BinHash
	//   If the total 'bitsize' is not aligned to a uint8_t boundary, the
	//   remaining bits in the last element of the array will be left-aligned
	//   (MSB-first).
	//
	const uint8_t			PositiveBits[sizeof(uint8_t) * 8]	= {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
	int						BinHashSize							= (bitsize / (sizeof(uint8_t) * 8)) + (0 == (bitsize % (sizeof(uint8_t) * 8)) ? 0 : 1);
	std::vector<uint8_t>	BinHash(BinHashSize);

	for(int setpos = 0; setpos < BinHashSize; ++setpos){
		uint8_t	CurHashVal	= 0;
		for(size_t bitpos_base	= setpos * (sizeof(uint8_t) * 8), bitpos_in_part = 0; bitpos_in_part < (sizeof(uint8_t) * 8); ++bitpos_in_part){
			if((bitpos_base + bitpos_in_part) < static_cast<size_t>(bitsize)){
				if(0.0f <= BitResult[bitpos_base + bitpos_in_part]){
					CurHashVal |= PositiveBits[bitpos_in_part];
				}
			}
		}
		BinHash[setpos] = CurHashVal;
	}

	// Print messages
	if(IS_VECDBG_MESSAGE()){
		std::string	strDuration		= duration.GetDurationString();
		MSG_VECPRN("Make Binary Hash[%d] - Calculation time : %s", BinHashSize, strDuration.c_str());

		std::string	strHexBinHash	= MakeStringFromUint8Array(BinHash, true, "    ");
		std::string	strBitBinHash	= MakeStringFromUint8Array(BinHash, false, "    ");

		// Print data
		MSG_VECPRN("Binary Hash uint8_t[%d] = Hex %s", BinHashSize, strHexBinHash.c_str());
		MSG_VECPRN("");
		MSG_VECPRN("Binary Hash uint8_t[%d] = Bit %s", BinHashSize, strBitBinHash.c_str());
	}

	return BinHash;
}

//
// Calculates a FNV-1a hash value from an embedding vector for seoundary hash key.
//
// This function calculates the FNV-1a hash. FNV-1a is the default hash function
// used in K2Hash library. (http://www.isthe.com/chongo/tech/comp/fnv/index.html)
//
// This hash value is used in conjunction with a binary hash to serve as a unique
// key for storing vector data in K2Hash. Since a binary hash alone carries a risk
// of collisions, appending this FNV-1a hash value minimizes the probability of
// collisions to virtually zero.
//
uint64_t CalSecondHash(const std::vector<float>& embedding)
{
	static const uint64_t	prime	= 1099511628211ULL;
	uint64_t				hash	= 14695981039346656037ULL;

	const uint8_t*			pbyte	= reinterpret_cast<const uint8_t*>(embedding.data());
	for(size_t length = embedding.size() * sizeof(float); length; --length){
		hash ^= static_cast<uint64_t>(*pbyte++);
		hash *= prime;
	}

	// Use little-endian
	//
	// The BinaryHashe is typically treated as big-endian, as they are intended to
	// be used starting from the first byte at the bit level. However, since this
	// value is not split, it is treated as little-endian. (Little-endian is the
	// standard for most modern CPUs, making it more efficient.)
	//
	if(!IsHostLittleEndian()){
		hash = htole64(hash);
	}

	return hash;
}

//
// Calculates a FNV-1a hash value from string for summary K2Hash value
//
uint64_t CalFnvA1Hash(const char* str)
{
	if(!str){
		return 0;
	}

	static const uint64_t	prime	= 1099511628211ULL;
	uint64_t				hash	= 14695981039346656037ULL;

	const uint8_t*			pbyte	= reinterpret_cast<const uint8_t*>(str);
	while('\0' != *pbyte){
		hash ^= static_cast<uint64_t>(*pbyte++);
		hash *= prime;
	}

	// Use little-endian
	//
	if(!IsHostLittleEndian()){
		hash = htole64(hash);
	}
	return hash;
}

//---------------------------------------------------------
// Utility for test datas
//---------------------------------------------------------
//
// Generate Embedding data as sample(test)
//
// Generates multidimensional embedding data for testing and development.
// The number of dimensions can be specified as a parameter.
//
// [Creation steps]
//	1) Initialize an array with random values for the specified dimensions.
//	2) Calculate the L2 norm(length) of the vector.
//	3) Normalize the vector to a length of 1 by dividing all elements by the L2 norm.
//	   (Crucial step to simulate real-world AI embedding vectors.)
//
std::vector<float> GenerateEmbedding(int dimension)
{
	LapTime				duration(IS_VECDBG_MESSAGE());
	std::vector<float>	dummy_vector(dimension);

	// Generate random numbers (uniform distribution centered around zero)
	std::random_device						rd;
	std::mt19937							gen(rd());
	std::uniform_real_distribution<float>	dist(-1.0f, 1.0f);

	// 1) Initialize an array with random values for the specified dimensions
	float	sum_of_squares	= 0.0f;
	for(int cnt = 0; cnt < dimension; ++cnt){
		dummy_vector[cnt]	=  dist(gen);
		sum_of_squares		+= dummy_vector[cnt] * dummy_vector[cnt];
	}

	// 2) Calculate the L2 norm(length) of the vector
	float	length = std::sqrt(sum_of_squares);

	// 3) Normalize the vector to a length of 1 by dividing all elements by the L2 norm
	//   (Crucial step to simulate real-world AI embedding vectors.)
	//
	for(int cnt = 0; cnt < dimension; ++cnt){
		dummy_vector[cnt] /= length;
	}

	// Print messages
	if(IS_VECDBG_MESSAGE()){
		std::string			strDuration	= duration.GetDurationString();
		MSG_VECPRN("Dummy Embedding[%d] - Calculation time : %s", dimension, strDuration.c_str());

		std::stringstream	ss;
		std::string			strEmbedding = MakeStringFromFloatArray(dummy_vector, "    ");
		ss << "Dummy Embedding float[" << dimension << "] = " << strEmbedding << std::endl;

		//
		// Validate the data
		//  - Recalculate to verify whether the length has truly become 1.
		//  - Vector length (sum of squares) after normalization -> (Success if close to 1)
		//
		std::string			strChecksum	= ::GetChecksumString(dummy_vector);
		ss << "Dummy Embedding length after normalization(sum of squares): " << strChecksum << " (-> Success if close to 1)" << std::endl;

		// Print all
		MSG_VECPRN("%s", ss.str().c_str());
	}

	return dummy_vector;
}

//
// Create Embedding data file
//
bool CreateEmbeddingFile(const std::string& strFile, const std::vector<float>& embedding, const std::string& strArrayName)
{
	// Check data
	if(embedding.empty()){
		ERR_VECPRN("Embedding data is empty.");
		return false;
	}
	// Check file
	if(strFile.empty()){
		ERR_VECPRN("file parameter is null or empty.");
		return false;
	}else{
		std::ifstream	ifs(strFile);
		if(ifs.is_open()){
			WAN_VECPRN("The file(%s) exists, so this file will be overwritten.", strFile.c_str());
			ifs.close();
		}
	}

	// Make strings for embedding data file
	std::string			strFloatArray	= MakeStringFromFloatArray(embedding, "    ");
	std::string			strCurDisp		= GetCurrentTimespecToDateString();

	std::stringstream	ss;
	ss <<	"#----------------------------------------------------------\n"
			"# The Embedding data file\n"
			"#\n"
			"# This file was generated using the tools provided with K2HVEC.\n"
			"#\n"
			"# CREATE: " << strCurDisp <<
			"\n#----------------------------------------------------------\n"
			"\n";

	ss <<	strArrayName << " = " << strFloatArray << "\n\n";

	ss <<	"#\n"
			"# EOF\n"
			"#\n";

	// Write to file
	std::ofstream	ofs(strFile, std::ios::out | std::ios::trunc);
	if(!ofs.is_open()){
		ERR_VECPRN("Could not open file(%s).", strFile.c_str());
		return false;
	}
	ofs << ss.str();
	ofs.close();

	return true;
}

//
// Generate Embedding data and Save it to file
//
bool GenerateEmbeddingFile(const std::string& strFile, const std::string& strArrayName, int dimension)
{
	// Generate embedding data
	std::vector<float>	embedding = GenerateEmbedding(dimension);

	if(!CreateEmbeddingFile(strFile, embedding, strArrayName)){
		return false;
	}
	return true;
}

//
// Load Generate Embedding data from file
//
bool LoadEmbeddingFile(const std::string& strFile, const std::string& strArrayName, std::vector<float>& embedding)
{
	if(strFile.empty()){
		ERR_VECPRN("file parameter is null or empty.");
		return false;
	}

	// Open file
	std::ifstream	ifs(strFile);
	if(!ifs.is_open()){
		ERR_VECPRN("Could not open file(%s).", strFile.c_str());
		return false;
	}

	// clear
	embedding.clear();

	// Loading
	std::string			line;
	bool				in_block = false;				// true means in {...} area
	while(std::getline(ifs, line)){
		if(line.empty()){
			continue;
		}
		if(in_block){
			// in {...} block
			bool	found_end_block = false;
			if(!ProjectionMatrix::ParseAndSetFloatValues(line, embedding, found_end_block)){
				ERR_VECPRN("Something wrong during reading %s keyword.", strArrayName.c_str());
				return false;
			}
			if(found_end_block){
				in_block = false;						// end of data block
			}
		}else{
			// out of {..} block
			std::string	value("");
			if(ProjectionMatrix::GetKeywordValue(line, strArrayName, false, value)){
				// Found start line ( value is "{..." )
				if('{' != value.front()){
					ERR_VECPRN("%s keyword first line is wrong format.", strArrayName.c_str());
					return false;
				}
				in_block = true;

				// trim left
				value = ProjectionMatrix::TrimWithoutComment(value.substr(1));
				if(!value.empty()){
					// The first line has float datas.
					bool	found_end_block = false;
					if(!ProjectionMatrix::ParseAndSetFloatValues(value, embedding, found_end_block)){
						ERR_VECPRN("Something wrong during reading %s keyword.", strArrayName.c_str());
						return false;
					}
					if(found_end_block){
						in_block = false;				// end of data block
					}
				}
			}else{
				continue;								// not found keyword, so skip
			}
		}
	}
	ifs.close();

	return true;
}

//---------------------------------------------------------
// Class ProjectionMatrix Variables
//---------------------------------------------------------
const char*	ProjectionMatrix::DEFAULT_BUILTIN_PROJECTION_MATRIX_NAME	= "builtin";
const char*	ProjectionMatrix::DEFAULT_BUILTIN_PROJECTION_MATRIX_VERSION	= "1.0";

const char*	ProjectionMatrix::DEFAULT_CUSTOM_PROJECTION_MATRIX_NAME		= "ExtPM";
const char*	ProjectionMatrix::DEFAULT_CUSTOM_PROJECTION_MATRIX_VERSION	= "0.0";

const char*	ProjectionMatrix::BUILTIN_KEY_PROJECTION_MATRIX_NAME		= "extern constexpr char projection_matrix_name[]";
const char*	ProjectionMatrix::BUILTIN_KEY_PROJECTION_MATRIX_VERSION		= "extern constexpr char projection_matrix_version[]";
const char*	ProjectionMatrix::BUILTIN_KEY_PROJECTION_MATRIX_BITSIZE		= "extern constexpr int projection_matrix_bitsize";
const char*	ProjectionMatrix::BUILTIN_KEY_PROJECTION_MATRIX_DIMENSION	= "extern constexpr int projection_matrix_dimension";
const char*	ProjectionMatrix::BUILTIN_KEY_PROJECTION_MATRIX_DATA		= "extern constexpr float raw_projection_matrix[]";

const char*	ProjectionMatrix::CUSTOM_KEY_PROJECTION_MATRIX_NAME			= "PROJECTION_MATRIX_NAME";
const char*	ProjectionMatrix::CUSTOM_KEY_PROJECTION_MATRIX_VERSION		= "PROJECTION_MATRIX_VERSION";
const char*	ProjectionMatrix::CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE		= "PROJECTION_MATRIX_BITSIZE";
const char*	ProjectionMatrix::CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION	= "PROJECTION_MATRIX_DIMENSION";
const char*	ProjectionMatrix::CUSTOM_KEY_PROJECTION_MATRIX_DATA			= "PROJECTION_MATRIX_DATA";

constexpr int ProjectionMatrix::MAX_MATRIX_BITSIZE;
constexpr int ProjectionMatrix::MAX_MATRIX_DIMENSION;

//---------------------------------------------------------
// Class ProjectionMatrix Methods (Utilities)
//---------------------------------------------------------
//
// Utility for Load Custom Protection Matrix
//
std::string ProjectionMatrix::TrimWithoutComment(const std::string& str)
{
	if(str.empty()){
		return "";
	}

	// find '#' in out side of quates
	std::string	text		= str;
	bool		in_quotes	= false;
	size_t	comment_pos	= std::string::npos;
	for(size_t pos = 0; pos < text.size(); ++pos){
		char tmpch	= text[pos];
		if('"' == tmpch){
			in_quotes = !in_quotes;
		}else if(!in_quotes && '#' == tmpch){
			comment_pos = pos;
			break;
		}
	}
	if(std::string::npos != comment_pos){
		text.erase(comment_pos);						// cut after '#'
	}

	// trim left/right
	size_t	start = text.find_first_not_of(" \t\r\n");
	if(start == std::string::npos){
		return "";										// empty
	}
	size_t	end = text.find_last_not_of(" \t\r\n");

	return text.substr(start, end - start + 1);
}

bool ProjectionMatrix::GetKeywordValue(const std::string& str, const std::string& keyword, bool trim_quotes, std::string& found)
{
	if(keyword.empty()){
		MSG_VECPRN("keyword is empty.");
		return false;									// wrong parameter
	}
	found = TrimWithoutComment(str);
	if(found.empty()){
		return false;									// empty line
	}
	// keyword must start at 0
	if(0 != str.rfind(keyword, 0)){
		return false;									// not found keyword
	}
	found = found.substr(keyword.size());

	// trim left
	size_t	foundpos = found.find_first_not_of(" \t\r\n");
	if(foundpos != std::string::npos){
		found = found.substr(foundpos);
	}
	if(found.empty() || '=' != found.front()){
		MSG_VECPRN("string(%s) starts with keyword, but not found = charactor.", found.c_str());
		return false;									// wrong format
	}
	found = found.substr(1);

	// trim left
	foundpos = found.find_first_not_of(" \t\r\n");
	if(foundpos != std::string::npos){
		found = found.substr(foundpos);
	}

	// cut ';' and after it
	foundpos = found.find_first_of(";");
	if(foundpos != std::string::npos){
		found.resize(foundpos);
	}

	// trim right
	foundpos = found.find_last_not_of(" \t\r\n");
	if(foundpos != std::string::npos){
		found.resize(foundpos + 1);
	}

	// cut quotes
	if(trim_quotes){
		if(found.size() < 2 || '"' != found.front() || '"' != found.back()){
			MSG_VECPRN("string(%s) starts with keyword, but wrong format for value.", found.c_str());
			return false;								// wrong format
		}
		found = found.substr(1, found.size() - 2);
	}

	if(found.empty()){
		MSG_VECPRN("string starts with keyword, but value is empty.");
		return false;									// empty
	}
	return true;
}

bool ProjectionMatrix::ParseAndSetFloatValues(const std::string& str, std::vector<float>& datalist, bool& found_end_block)
{
	found_end_block	= false;

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
					ERR_VECPRN("Found charactor after } word, so it is wrong format.");
					return false;						// wrong format
				}
			}
			break;										// reach to end
		}

		// read one float data
		float	fdata;
		if(ss >> fdata){								// read string as float
			if('f' == ss.peek() || 'F' == ss.peek()){
				ss.get();								// skip 'f' word
			}
			datalist.push_back(fdata);					// push float data
		}else{
			ERR_VECPRN("Value string is not float format.");
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

//---------------------------------------------------------
// Class ProjectionMatrix Methods(accesser)
//---------------------------------------------------------
//
// These are utiliry for accessing internal data(used by test)
//
std::string ProjectionMatrix::GetProjectionMatrixName()
{
	return ProjectionMatrix::Get().PMatrixName;
}

std::string ProjectionMatrix::GetProjectionMatrixVersion()
{
	return ProjectionMatrix::Get().PMatrixVersion;
}

int ProjectionMatrix::GetProjectionMatrixBitsize()
{
	return ProjectionMatrix::Get().PMatrixBitsize;
}

int ProjectionMatrix::GetProjectionMatrixDimension()
{
	return ProjectionMatrix::Get().PMatrixDimension;
}

const std::vector<float>& ProjectionMatrix::GetProjectionMatrix()
{
	return ProjectionMatrix::Get().PMatrixVector;
}

//---------------------------------------------------------
// Class ProjectionMatrix Methods
//---------------------------------------------------------
//
// Create/Keep singleton object
//
ProjectionMatrix& ProjectionMatrix::Get()
{
	static ProjectionMatrix	singleton;
	return singleton;
}

//
// Generate Projection Matrix
//
// This generates a projection matrix used for calculating Locality Sensitive Hashing(LSH).
// The projection matrix is generated as a flat array of type "float[bitsize * dimension]",
// intended for use as a "float[bitsize][dimension]" array.
//
// Each element is initialized using a computationally efficient uniform distribution
// (ranging from -1.0 to 1.0) rather than a computationally expensive normal distribution.
// Since the values are evenly split between positive and negative(50% each), this approach
// functions effectively as an LSH filter.
//
std::vector<float> ProjectionMatrix::GenerateProjectionMatrix(int bitsize, int dimension)
{
	LapTime				duration(IS_VECDBG_MESSAGE());

	// Returns a contiguous 1-D array instead of a 2-D array.
	std::vector<float>	projection_matrix(bitsize * dimension);
	std::random_device	rd;
	std::mt19937		gen(rd());

	// Initialize using a computationally efficient uniform distribution(range: -1.0 to 1.0).
	std::uniform_real_distribution<float>	dist(-1.0f, 1.0f);

	for(int cnt = 0; cnt < bitsize * dimension; ++cnt){
		projection_matrix[cnt] = dist(gen);
	}

	// Print messages
	if(IS_VECDBG_MESSAGE()){
		std::string	strDuration = duration.GetDurationString();

		//
		// Validate the data
		//  - Recalculate to verify whether the length has truly become 1.
		//  - Vector length (sum of squares) after normalization -> (Success if close to 1)
		//
		std::string	strChecksum	= ::GetChecksumString(projection_matrix);

		MSG_VECPRN("Projection Matrix[%d * %d] - Checksum(%s) - Calculation time : %s", bitsize, dimension, strChecksum.c_str(), strDuration.c_str());
	}

	return projection_matrix;
}

//
// Export the Projection Matrix as a string for C-SourceCode and Custom use
//
// [NOTE]
// Specifies the Projection Matrix, bit-size, and dimensions, and returns a
// multi-line string suitable for C-SourceCode and Custom file.
// This is a low-level function used for outputting to a file.
//
std::string ProjectionMatrix::ExportProjectionMatrixAsCArray(const std::vector<float>& promatrix, int bitsize, int dimension, const char* name, const char* version, bool builtin)
{
	std::stringstream	ss;
	std::string			strCurDisp	= GetCurrentTimespecToDateString();

	std::string			strSep(builtin			? ";"										: "");
	std::string			strNameKey(builtin		? BUILTIN_KEY_PROJECTION_MATRIX_NAME		: CUSTOM_KEY_PROJECTION_MATRIX_NAME);
	std::string			strVersionKey(builtin	? BUILTIN_KEY_PROJECTION_MATRIX_VERSION 	: CUSTOM_KEY_PROJECTION_MATRIX_VERSION);
	std::string			strBitsizeKey(builtin	? BUILTIN_KEY_PROJECTION_MATRIX_BITSIZE		: CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE);
	std::string			strDimensionKey(builtin	? BUILTIN_KEY_PROJECTION_MATRIX_DIMENSION	: CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION);
	std::string			strDataKey(builtin		? BUILTIN_KEY_PROJECTION_MATRIX_DATA		: CUSTOM_KEY_PROJECTION_MATRIX_DATA);
	std::string			strName;
	std::string			strVersion;
	if(VECEMPTYSTR(name)){
		strName = builtin ? DEFAULT_BUILTIN_PROJECTION_MATRIX_NAME : DEFAULT_CUSTOM_PROJECTION_MATRIX_NAME;
	}else{
		strName = name;
	}
	if(VECEMPTYSTR(version)){
		strVersion = builtin ? DEFAULT_BUILTIN_PROJECTION_MATRIX_VERSION : DEFAULT_CUSTOM_PROJECTION_MATRIX_VERSION;
	}else{
		strVersion = version;
	}

	// Put header
	if(builtin){
		ss <<	"/*\n"
				" * Projecion Matrix data for K2HVEC\n"
				" *\n"
				" * Copyright 2026 LY Corporation.\n"
				" *\n"
				" * K2HVEC is a high-performance vector database for fast vector search,\n"
				" * built on top of the K2HASH key-value store.\n"
				" *\n"
				" * For the full copyright and license information, please view\n"
				" * the license file that was distributed with this source code.\n"
				" *\n"
				" */\n"
				"\n"
				"//---------------------------------------------------------\n"
				"// The Projection Matrix data file used by K2HVEC.\n"
				"//\n"
				"// This file was generated using the tools provided with K2HVEC.\n"
				"//\n"
				"// CREATE: " << strCurDisp <<
				"\n//---------------------------------------------------------\n"
				"\n";
	}else{
		ss <<	"#----------------------------------------------------------\n"
				"# The Projection Matrix data file used by K2HVEC.\n"
				"#\n"
				"# This file was generated using the tools provided with K2HVEC.\n"
				"#\n"
				"# CREATE: " << strCurDisp <<
				"\n#----------------------------------------------------------\n"
				"\n";
	}

	// Put name and version
	ss <<	strNameKey		<< " = \""	<< strName				<< "\"" << strSep << "\n" <<
			strVersionKey	<< " = \""	<< strVersion			<< "\"" << strSep << "\n" <<
			strBitsizeKey	<< " = "	<< std::dec	<< bitsize			<< strSep << "\n" <<
			strDimensionKey	<< " = "	<< std::dec	<< dimension		<< strSep << "\n" <<
			"\n";

	// Put projection matrix data
	std::string	strFloatArray = MakeStringFromFloatArray(promatrix, "    ");
	ss << strDataKey << " = " << strFloatArray << strSep << "\n\n";

	// Put footer
	if(builtin){
		ss <<	"/*\n"
				" * Local variables:\n"
				" * tab-width: 4\n"
				" * c-basic-offset: 4\n"
				" * End:\n"
				" * vim600: noexpandtab sw=4 ts=4 fdm=marker\n"
				" * vim<600: noexpandtab sw=4 ts=4\n"
				" */\n";
	}else{
		ss <<	"#\n"
				"# EOF\n"
				"#\n";
	}

	return ss.str();
}

//
// Generate a new Projection Matrix and Create strings for C-SourceCode or Custom file
//
// [NOTE]
// Specify the bit-size and dimensions to create a new Projection Matrix, then return
// it as a multi-line string suitable for C-SourceCode and Custom files.
// This is used to create a new Projection Matrix and output it to a file.
//
std::string ProjectionMatrix::CreateProjectionMatrixAsCArray(int bitsize, int dimension, const char* name, const char* version, bool builtin)
{
	std::vector<float>	promatrix = GenerateProjectionMatrix(bitsize, dimension);

	return ExportProjectionMatrixAsCArray(promatrix, bitsize, dimension, name, version, builtin);
}

//
// Create Protection Matrix file for builtin and custom
//
// [NOTE]
// Generates a Projection Matrix data and outputs it either as a C source file(for
// builtin) or as a Custom Projection Matrix file.
// The output file can be directly integrated into the K2HVEC library(in the case
// of C source code) or loaded as a Custom Projection Matrix.
//
bool ProjectionMatrix::CreateProjectionMatrixFileAsCArray(const char* file, int bitsize, int dimension, const char* name, const char* version, bool builtin)
{
	if(VECEMPTYSTR(file)){
		ERR_VECPRN("file parameter is null or empty.");
		return false;
	}
	// check file exists
	std::ifstream	ifs(file);
	if(ifs.is_open()){
		WAN_VECPRN("The file(%s) exists, so this file will be overwritten.", file);
		ifs.close();
	}

	// create new projection matrix as file contents
	std::string		strProtectionMatrix = CreateProjectionMatrixAsCArray(bitsize, dimension, name, version, builtin);
	std::ofstream	ofs(file, std::ios::out | std::ios::trunc);
	if(!ofs.is_open()){
		ERR_VECPRN("Could not open file(%s).", file);
		return false;
	}
	ofs << strProtectionMatrix;
	ofs.close();

	return true;
}

bool ProjectionMatrix::ExportProjectionMatrixFileAsCArray(const char* file, bool builtin)
{
	if(VECEMPTYSTR(file)){
		ERR_VECPRN("file parameter is null or empty.");
		return false;
	}
	// check file exists
	std::ifstream	ifs(file);
	if(ifs.is_open()){
		WAN_VECPRN("The file(%s) exists, so this file will be overwritten.", file);
		ifs.close();
	}

	// export projection matrix as file contents
	std::string		strProtectionMatrix = ExportProjectionMatrixAsCArray(ProjectionMatrix::Get().PMatrixVector, ProjectionMatrix::Get().PMatrixBitsize, ProjectionMatrix::Get().PMatrixDimension, ProjectionMatrix::Get().PMatrixName.c_str(), ProjectionMatrix::Get().PMatrixVersion.c_str(), builtin);
	std::ofstream	ofs(file, std::ios::out | std::ios::trunc);
	if(!ofs.is_open()){
		ERR_VECPRN("Could not open file(%s).", file);
		return false;
	}
	ofs << strProtectionMatrix;
	ofs.close();

	return true;
}

//
// Load Custom Protection Matrix
//
// [NOTE]
// This is a wrapper for the SetCustomProjectionMatrix method.
//
bool ProjectionMatrix::LoadCustomProjectionMatrix(const char* file)
{
	return ProjectionMatrix::Get().SetCustomProjectionMatrix(file);
}

//---------------------------------------------------------
// Class ProjectionMatrix Methods(calculation)
//---------------------------------------------------------
//
// Calculates and Gets a binary hash value by Projection Matrix
//
std::vector<uint8_t> ProjectionMatrix::GetBinaryHash(const std::vector<float>& embedding, int bitsize)
{
	return ProjectionMatrix::Get().MakeBinaryHash(embedding, bitsize);
}

//
// Calculates and Gets a checksum value by Projection Matrix
//
float ProjectionMatrix::GetChecksum(void)
{
	return ProjectionMatrix::Get().MakeChecksum();
}

std::string  ProjectionMatrix::GetChecksumString(void)
{
	return ProjectionMatrix::Get().MakeChecksumString();
}

//
// Calculates a second hash(FNV-1a value by Projection Matrix)
//
uint64_t ProjectionMatrix::GetSecondHash(void)
{
	return ProjectionMatrix::Get().MakeSecondHash();
}

std::string ProjectionMatrix::GetSecondHashString(void)
{
	return ProjectionMatrix::Get().MakeSecondHashString();
}

//---------------------------------------------------------
// ProjectionMatrix Constructor/Destructor
//---------------------------------------------------------
ProjectionMatrix::ProjectionMatrix(): PMatrixName(""), PMatrixVersion(""), PMatrixBitsize(0), PMatrixDimension(0)
{
	if(!LoadBuiltinProjectionMatrix()){
		ERR_VECPRN("Failed to initialize Built-in Projection Matrix");
	}
}

ProjectionMatrix::~ProjectionMatrix()
{
}

//---------------------------------------------------------
// ProjectionMatrix Methods
//---------------------------------------------------------
//
// Load builtin Protection Matrix
//
// [NOTE]
// Loads the built-in Projection Matrix defined in the k2hvecbuiltinpm.cc/h files
// and sets it into internal data.
// This serves as the default Projection Matrix when no custom Projection Matrix is
// specified.
//
bool ProjectionMatrix::LoadBuiltinProjectionMatrix()
{
	if(projection_matrix_bitsize <= 0 || ProjectionMatrix::MAX_MATRIX_BITSIZE < projection_matrix_bitsize || projection_matrix_dimension <= 0 || ProjectionMatrix::MAX_MATRIX_DIMENSION < projection_matrix_dimension){
		ERR_VECPRN("Builtin Protection Matrix Variables are wrong.");
		return false;
	}

	PMatrixVector.assign(raw_projection_matrix, raw_projection_matrix + (projection_matrix_bitsize * projection_matrix_dimension));
	PMatrixName.assign(projection_matrix_name);
	PMatrixVersion.assign(projection_matrix_version);
	PMatrixBitsize		= projection_matrix_bitsize;
	PMatrixDimension	= projection_matrix_dimension;

	if(PMatrixVector.size() != (static_cast<size_t>(PMatrixBitsize) * PMatrixDimension)){
		ERR_VECPRN("Internal default Projection Matrix arrays(%lu) does not match the bitsize(%d) and dimension(%d).", PMatrixVector.size(), PMatrixBitsize, PMatrixDimension);
		return false;
	}
	return true;
}

//
// Set Custom Protection Matrix
//
// [NOTE]
// Loads the Custom Projection Matrix specified in the file and sets it into the
// internal data.
// The Custom Projection Matrix replaces the built-in Projection Matrix.
//
bool ProjectionMatrix::SetCustomProjectionMatrix(const char* file)
{
	if(VECEMPTYSTR(file)){
		ERR_VECPRN("file parameter is null or empty.");
		return false;
	}

	std::ifstream	ifs(file);
	if(!ifs.is_open()){
		ERR_VECPRN("Could not open file(%s).", file);
		return false;
	}

	// Clear existed Projection Matrix
	PMatrixName.clear();
	PMatrixVersion.clear();
	PMatrixBitsize		= 0;
	PMatrixDimension	= 0;
	PMatrixVector.clear();

	// Loading
	std::string	line;
	bool		in_block = false;						// true means in {...} area
	while(std::getline(ifs, line)){
		if(line.empty()){
			continue;
		}

		if(in_block){
			// in {...} block
			bool	found_end_block = false;
			if(!ParseAndSetFloatValues(line, PMatrixVector, found_end_block)){
				ERR_VECPRN("Something wrong during reading %s keyword.", CUSTOM_KEY_PROJECTION_MATRIX_DATA);
				return false;
			}
			if(found_end_block){
				in_block = false;						// end of data block
			}

		}else{
			// out of {..} block
			std::string	value("");
			if(GetKeywordValue(line, CUSTOM_KEY_PROJECTION_MATRIX_NAME, true, value)){
				PMatrixName.assign(value);

			}else if(GetKeywordValue(line, CUSTOM_KEY_PROJECTION_MATRIX_VERSION, true, value)){
				PMatrixVersion.assign(value);

			}else if(GetKeywordValue(line, CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE, false, value)){
				try{
					int	tmp_bitsize = std::stoi(value);
					if(tmp_bitsize <= 0 || ProjectionMatrix::MAX_MATRIX_BITSIZE < tmp_bitsize){
						ERR_VECPRN("%s keyword value(%d) is wrong/overflow, its value must be decimal string.", CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE, tmp_bitsize);
						return false;
					}
					PMatrixBitsize = tmp_bitsize;
				}catch(const std::invalid_argument& e){
					ERR_VECPRN("%s keyword value(%s) is wrong, its value must be decimal string.", CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE, value.c_str());
					return false;
				}catch(const std::out_of_range& e){
					ERR_VECPRN("%s keyword value(%s) is int out of range wrong.", CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE, value.c_str());
					return false;
				}

			}else if(GetKeywordValue(line, CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION, false, value)){
				try{
					int	tmp_dimension = std::stoi(value);
					if(tmp_dimension <= 0 || ProjectionMatrix::MAX_MATRIX_DIMENSION < tmp_dimension){
						ERR_VECPRN("%s keyword value(%d) is wrong/overflow, its value must be decimal string.", CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION, tmp_dimension);
						return false;
					}
					PMatrixDimension = tmp_dimension;
				}catch(const std::invalid_argument& e){
					ERR_VECPRN("%s keyword value(%s) is wrong, its value must be decimal string.", CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION, value.c_str());
					return false;
				}catch(const std::out_of_range& e){
					ERR_VECPRN("%s keyword value(%s) is int out of range wrong.", CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION, value.c_str());
					return false;
				}

			}else if(GetKeywordValue(line, CUSTOM_KEY_PROJECTION_MATRIX_DATA, false, value)){
				// Maybe value is "{..."
				if('{' != value.front()){
					ERR_VECPRN("%s keyword first line is wrong format.", CUSTOM_KEY_PROJECTION_MATRIX_DATA);
					return false;
				}
				in_block = true;

				// trim left
				value = TrimWithoutComment(value.substr(1));
				if(!value.empty()){
					// The first line has float datas.
					bool	found_end_block = false;
					if(!ParseAndSetFloatValues(value, PMatrixVector, found_end_block)){
						ERR_VECPRN("Something wrong during reading %s keyword.", CUSTOM_KEY_PROJECTION_MATRIX_DATA);
						return false;
					}
					if(found_end_block){
						in_block = false;				// end of data block
					}
				}
			}else{
				continue;								// not found keyword, so skip
			}
		}
	}
	ifs.close();

	// last check
	if(PMatrixName.empty()){
		ERR_VECPRN("The PMatrixVector name is not specified.");
		return false;
	}
	if(PMatrixVersion.empty()){
		ERR_VECPRN("The PMatrixVector version is not specified.");
		return false;
	}
	if(PMatrixBitsize <= 0 || PMatrixDimension <= 0 || PMatrixVector.size() != (static_cast<size_t>(PMatrixBitsize) * PMatrixDimension)){
		ERR_VECPRN("The number of PMatrixVector arrays(%lu) does not match the bitsize(%d) and dimension(%d).", PMatrixVector.size(), PMatrixBitsize, PMatrixDimension);
		return false;
	}

	return true;
}

//---------------------------------------------------------
// Class ProjectionMatrix Methods(calculation)
//---------------------------------------------------------
//
// Calculates a binary hash value by Projection Matrix
//
std::vector<uint8_t> ProjectionMatrix::MakeBinaryHash(const std::vector<float>& embedding, int bitsize)
{
	return CalBinaryHash(embedding, PMatrixVector, (0 < bitsize ? bitsize : PMatrixBitsize), PMatrixDimension);
}

//
// Calculates a checksum value by Projection Matrix
//
float ProjectionMatrix::MakeChecksum(void)
{
	return ::CalChecksum(PMatrixVector);
}

std::string ProjectionMatrix::MakeChecksumString(void)
{
	return ::GetChecksumString(PMatrixVector);
}

//
// Calculates a second hash(FNV-1a value by Projection Matrix)
//
uint64_t ProjectionMatrix::MakeSecondHash(void)
{
	return ::CalSecondHash(PMatrixVector);
}

std::string ProjectionMatrix::MakeSecondHashString(void)
{
	return ::MakeFlatStringFromUint64(::CalSecondHash(PMatrixVector), false);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
