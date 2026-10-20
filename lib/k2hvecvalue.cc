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

#include <memory>
#include <cstring>
#include <limits>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cmath>

#include "k2hveccommon.h"
#include "k2hvecdb.h"
#include "k2hvecvalue.h"
#include "k2hvecmath.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"

//---------------------------------------------------------
// About K2Hash value for K2HVEC
//---------------------------------------------------------
// The value associated with a key that K2hVec registers in K2hash uses the
// following format:
//
//	+-----------------+---------------------------------------------+
//	|                 | Embedding Length       (uint64_t: 8 byte)   |
//	|                 +---------------------------------------------+
//	| Header(24 byte) | System Embedding Length(uint64_t: 8 byte)   |
//	|                 +---------------------------------------------+
//	|                 | Content Text Length    (uint64_t: 8 byte)   |
//	|                 +---------------------------------------------+
//	|                 | Embedding Model Length (uint64_t: 8 byte)   |
//	+-----------------+---------------------------------------------+
//	| Embedding                                (float array: 4 * N) |
//	+---------------------------------------------------------------+
//	| System Embedding                         (float array: 4 * N) |
//	+---------------------------------------------------------------+
//	| Raw Content Text string                  (char array : 1 * N) |
//	+---------------------------------------------------------------+
//	| Embedding Model string(*1)               (char array : 1 * N) |
//	+---------------------------------------------------------------+
//  *1) If the allocation is at an unaligned boundary, this does not
//      pose a problem because it is a "char*".
//
// This structure is stored and read/written as a contiguous array of
// unsigned char.
// All multi-byte data members(uint64_t/float) are encoded in Little
// Endian format.
// (Little Endian is adopted because Big Endian CPUs are extremely
// rare in modern architecture, and standardizing on Little Endian
// yields better performance and compatibility.)
//
// To manage this data structure, two classes are provided: K2hVecValue
// and RawK2hVecValue.
// (Note: RawK2hVecValue is an internal helper class and is not directly
// accessible to API users.)
//
// K2hVecValue is a public class used by other components to hold
// "Embedding", "System Embedding", "Raw Content Text". and
// "Embedding Model" as C++ objects.
//
// RawK2hVecValue is an internal utility class used for assembling,
// converting, and serializing between the raw unsigned char byte
// sequence(K2hash value format) and the C++ objects managed by
// K2hVecValue.
// Endianness conversion for each data member is also implemented in
// this class.
//
// These file(k2hvecvalue.[cc|h]) implements the K2hVecValue class(public)
// and the RawK2hVecValue class(private).
//

//---------------------------------------------------------
// Internal Class : RawK2hVecValue
//---------------------------------------------------------
//
// RawK2hVecValue is a low-level access class for values stored in K2Hash.
//
// This class contains only header information as its member data.
// Values stored in K2Hash consist of a contiguous byte sequence containing
// this header information followed immediately by the actual data payload.
//
// This class holds only the header part as a member and is used to retrieve
// memory location addresses(pointers) to access the subsequent data payload
// following the header.
//
// It also constructs instances of this class from byte sequences stored in
// K2Hash, and outputs byte sequences for storing data from instances of
// this class.(Serialization)
//
// All multi-byte member data stored in K2Hash is persisted in Little Endian
// format.
// The conversion between host endianness and Little Endian is automatically
// handled by this class during serialization and deserialization.
//
// [NOTE]
// This class is intended for exclusive use by the K2hVecValue class and is
// not exposed publicly.
//
class RawK2hVecValue
{
	private:
		//
		// Type Header information
		//
		// This structure contains only the header information portion of
		// the data set as a K2Hash value.
		// NOTE: All length values are handled in bytes.
		//
		struct Header
		{
			uint64_t EmLength			= 0ULL;
			uint64_t SysEmLength		= 0ULL;
			uint64_t RawContentLength	= 0ULL;
			uint64_t EmModelLength		= 0ULL;
		};

		//
		// Static check at compiling
		//
		// Header structure size must be 32 bytes(uint64_t(8 bytes) * 4 = 32).
		//
		static_assert(sizeof(Header) == 32, "RawK2hVecValue::Header size mismatch, it must be 24 bytes");

	private:
		//
		// Header information (the sole member of this class)
		//
		Header	Head;

	private:
		//
		// Endianness conversion utilities (methods)
		//
		void CvtHeadLeToHost(void)
		{
			if(!IsHostLittleEndian()){
				Head.EmLength			= le64toh(Head.EmLength);
				Head.SysEmLength		= le64toh(Head.SysEmLength);
				Head.RawContentLength	= le64toh(Head.RawContentLength);
				Head.EmModelLength		= le64toh(Head.EmModelLength);
			}
		}

		void CvtHeadHostToLe(void)
		{
			if(!IsHostLittleEndian()){
				Head.EmLength			= htole64(Head.EmLength);
				Head.SysEmLength		= htole64(Head.SysEmLength);
				Head.RawContentLength	= htole64(Head.RawContentLength);
				Head.EmModelLength		= htole64(Head.EmModelLength);
			}
		}

		void CvtAllLeToHost(void)
		{
			if(!IsHostLittleEndian()){
				CvtHeadLeToHost();

				float*	pEmbedding = GetEmbedding();
				CvtFloatLeToHost(pEmbedding, Head.EmLength);

				float*	pSysEmbedding = GetSysEmbedding();
				CvtFloatLeToHost(pSysEmbedding, Head.SysEmLength);
			}
		}

		void CvtAllHostToLe(void)
		{
			if(!IsHostLittleEndian()){
				float*	pSysEmbedding = GetSysEmbedding();
				CvtFloatHostToLe(pSysEmbedding, Head.SysEmLength);

				float*	pEmbedding = GetEmbedding();
				CvtFloatHostToLe(pEmbedding, Head.EmLength);

				CvtHeadHostToLe();
			}
		}

	public:
		//
		// Custom deleter for RawK2hVecValue pointer
		//
		struct RawK2hVecValueDeleter{
			void operator()(RawK2hVecValue* ptr) const noexcept
			{
				delete[] reinterpret_cast<unsigned char*>(ptr);
			}
		};

		//
		// Types for RawK2hVecValue unique pointer
		//
		// unique_ptr type for automatic memory deallocation of buffers allocated
		// by MakeBytes.
		//
		using	RawK2hVecValuePtr = std::unique_ptr<RawK2hVecValue, RawK2hVecValueDeleter>;

		//
		// Attaches to K2Hash value data and constructs an instance of this class
		//
		// If endianness conversion occurs, the value data in the buffer will be
		// modified, so please pass a pointer to allocated memory or a temporary
		// memory buffer.
		// (Normally, when reading a value from K2Hash, it is retrieved into
		// allocated memory, so this is not an issue.)
		//
		// [NOTE]
		// Because the argument data may be modified, the passed buffer must not
		// be used as raw original data after calling this method.
		//
		static RawK2hVecValue* AttachBytes(unsigned char* pbytes)
		{
			RawK2hVecValue*	pVal = reinterpret_cast<RawK2hVecValue*>(pbytes);
			pVal->CvtAllLeToHost();
			return pVal;
		}

		//
		// Creates an instance of this class from a K2hVecValue object
		//
		// The generated(returned) RawK2hVecValue object is attached to memory
		// allocated as a byte sequence(unsigned char).
		// It sets the header information of the RawK2hVecValue members from the
		// K2hVecValue object parameter, and each actual data payload following
		// the header information is also stored in the allocated memory.
		// The pointer returned by this method(RawK2hVecValue object pointer and
		// the byte sequence memory) is automatically deallocated by unique_ptr.
		//
		// [NOTE]
		// The RawK2hVecValue object and actual memory returned by this function
		// have not been converted for endianness.
		// To write to K2Hash, call the Serialize() method.
		//
		static RawK2hVecValuePtr MakeBytes(const K2hVecValue& k2hvecval)
		{
			// Get each part length (NOTE: All length values are handled in bytes)
			const size_t	EmLength			= k2hvecval.GetEmbeddingLength();
			const size_t	SysEmLength			= k2hvecval.GetSysEmbeddingLength();
			const size_t	RawContentLength	= k2hvecval.GetContentTextInMainKeyLength();
			const size_t	EmModelLength		= k2hvecval.GetEmbeddingModelLength();

			// Get total byte length / Allocate buffer
			const size_t	TotalLength			= sizeof(RawK2hVecValue) + EmLength + SysEmLength + RawContentLength + EmModelLength;
			unsigned char*	pBytes				= new unsigned char[TotalLength]();		// zero clear for padding bytes

			// Set RawK2hVecValue pointer / Set length member
			RawK2hVecValue*	pVal				= reinterpret_cast<RawK2hVecValue*>(pBytes);
			pVal->Head.EmLength					= EmLength;
			pVal->Head.SysEmLength				= SysEmLength;
			pVal->Head.RawContentLength			= RawContentLength;
			pVal->Head.EmModelLength			= EmModelLength;

			// Set RawK2hVecValue members
			if(0 < EmLength){
				k2hvecval.CopyEmbedding(pVal->GetEmbedding(), EmLength);
			}
			if(0 < SysEmLength){
				k2hvecval.CopySysEmbedding(pVal->GetSysEmbedding(), SysEmLength);
			}
			if(0 < RawContentLength){
				k2hvecval.CopyContentTextInMainKey(pVal->GetRawContentText(), RawContentLength);
			}
			if(0 < EmModelLength){
				k2hvecval.CopyEmbeddingModel(pVal->GetEmbeddingModel(), EmModelLength);
			}

			// return unique_ptr
			return RawK2hVecValuePtr(pVal);
		}

	private:
		// [NOTE]
		// Declares constructors as private to restrict class instantiation strictly
		// to class methods(factory methods) and prevent direct construction from
		// outside.
		//
		RawK2hVecValue() {}
		~RawK2hVecValue() {}

		//
		// Basic Accessors for underlying data
		//
		const uint8_t* GetExtDataPos() const
		{
			const RawK2hVecValue* ptmp = this;
			return reinterpret_cast<const uint8_t*>(++ptmp);
		}

		uint8_t* GetExtDataPos()
		{
			RawK2hVecValue* ptmp = this;
			return reinterpret_cast<uint8_t*>(++ptmp);
		}

	public:
		//
		// Accessors for underlying data
		//
		size_t GetEmbeddingCount() const
		{
			if(std::numeric_limits<size_t>::max() < (Head.EmLength / sizeof(float))){
				ERR_VECPRN("Embedding length has overflow size lengh values(over size_t max).");
				return 0;
			}
			return static_cast<size_t>(Head.EmLength / sizeof(float));
		}

		size_t GetSysEmbeddingCount() const
		{
			if(std::numeric_limits<size_t>::max() < (Head.SysEmLength / sizeof(float))){
				ERR_VECPRN("System Embedding length has overflow size lengh values(over size_t max).");
				return 0;
			}
			return static_cast<size_t>(Head.SysEmLength / sizeof(float));
		}

		size_t GetRawContentTextCount() const
		{
			if(std::numeric_limits<size_t>::max() < Head.RawContentLength){
				ERR_VECPRN("Raw Content Text length has overflow size lengh values(over size_t max).");
				return 0;
			}
			return static_cast<size_t>(Head.RawContentLength);
		}

		size_t GetEmbeddingModelCount() const
		{
			if(std::numeric_limits<size_t>::max() < Head.EmModelLength){
				ERR_VECPRN("Embedding Model length has overflow size lengh values(over size_t max).");
				return 0;
			}
			return static_cast<size_t>(Head.EmModelLength);
		}

		// [NOTE]
		// Regarding the Get methods returning <float *>
		//
		// GetExtDataPos() is designed to return <uint8_t *>, which causes cppcheck
		// warnings/errors when cast directly to <float *>.
		// Since data setting is handled by the RawK2hVecValue class, <float> data
		// is guaranteed to reside in properly aligned memory.
		// Therefore, no structural code refactoring is needed here, and we handle
		// this via casting.
		// Directly writing "reinterpret_cast<const float*>(GetExtDataPos())" causes
		// an error even with aligned memory.
		// To bypass this, we cast to <void *> first instead of using
		// "// cppcheck-suppress invalidPointerCast" comment line.
		//
		const float* GetEmbedding() const
		{
			return reinterpret_cast<const float*>(static_cast<const void*>(GetExtDataPos()));
		}

		float* GetEmbedding()
		{
			return reinterpret_cast<float*>(static_cast<void*>(GetExtDataPos()));
		}

		const float* GetSysEmbedding() const
		{
			return reinterpret_cast<const float*>(static_cast<const void*>(GetExtDataPos() + Head.EmLength));
		}

		float* GetSysEmbedding()
		{
			return reinterpret_cast<float*>(static_cast<void*>(GetExtDataPos() + Head.EmLength));
		}

		const char* GetRawContentText() const
		{
			return reinterpret_cast<const char*>(GetExtDataPos() + Head.EmLength + Head.SysEmLength);
		}

		char* GetRawContentText()
		{
			return reinterpret_cast<char*>(GetExtDataPos() + Head.EmLength + Head.SysEmLength);
		}

		const char* GetEmbeddingModel() const
		{
			return reinterpret_cast<const char*>(GetExtDataPos() + Head.EmLength + Head.SysEmLength + Head.RawContentLength);
		}

		char* GetEmbeddingModel()
		{
			return reinterpret_cast<char*>(GetExtDataPos() + Head.EmLength + Head.SysEmLength + Head.RawContentLength);
		}

		//
		// Serializes for K2Hash (consumes/destroys the object)
		//
		// Returns a byte sequence for storing data in K2Hash.
		// After calling this method, the header information(member data) of this
		// object may be corrupted due to endianness conversion. Therefore, any
		// access to this object is prohibited after this method is called.
		// To enforce this rule, this method is declared with an rvalue reference
		// qualifier("&&"), meaning it can only be called on temporary objects or
		// objects explicitized via std::move().
		//
		// Example(please call it as follows):
		//
		//		RawK2hVecValuePtr pVal = RawK2hVecValue::MakeBytes(k2hvecval);
		//		size_t len = 0;
		//		const unsigned char* pBytes = std::move(*pVal).Serialize(len);
		//
		//	1) *pVal            ->	Dereferences the unique_ptr to obtain the
		//							RawK2hVecValue lvalue reference.
		//	2) std::move(*pVal) ->	Casts the instance into an rvalue reference.
		//	3) .Serialize(len)  ->	Invokes Serialize() && because the target
		//							instance is now an rvalue.
		//
		const unsigned char* Serialize(size_t& length) &&
		{
			// Check total length
			uint64_t length64 = sizeof(RawK2hVecValue) + Head.EmLength + Head.SysEmLength + Head.RawContentLength + Head.EmModelLength;
			if(std::numeric_limits<size_t>::max() < length64){
				ERR_VECPRN("RawK2hVecValue has overflow size lengh values(over size_t max).");
				return nullptr;
			}

			// Convert to Little Endian
			CvtAllHostToLe();

			// Set length
			length = static_cast<size_t>(length64);

			return reinterpret_cast<const unsigned char*>(this);
		}
};

// Static check at compiling
//
// RawK2hVecValue class size is sa same as Header structure(32 bytes).
//
static_assert(sizeof(RawK2hVecValue) == 32, "RawK2hVecValue size mismatch, it must be 32 bytes");

//---------------------------------------------------------
// Class methods : K2hVecValue
//---------------------------------------------------------
//
// Calculate the number of combinations of target binary hashes(n bit) by specifying
// the number of XOR bits(r).
// 
//	       n * (n-1) * ... * (n-r+1)
//	nCr = ---------------------------
//	        r * (r-1) * ... * 1
//
uint64_t K2hVecValue::GetXorCombinationCount(size_t total_bit_cnt, size_t r_bit_cnt)
{
	if(total_bit_cnt < r_bit_cnt){
		return 0;
	}
	if(0 == r_bit_cnt || total_bit_cnt == r_bit_cnt){
		return 1;
	}
	// symmetry
	if((total_bit_cnt / 2) < r_bit_cnt){
		r_bit_cnt = total_bit_cnt - r_bit_cnt;
	}

	uint64_t result = 1;
	for(size_t cnt = 1; cnt <= r_bit_cnt; ++cnt){
		result = result * (total_bit_cnt - cnt + 1) / cnt;
	}
	return result;
}

//
// Calculate the total number of combinations of the target binary hash(n bits) for
// XOR bit counts ranging from 0 to r bits, based on a specified XOR bit count(r).
// 
//	total = nC0 + nC1 + ... + nCr
//
uint64_t K2hVecValue::GetTotalXorCombinationCount(size_t total_bit_cnt, size_t r_bit_cnt)
{
	if(total_bit_cnt < r_bit_cnt){
		return 0;
	}
	if(0 == r_bit_cnt){
		return 1;
	}

	uint64_t total_sum = 0;
	if(total_bit_cnt == r_bit_cnt){
		if(total_bit_cnt < 64){
			total_sum = 1ULL << total_bit_cnt;					// = 2^n
		}else{
			total_sum = std::numeric_limits<uint64_t>::max();	// = 2^64(max)
		}
	}else{
		total_sum = 1;											// nC0 = 1
		for(size_t cnt = 1; cnt <= r_bit_cnt; ++cnt){
			total_sum += K2hVecValue::GetXorCombinationCount(total_bit_cnt, cnt);
		}
	}
	return total_sum;
}

//
// Calculate the total number of XOR results derived from the binary hashes(n bits)
// and return the XOR bit count reached just before hitting the upper limit of that
// total.
// 
//	return <XOR bit limit count>
//
size_t K2hVecValue::GetLimitXorBit(size_t total_bit_cnt, size_t limit_cnt)
{
	if(limit_cnt <= 1){
		return 0;
	}
	size_t valid_r_bit_cnt = 0;
	for(size_t cnt = 1; cnt <= total_bit_cnt; ++cnt){
		uint64_t total_comb = K2hVecValue::GetTotalXorCombinationCount(total_bit_cnt, cnt);

		if(limit_cnt < total_comb){
			break;
		}
		valid_r_bit_cnt = cnt;
	}
	return valid_r_bit_cnt;
}

//
// Create an array of binary hash strings derived from a binary hash string
// (where the number of characters equals the total number of binary hash bits)
// based on the XOR bit count.
//
std::vector<std::string> K2hVecValue::GetXorBinaryHashStrings(const std::string& strBinaryHash, const std::string& strPrefix, size_t r_bit_cnt)
{
	std::vector<std::string>	result;
	size_t						total_bits = strBinaryHash.size();
	if(total_bits < r_bit_cnt){
		return result;
	}
	if(0 == r_bit_cnt){
		result.push_back(strPrefix + strBinaryHash);
		return result;
	}

	// Pre-allocation of Memory
	result.reserve(K2hVecValue::GetXorCombinationCount(total_bits, r_bit_cnt));

	// Create a flag array for combinations(nCr) selection
	// (set the last r_bit_cnt elements to true)
	//
	std::vector<bool>	bit_mask(total_bits, false);
	std::fill(bit_mask.end() - r_bit_cnt, bit_mask.end(), true);

	// Sequentially generate all combinations and flip the characters '0' and '1'.
	//
	// [NOTE]
	// std::next_permutation is a standard C++ algorithm that reorders the elements
	// in a range into the next lexicographical(dictionary-order) permutation.
	//
	std::string	strBase			= strPrefix + strBinaryHash;
	size_t		prefix_length	= strPrefix.size();
	do{
		std::string	current_hash = strBase;						// original hash value

		for(size_t pos = 0; pos < total_bits; ++pos){
			if(bit_mask[pos]){
				size_t	target_idx			= prefix_length + pos;
				current_hash[target_idx]	= (current_hash[target_idx] == '0') ? '1' : '0';
			}
		}
		result.push_back(std::move(current_hash));

	}while(std::next_permutation(bit_mask.begin(), bit_mask.end()));

	return result;
}

//
// Get an array of binary hash string arrays for each XOR bit count up to r_bit_cnt
//
std::vector<std::vector<std::string>> K2hVecValue::GetTotalXorBinaryHashStrings(const std::string& strBinaryHash, const std::string& strPrefix, size_t r_bit_cnt)
{
	std::vector<std::vector<std::string>>	result;
	size_t									total_bits = strBinaryHash.size();
	if(total_bits < r_bit_cnt){
		return result;
	}

	// Pre-allocation of memory area(r_bit_cnt + 1 elements)
	result.reserve(r_bit_cnt + 1);

	for(size_t cnt = 0; cnt <= r_bit_cnt; ++cnt){
		result.push_back(std::move(K2hVecValue::GetXorBinaryHashStrings(strBinaryHash, strPrefix, cnt)));
	}
	return result;
}

//
// Creating Keys on K2Hash
//
// Two types of keys are created to store vector data values:
//	Main Key		A key used to store the vector data value(the actual data)
//					in K2Hash.
//	Neighbor Keys	Neighbor keys(XOR keys) used to search for the vector data
//					in K2Hash.
//
// The format for each key is as follows:
//	Main Key		"<Prefix>" + "<Binary Hash(bin string)>" + "-" + "<Second Hash (hex string)>"
//	Neighbor Keys	"<Prefix>" + "<XOR(0bit from Binary Hash) string>"
//					"<Prefix>" + "<XOR(1bit from Binary Hash) string>", ...
//					"<Prefix>" + "<XOR(2bit from Binary Hash) string>", ...
//					...
//
//	<Prefix>		"Projection Matrix Name" + "@" + "Projection Matrix Version" + "/"
//	<Neighbor Key>	Multiple neighbor keys are created, this returns a nested
//					 array.
//					The first element is the XOR value for 0 bits, the next
//					is an array of XOR values for 1 bit, and so on.
//					This value consists of the "<Prefix>" combined with the
//					XOR-derived binary hash(or simply "XOR Hash").
//					The XOR binary hash is generated from the original binary
//					hash by sequentially increasing the number of XOR-flipped
//					bits, starting from 0 bits (unmodified) up to 1 bit, 2 bits,
//					and so on.
//					There is an upper limit on the number of generated XOR values.
//					As the bit length of the original binary hash increases,
//					generating all possible XOR combinations would result in
//					an excessively large list. Therefore, an upper threshold
//					is set to restrict calculations to a corresponding maximum
//					number of XOR bits.
//
// Example)
//	Projection Matrix	Name	: "builtin"
//  					Version	: "1.0"
//	Second hash(64 bit)			: 0x1122334455667788
//	Binary hash Bit count		: 12
//	Binary hash(12 bit)			: 0x1010101010
//	Limit of Neighbor Key count	: 1000			(Number of keys within this value)
//
//	=>	Main Key				: "builtin@1.0/101010101010-1122334455667788"
//		Neighbor Keys			: [	[ "builtin@1.0/101010101010" ],
//								    [ "builtin@1.0/001010101010", "builtin@1.0/111010101010", ...],
//								    [ "builtin@1.0/011010101010", "builtin@1.0/000010101010", ...],
//								    [ ... ],
//								    [ ... ] ]
//	=>	Limit XOR bit			: 4 bit
//		Neighbor Key count		: 794
//
bool K2hVecValue::RawMakeKeyStrings(const std::vector<float>& embedding, std::string* pmainkey, size_t limitcount, std::vector<std::vector<std::string>>* pneighborkeys)
{
	if(!pmainkey && !pneighborkeys){
		ERR_VECPRN("parameters are wrong.");
		return false;
	}
	if(0 == embedding.size()){
		ERR_VECPRN("Cannot create the K2Hash key because the Embedding data is empty.");
		return false;
	}

	// Binary hash bit size
	int	bitsize		= K2hVecDb::GetBinHashSize();

	// Make strings for each part
	std::string	strPrefix	= ProjectionMatrix::GetProjectionMatrixName() + "@" + ProjectionMatrix::GetProjectionMatrixVersion() + "/";
	std::string	strBinHash	= MakeFlatStringFromUint8Array(ProjectionMatrix::GetBinaryHash(embedding, bitsize), true, bitsize);

	// Set main key
	if(pmainkey){
		std::string	strSecHash	= MakeFlatStringFromUint64(CalSecondHash(embedding), false);
		pmainkey->assign(strPrefix + strBinHash + "-" + strSecHash);
	}

	// Get total neighbor keys(XOR keys)
	if(pneighborkeys){
		*pneighborkeys = std::move(K2hVecValue::GetTotalXorBinaryHashStrings(strBinHash, strPrefix, K2hVecValue::GetLimitXorBit(bitsize, limitcount)));
	}

	return true;
}

//
// Get key name for indirect content text
//
std::string K2hVecValue::MakeIndirectContentKeyString(const std::string& mainkey)
{
	return mainkey + K2hVecValue::INDIRECT_CONTENTS_KEY_SUFFIX;
}

//
// Derive the Neighbor keys from the Main key
//
bool K2hVecValue::DeriveNeighborKeys(const std::string& strMainkey, std::vector<std::vector<std::string>>& NeighborKeys)
{
	if(strMainkey.empty()){
		ERR_VECPRN("Mainkey is empty.");
		return false;
	}

	// cut after lastest '-' separator
	std::string	shortkey;									// ex. "builtin@1.0/101010101010"
	size_t		pos = strMainkey.rfind('-');
	if(std::string::npos != pos) {
		shortkey = strMainkey.substr(0, pos);
	}else{
		shortkey = strMainkey;
	}

	// separate with '/'
	std::string	strPrefix;									// ex. "builtin@1.0/"
	std::string	strBinHash;									// ex. "101010101010"
	pos = shortkey.rfind('/');
	if(std::string::npos != pos) {
		strPrefix	= shortkey.substr(0, pos + 1);
		strBinHash	= shortkey.substr(pos + 1);
	}else{
		strPrefix.clear();
		strBinHash	= shortkey;
	}

	// get common values
	int		bitsize		= K2hVecDb::GetBinHashSize();
	size_t	limitcount	= K2hVecDb::GetNeighborKeysLimit();

	// make keys
	NeighborKeys = std::move(K2hVecValue::GetTotalXorBinaryHashStrings(strBinHash, strPrefix, K2hVecValue::GetLimitXorBit(bitsize, limitcount)));

	return true;
}

//
// Derive main Neighbor key from the Main key
//
bool K2hVecValue::DeriveMainNeighborKey(const std::string& strMainkey, std::string& strNeighborKey)
{
	if(strMainkey.empty()){
		ERR_VECPRN("Mainkey is empty.");
		return false;
	}

	// Make neighbor key name from main key
	size_t		pos = strMainkey.rfind('-');
	if(std::string::npos == pos) {
		ERR_VECPRN("Failed to make main neighbor key.");
		return false;
	}
	strNeighborKey = strMainkey.substr(0, pos);

	return true;
}

//---------------------------------------------------------
// Class methods : Remove key and neighbor keys
//---------------------------------------------------------
// [NOTE]
// Since the Remove methods do not manipulate internal data, it is provided as a class methods.
//
// Remove mainkey name in neighbor key's subkey
//
bool K2hVecValue::RemoveNeighborKey(K2HShm* pK2hobj, const std::string& key)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}

	// Get neighbor key from main key
	std::string	neighborkey;
	if(!K2hVecValue::DeriveMainNeighborKey(key, neighborkey)){
		ERR_VECPRN("Failed to generate index keys for main key(%s).", key.c_str());
		return false;
	}

	// Remove key name in neighbor key's subkey
	if(!pK2hobj->Remove(neighborkey.c_str(), key.c_str())){
		ERR_VECPRN("Failed to remove main key(%s) index in neighbor key(%s) subkey.", key.c_str(), neighborkey.c_str());
		return false;
	}
	DMP_VECPRN("Removed main key(%s) index in index key(%s) subkey.", key.c_str(), neighborkey.c_str());

	return true;
}

//
// Remove main key and neighbor key's subkey
//
bool K2hVecValue::Remove(K2HShm* pK2hobj, const std::string& key, bool mainkeyonly)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}

	// Remove main key
	if(!pK2hobj->Remove(key.c_str(), true)){
		ERR_VECPRN("Failed to remove key(%s).", key.c_str());
		return false;
	}

	// Remove indirect key if it is existed
	std::string	indirectkey = K2hVecValue::MakeIndirectContentKeyString(key);
	if(!pK2hobj->Remove(indirectkey.c_str(), true)){
		MSG_VECPRN("Filed to remove indirect key(%s), it maybe not the min key does not have indirect key. so continue...", indirectkey.c_str());
	}

	// Remove main neighbor key
	if(!mainkeyonly){
		if(!K2hVecValue::RemoveNeighborKey(pK2hobj, key)){
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------
// Class : K2hVecValue
//---------------------------------------------------------
K2hVecValue::K2hVecValue()
{
}

K2hVecValue::K2hVecValue(const std::vector<float>& em, const std::string& content, const std::string& model, const std::vector<float>* psysem)
{
	if(!Initialize(em, content, model, psysem)){
		ERR_VECPRN("Failed to initialize member variables in constructor.");
		// Reset
		clear();
	}
}

K2hVecValue::~K2hVecValue()
{
}

//
// Status
//
bool K2hVecValue::empty(void) const
{
	return (ValueState::UNSET == ValueStatus);
}

//
// Clear
//
void K2hVecValue::clear(void)
{
	Embedding.clear();
	SysEmbedding.clear();
	ContentText.clear();
	EmbeddingModel.clear();
	ValueStatus = ValueState::UNSET;
	ContentType	= ContentValueType::DIRECT;
}

//
// Initialize (set values directly)
//
bool K2hVecValue::Initialize(const std::vector<float>& em, const std::string& content, const std::string& model, const std::vector<float>* psysem)
{
	if(em.empty()){
		ERR_VECPRN("Embedding data is empty.");
		return false;
	}
	if(model.empty()){
		ERR_VECPRN("Embedding model string is empty.");
		return false;
	}
	if(psysem && psysem->empty()){
		MSG_VECPRN("Specied empty System Embedding.");
	}

	Embedding		= em;
	ContentText		= content;
	EmbeddingModel	= model;
	if(psysem && !psysem->empty()){
		SysEmbedding = *psysem;
	}else{
		SysEmbedding.clear();
	}

	ValueStatus		= ValueState::FULLY_SET;
	if(ContentText.size() <= K2hVecDb::GetContentDirectLimit()){
		ContentType	= ContentValueType::DIRECT;
	}else{
		ContentType	= ContentValueType::INDIRECT;
	}

	return true;
}

//
// Initialize values from a value(byte array pointer) read from K2Hash.
//
// [NOTE]
// The contents of the byte array(pbytes) may be modified if endian conversion occurs.
//
bool K2hVecValue::Initialize(unsigned char* pbytes, size_t length)
{
	if(!pbytes){
		ERR_VECPRN("pbytes parameter is NULL.");
		return false;
	}

	// RawK2hVecValue object size is 32 bytes as minimum.
	if(length < 32){
		ERR_VECPRN("pbytes parameter is short(< 32).");
		return false;
	}

	// clear
	clear();

	// Attach
	RawK2hVecValue*	rawobj = RawK2hVecValue::AttachBytes(pbytes);

	// Get each length and data pointer
	size_t			EmbeddingCount		= rawobj->GetEmbeddingCount();
	size_t			SysEmbeddingCount	= rawobj->GetSysEmbeddingCount();
	size_t			RawContentCount		= rawobj->GetRawContentTextCount();
	size_t			EmModelCount		= rawobj->GetEmbeddingModelCount();
	if(length < (sizeof(RawK2hVecValue) + (EmbeddingCount * sizeof(float)) + (SysEmbeddingCount * sizeof(float)) + RawContentCount + EmModelCount)){
		ERR_VECPRN("The specified data length(%lu) differs from the data length(%lu) of pbytes.", length, (sizeof(RawK2hVecValue) + (EmbeddingCount * sizeof(float)) + (SysEmbeddingCount * sizeof(float)) + RawContentCount + EmModelCount));
		return false;
	}

	const float*	pEmbedding			= rawobj->GetEmbedding();
	const float*	pSysEmbedding		= rawobj->GetSysEmbedding();
	const char*		pRawContentText		= rawobj->GetRawContentText();
	const char*		pEmbeddingModel		= rawobj->GetEmbeddingModel();

	// assign all member
	if(0 < EmbeddingCount && pEmbedding){
		Embedding.assign(pEmbedding, pEmbedding + EmbeddingCount);
	}
	if(0 < SysEmbeddingCount && pSysEmbedding){
		SysEmbedding.assign(pSysEmbedding, pSysEmbedding + SysEmbeddingCount);
	}
	if(0 < EmModelCount && pEmbeddingModel){
		EmbeddingModel.assign(pEmbeddingModel, pEmbeddingModel + EmModelCount);
	}
	if(0 < RawContentCount && pRawContentText){
		if(RawContentCount == K2hVecValue::INDIRECT_TYPE_CONTENTS_LENGTH && 0 == std::strncmp(pRawContentText, K2hVecValue::INDIRECT_TYPE_CONTENTS, K2hVecValue::INDIRECT_TYPE_CONTENTS_LENGTH)){
			// Indirect content text
			ValueStatus = ValueState::UNLOADED_CONTENT_SET;
			ContentType	= ContentValueType::INDIRECT;
		}else{
			// Direct content text
			ContentText.assign(pRawContentText, pRawContentText + RawContentCount);
			ValueStatus = ValueState::FULLY_SET;
			ContentType	= ContentValueType::DIRECT;
		}
	}else{
		// ContentText is empty
		ValueStatus = ValueState::FULLY_SET;
		ContentType	= ContentValueType::DIRECT;
	}

	return true;
}

//---------------------------------------------------------
// Creating Keys on K2Hash
//---------------------------------------------------------
bool K2hVecValue::MakeMainKeyStrings(std::string& mainkey, std::string& neighborkey) const
{
	if(0 == Embedding.size()){
		ERR_VECPRN("Cannot create the K2Hash key because the Embedding data is empty.");
		return false;
	}
	if(!K2hVecValue::RawMakeKeyStrings(Embedding, &mainkey, K2hVecDb::GetNeighborKeysLimit(), nullptr)){
		ERR_VECPRN("Failed to make main key.");
		return false;
	}
	if(!K2hVecValue::DeriveMainNeighborKey(mainkey, neighborkey)){		// ex. "builtin@1.0/101010101010"
		ERR_VECPRN("Failed to make main neighbor key.");
		return false;
	}
	return true;
}

bool K2hVecValue::MakeMainKeyString(std::string& mainkey) const
{
	if(0 == Embedding.size()){
		ERR_VECPRN("Cannot create the K2Hash key because the Embedding data is empty.");
		return false;
	}
	return K2hVecValue::RawMakeKeyStrings(Embedding, &mainkey, K2hVecDb::GetNeighborKeysLimit(), nullptr);
}

bool K2hVecValue::MakeNeighborKeys(std::vector<std::vector<std::string>>& neighborkeys) const
{
	if(0 == Embedding.size()){
		ERR_VECPRN("Cannot create the K2Hash key because the Embedding data is empty.");
		return false;
	}
	return K2hVecValue::RawMakeKeyStrings(Embedding, nullptr, K2hVecDb::GetNeighborKeysLimit(), &neighborkeys);
}

//---------------------------------------------------------
// Set value
//---------------------------------------------------------
//
// Create Main Key and Set neighbor key
//
bool K2hVecValue::Set(K2HShm* pK2hobj, const std::string* pPass, const time_t* pexpire)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}
	if(0 == Embedding.size()){
		ERR_VECPRN("Cannot create the K2Hash key because the Embedding data is empty.");
		return false;
	}

	// Parameters
	const char*	encpass = nullptr;
	if(pPass && !pPass->empty()){
		encpass = pPass->c_str();
	}

	// Content text
	if(ValueState::UNSET == ValueStatus){
		ERR_VECPRN("The data has not been loaded(set) yet.");
		return false;
	}else if(ValueState::UNLOADED_CONTENT_SET == ValueStatus){
		// Try to load content text
		if(!LoadContentText(pK2hobj, true, encpass)){
			ERR_VECPRN("Failed to try to load indirect content text.");
			return false;
		}
	}

	// Get keys
	std::string	mainkey;									// ex. "builtin@1.0/101010101010-1122334455667788"
	std::string	neighborkey;								// ex. "builtin@1.0/101010101010"
	if(!MakeMainKeyStrings(mainkey, neighborkey)){
		ERR_VECPRN("Something error occurred for creating keys.");
		return false;
	}

	// Make RawK2hVecValue object and K2hash value raw data
	RawK2hVecValue::RawK2hVecValuePtr	rawPtr		= RawK2hVecValue::MakeBytes(*this);
	size_t								rawLength	= 0;
	const unsigned char*				pRawBytes	= std::move(*rawPtr).Serialize(rawLength);
	if(0 == rawLength || !pRawBytes){
		ERR_VECPRN("Could not make raw data for K2hash value.");
		return false;
	}

	// Set main key and value
	if(!pK2hobj->Set(reinterpret_cast<const unsigned char*>(mainkey.c_str()), mainkey.size() + 1, pRawBytes, rawLength, encpass, pexpire)){
		ERR_VECPRN("Filed to create key(%s).", mainkey.c_str());
		return false;
	}
	DMP_VECPRN("Created key(%s).", mainkey.c_str());

	// Inidirect type
	if(ContentValueType::INDIRECT == ContentType){
		// indirect key name for content text
		std::string	indirectkey = K2hVecValue::MakeIndirectContentKeyString(mainkey);
		if(!pK2hobj->Set(reinterpret_cast<const unsigned char*>(indirectkey.c_str()), indirectkey.size() + 1, reinterpret_cast<const unsigned char*>(ContentText.c_str()), ContentText.size() + 1, encpass, pexpire)){
			ERR_VECPRN("Filed to create key(%s).", indirectkey.c_str());
			return false;
		}
	}

	// Get neighbor key's subkeys
	bool		need_free = true;
	K2HSubKeys	tmpSubKeys;
	K2HSubKeys*	pSubKeys;
	if(nullptr == (pSubKeys = pK2hobj->GetSubKeys(neighborkey.c_str()))){
		pSubKeys	= &tmpSubKeys;
		need_free	= false;
	}

	// Add subkey(mainkey) to subkeys list
	pSubKeys->insert(mainkey.c_str());

	// Set(Create) neighbor key
	bool result;
	if(false == (result = pK2hobj->Set(neighborkey.c_str(), nullptr, pSubKeys, false, encpass, pexpire))){
		ERR_VECPRN("Filed to insert %s subkey to key(%s).", mainkey.c_str(), neighborkey.c_str());
	}else{
		DMP_VECPRN("Inserted %s index key to key(%s).", mainkey.c_str(), neighborkey.c_str());
	}
	if(need_free){
		K2H_Delete(pSubKeys);
	}

	return result;
}

//---------------------------------------------------------
// Load/Get value
//---------------------------------------------------------
//
// Load values from k2hash key
//
bool K2hVecValue::Load(const K2HShm* pK2hobj, const std::string& key, bool checkattr, const std::string* pPass, bool force_content_load)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}

	// Parameters
	const char*	encpass = nullptr;
	if(pPass && !pPass->empty()){
		encpass = pPass->c_str();
	}

	// Get value
	ssize_t			valueLength;
	unsigned char*	pbyValue = nullptr;
	if(0 > (valueLength = pK2hobj->Get(reinterpret_cast<const unsigned char*>(key.c_str()), key.size() + 1, &pbyValue, checkattr, encpass)) || !pbyValue){
		ERR_VECPRN("The value for the %s key was not found in K2Hash.", key.c_str());
		K2H_Free(pbyValue);
		return false;
	}

	// Set/Load to K2hVecValue
	if(!Initialize(pbyValue, static_cast<size_t>(valueLength))){
		ERR_VECPRN("Failed to load the value for key(%s) by wrong format.", key.c_str());
		K2H_Free(pbyValue);
		return false;
	}
	K2H_Free(pbyValue);

	// Content text
	if(force_content_load){
		if(!LoadContentText(pK2hobj, checkattr, encpass)){
			return false;
		}
	}

	return true;
}

//
// Check and Load Content Text for Indirect type
//
bool K2hVecValue::LoadContentText(const K2HShm* pK2hobj, bool checkattr, const char* encpass)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}

	if(ValueState::UNSET == ValueStatus){
		ERR_VECPRN("The value is not loaded yet.");
		return false;

	}else if(ValueState::UNLOADED_CONTENT_SET == ValueStatus){
		// Get indirect content key
		std::string	mainkey;
		if(!MakeMainKeyString(mainkey)){
			return false;
		}
		std::string	indirectkey = K2hVecValue::MakeIndirectContentKeyString(mainkey);

		// Get content text
		ssize_t			valueLength;
		unsigned char*	pbyValue = nullptr;
		if(0 > (valueLength = pK2hobj->Get(reinterpret_cast<const unsigned char*>(indirectkey.c_str()), indirectkey.size() + 1, &pbyValue, checkattr, encpass)) || !pbyValue){
			ERR_VECPRN("The value for the %s key was not found in K2Hash.", indirectkey.c_str());
			K2H_Free(pbyValue);
			return false;
		}

		// Set content text
		ContentText.assign(reinterpret_cast<const char*>(pbyValue), static_cast<size_t>(valueLength));
		K2H_Free(pbyValue);
	}
	return true;
}

bool K2hVecValue::Get(const K2HShm* pK2hobj, const std::string& key, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr, const std::string* pPass)
{
	// Load values from k2hash
	if(!Load(pK2hobj, key, checkattr, pPass, true)){	// force load content text if it is indirect type
		return false;
	}
	em		= Embedding;
	sysem	= SysEmbedding;
	model	= EmbeddingModel;
	content = ContentText;

	return true;
}

bool K2hVecValue::GetEmbedding(std::vector<float>& em) const
{
	if(empty()){
		return false;
	}
	em = Embedding;
	return true;
}

bool K2hVecValue::GetSysEmbedding(std::vector<float>& sysem) const
{
	if(empty()){
		return false;
	}
	sysem = SysEmbedding;
	return true;
}

bool K2hVecValue::GetContentText(std::string& content, const K2HShm* pK2hobj, bool checkattr, const char* encpass)
{
	if(empty()){
		return false;
	}
	if(!pK2hobj && ValueState::FULLY_SET != ValueStatus){
		ERR_VECPRN("The content text is not loaded yet.");
		return false;
	}
	if(!LoadContentText(pK2hobj, checkattr, encpass)){
		return false;
	}
	content = ContentText;

	return true;
}

bool K2hVecValue::GetEmbeddingModel(std::string& model) const
{
	if(empty()){
		return false;
	}
	model = EmbeddingModel;
	return true;
}

//---------------------------------------------------------
// Search
//---------------------------------------------------------
//
// Check for a match with the detected data
//
// This method checks for the existence of a value associated with the specified
// key and verifies whether that value meets the matching criteria.
//
// The retrieved value is validated against the following conditions:
//
//	Embedding Model Name:
//		If the "model" parameter is not empty, the value's Embedding Model must
//		match it.
//		If the "model" parameter is empty, this check is skipped.
//
//	System Embedding:
//		If "pSysEmbedding" is not nullptr, the cosine similarity against value's
//		System Embedding is calculated. A match is determined if the result is
//		greater than or equal to "score". 
//		If "pSysEmbedding" is nullptr, this check is skipped.
//
//	Embedding:
//		The value's Embedding is checked against "embedding" using cosine similarity.
//		The cosine similarity must be greater than or equal to "score".
//		If a match is found, the method returns true and assigns the calculated
//		cosine similarity value to "cosval" parameter.
//
// Upon a successful match, the method returns true, sets the cosine similarity
// value in "cosval", and populates this object's member variables with data
// extracted from the value.
// Please retrieve the ContentText from the object when a match is found.
//
bool K2hVecValue::LoadAndMatch(K2HShm* pK2hobj, const std::string& key, const std::vector<float>& em, float score, float& cosval, const std::string& model, const std::vector<float>* pSysEmbedding, bool checkattr, const char* encpass)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}
	MSG_VECPRN("Target key=%s, Score limit=%s", key.c_str(), MakeFlatStringFromFloat(score).c_str());

	// Get value with checkattr and encpass parameter
	unsigned char*	byValue = nullptr;
	ssize_t			length;
	if(0 > (length = pK2hobj->Get(reinterpret_cast<const unsigned char*>(key.c_str()), key.size() + 1, &byValue, checkattr, encpass)) || !byValue){
		// Not found
		MSG_VECPRN("Not found %s key in k2hash.", key.c_str());
		K2H_Free(byValue);
		return false;
	}
	if(static_cast<size_t>(length) <= sizeof(RawK2hVecValue)){
		// Wrong value length
		WAN_VECPRN("The value length(%ld) for %s key is incorrect, a minimum of %lu byte length is required.", length, key.c_str(), sizeof(RawK2hVecValue));
		K2H_Free(byValue);
		return false;
	}

	// Found correct value
	clear();									// Clear internal variables(this method repeats the data loading process).

	// Loads the read binary data into this class object
	if(!Initialize(byValue, length)){
		// Wrong value
		WAN_VECPRN("The data format is incorrect, the value read from k2hash(key=%s) cannot be mapped to this class data.", key.c_str());
		K2H_Free(byValue);
		return false;
	}
	K2H_Free(byValue);

	// Check model (if model is empty, all Embedding Models are matched.)
	if(!model.empty() && 0 != strcasecmp(EmbeddingModel.c_str(), model.c_str())){
		MSG_VECPRN("The data model(%s) of the value set for the key(%s) differs from the target model(%s).", EmbeddingModel.c_str(), key.c_str(), model.c_str());
		clear();
		return false;
	}

	// Check system embedding (if pSysEmbedding is empty, all items are matched.)
	if(pSysEmbedding && !pSysEmbedding->empty()){
		float syscos = CosineSimilarity(SysEmbedding, *pSysEmbedding);
		if(!std::isfinite(syscos)){
			MSG_VECPRN("The cosine similarity between the system embedding of the value set for the key(%s) and the target system embedding is invalid(NaN/Inf).", key.c_str());
			clear();
			return false;
		}

		float tmpsyscos;
		if(1.0f < syscos){
			tmpsyscos = 1.0f - (syscos - 1.0f);
		}else{
			tmpsyscos = syscos;
		}
		if(tmpsyscos < score){
			MSG_VECPRN("The cosine similarity(%.10g) between the system embedding of the value set for the key(%s) and the target system embedding differs by at least %.10g(therefore, they do not match).", syscos, key.c_str(), (1.0f - score));
			clear();
			return false;
		}
	}

	// Check embedding
	cosval = CosineSimilarity(Embedding, em);
	if(!std::isfinite(cosval)){
		MSG_VECPRN("The cosine similarity between the Embedding of the value set for the key(%s) and the target Embedding is invalid(NaN/Inf).", key.c_str());
		clear();
		return false;
	}

	float tmpcos;
	if(1.0f < cosval){
		tmpcos = 1.0f - (cosval - 1.0f);
	}else{
		tmpcos = cosval;
	}
	if(score > tmpcos){
		MSG_VECPRN("The cosine similarity(%.10g) between the Embedding of the value set for the key(%s) and the target Embedding differs by at least %.10g(therefore, they do not match).", tmpcos, key.c_str(), (1.0f - score));
		clear();
		return false;
	}

	//
	// Match
	//
	MSG_VECPRN("Target key=%s, Score limit=%s, Result Cosine Similarity=%s(%s)", key.c_str(), MakeFlatStringFromFloat(score).c_str(), MakeFlatStringFromFloat(cosval).c_str(), MakeFlatStringFromFloat(tmpcos).c_str());

	return true;
}

bool K2hVecValue::Search(K2HShm* pK2hobj, const std::vector<float>& em, float score, const std::string& model, const std::vector<float>* pSysEmbedding, bool checkattr, const char* encpass, bool force_content_load)
{
	if(!pK2hobj){
		ERR_VECPRN("K2HShm object pointer is NULL.");
		return false;
	}

	//
	// Create neighbor keys based on search priority
	//
	std::string								mainkey;
	std::vector<std::vector<std::string>>	neighborkeys;
	if(!K2hVecValue::RawMakeKeyStrings(em, &mainkey, K2hVecDb::GetNeighborKeysLimit(), &neighborkeys)){
		ERR_VECPRN("Failed to create keys for searching with priority.");
		return false;
	}

	//
	// Check main key
	//
	float	cosval = 0.0f;
	K2hVecValue	tmpVecValue;										// temporary value
	if(tmpVecValue.LoadAndMatch(pK2hobj, mainkey, em, score, cosval, model, pSysEmbedding, checkattr, encpass)){
		// match
		MSG_VECPRN("Found same embedding(main key(%s)) value.", mainkey.c_str());
		*this = std::move(tmpVecValue);								// move temporary value to this

		// Content Text
		if(force_content_load){
			if(!LoadContentText(pK2hobj, checkattr, encpass)){
				return false;
			}
		}
		return true;
	}

	//
	// Search neighbor keys
	//
	// The search is conducted layer by layer(by XOR bit-flip distance)
	// in order of increasing depth.
	// If matching candidates are found within a layer, the closest one
	// within that layer is returned.
	//
	bool		found			= false;
	float		lastestCosValue	= 0.0f;				// Last cosine similarity
	K2hVecValue	tmpLastVecValue;					// last value
	std::string	strLastKey;
	for(const auto& keylist: neighborkeys){							// keylist	= std::vector<std::string>
		for(const auto& key: keylist){								// key		= std::string
			// Get subkeys(naighbor keys) for main key
			K2HSubKeys*	pSubKey;
			if(nullptr != (pSubKey = pK2hobj->GetSubKeys(key.c_str(), checkattr))){
				// Found and got subkeys -> convert std::string array
				MSG_VECPRN("Found subkeys in naighbor key(%s).", key.c_str());

				std::vector<std::string>	subkeylist;
				if(0 < pSubKey->StringArray(subkeylist)){
					// Loop : Get all subkey's value and check it.
					for(const auto& subkey: subkeylist){
						// [NOTE]
						// Since subkeylist is an array of strings generated from
						// binary data, the actual string length of an individual
						// subkey may differ from its size.
						// Because K2HShm::Get() is called with the key typed
						// "const unsigned char*" within the LoadAndMatch() function,
						// the actual string length must match the std::string size.
						//
						size_t		pos				= subkey.find('\0');
						std::string	strNaighborKey	= (std::string::npos != pos) ? subkey.substr(0, pos) : subkey;

						// Check value for main key
						cosval = 0.0f;
						if(tmpVecValue.LoadAndMatch(pK2hobj, strNaighborKey, em, score, cosval, model, pSysEmbedding, checkattr, encpass)){
							// Matched
							MSG_VECPRN("Found naighbor key(%s) value exceeding the cosine similarity threshold.", strNaighborKey.c_str());

							if(1.0f < cosval){
								cosval = 1.0f - (cosval - 1.0f);
							}
							if(lastestCosValue < cosval){
								// Found: Closest cosine similarity
								MSG_VECPRN("Found naighbor key(%s) value most exceeding the cosine similarity threshold.", strNaighborKey.c_str());
								lastestCosValue	= cosval;
								tmpLastVecValue = std::move(tmpVecValue);	// move temporary value to lastest object
								strLastKey		= key;
								found			= true;
							}
						}
					}
				}
				K2H_Delete(pSubKey);
			}
		}
		if(found){
			MSG_VECPRN("Found naighbor key(%s) value most exceeding the cosine similarity threshold.", strLastKey.c_str());
			*this = std::move(tmpLastVecValue);								// move temporary value to this

			// Content Text
			if(force_content_load){
				if(!LoadContentText(pK2hobj, checkattr, encpass)){
					return false;
				}
			}
			return true;
		}
	}
	// Not foud
	MSG_VECPRN("Not found value for key(%s).", mainkey.c_str());

	return false;
}

//---------------------------------------------------------
// Get infomration
//---------------------------------------------------------
//
// Get length(byte count) infomration
//
size_t K2hVecValue::GetEmbeddingLength(void) const
{
	return Embedding.size() * sizeof(float);
}

size_t K2hVecValue::GetSysEmbeddingLength(void) const
{
	return SysEmbedding.size() * sizeof(float);
}

size_t K2hVecValue::GetContentTextInMainKeyLength(void) const
{
	// [NOTE]
	// This string length does not include the terminating '\0' character.
	//
	if(ContentValueType::DIRECT == ContentType){
		return ContentText.size();
	}else{
		return K2hVecValue::INDIRECT_TYPE_CONTENTS_LENGTH;
	}
}

size_t K2hVecValue::GetEmbeddingModelLength(void) const
{
	// [NOTE]
	// This string length does not include the terminating '\0' character.
	//
	return EmbeddingModel.size();
}

//
// Copy internal datas
//
void K2hVecValue::CopyEmbedding(float* dest, size_t length) const
{
	std::memcpy(dest, Embedding.data(), length);
}

void K2hVecValue::CopySysEmbedding(float* dest, size_t length) const
{
	std::memcpy(dest, SysEmbedding.data(), length);
}

void K2hVecValue::CopyContentTextInMainKey(char* dest, size_t length) const
{
	if(ContentValueType::DIRECT == ContentType){
		std::memcpy(dest, ContentText.data(), length);
	}else{
		std::memcpy(dest, INDIRECT_TYPE_CONTENTS, length);
	}
}

void K2hVecValue::CopyEmbeddingModel(char* dest, size_t length) const
{
	std::memcpy(dest, EmbeddingModel.data(), length);
}

//
// Move member values
//
// [NOTE]
// These methods retrieves the member values.
// However, it transfers ownership, the object is destroyed in the process of optimizing
// for speed, effectively resetting it.
// In other words, the object's member value becomes empty after this method is called.
//

bool K2hVecValue::MoveEmbedding(std::vector<float>& em)
{
	if(empty()){
		ERR_VECPRN("This object is empty, so could not move to embedding data.");
		return false;
	}
	em = std::move(Embedding);
	Embedding.clear();				// safeguard
	return true;
}

bool K2hVecValue::MoveSysEmbedding(std::vector<float>& sysem)
{
	if(empty()){
		ERR_VECPRN("This object is empty, so could not move to system embedding data.");
		return false;
	}
	sysem = std::move(SysEmbedding);
	SysEmbedding.clear();			// safeguard
	return true;
}

bool K2hVecValue::MoveContentText(std::string& content)
{
	if(empty()){
		ERR_VECPRN("This object is empty, so could not move to content text.");
		return false;
	}
	content = std::move(ContentText);
	ContentText.clear();			// safeguard
	return true;
}

bool K2hVecValue::MoveEmbeddingModel(std::string& model)
{
	if(empty()){
		ERR_VECPRN("This object is empty, so could not move to content text.");
		return false;
	}
	model = std::move(EmbeddingModel);
	EmbeddingModel.clear();			// safeguard
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
