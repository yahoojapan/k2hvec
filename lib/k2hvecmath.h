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

#ifndef	K2HVECMATH_H
#define	K2HVECMATH_H

#include <vector>
#include <cstdint>

//---------------------------------------------------------
// Utility for Mathematics and calculation
//---------------------------------------------------------
//
// Make string
//
std::string MakeStringFromFloatArray(const std::vector<float>& farray, const std::string& indent);
std::string MakeStringFromUint8Array(const std::vector<uint8_t>& uintarray, bool printbit, const std::string& indent);
std::string MakeFlatStringFromUint8Array(const std::vector<uint8_t>& uintarray, bool printbit, int bitsize = 0);
std::string MakeFlatStringFromUint64(uint64_t value, bool printbit);
std::string MakeFlatStringFromFloat(float value);

//
// Calculation of Check Sum
//
std::string GetChecksumString(const std::vector<float>& farray);
float CalChecksum(const std::vector<float>& farray);

//
// Calculation of Cosine Similarity
//
float CosineSimilarity(const std::vector<float>& data1, const std::vector<float>& data2, bool simple = false);
float CosineSimilarityStandard(const std::vector<float>& data1, const std::vector<float>& data2);
float CosineSimilaritySimple(const std::vector<float>& data1, const std::vector<float>& data2);

//
// Calculate binary hash / FNV-1a hash(scoundary hash)
//
std::vector<uint8_t> CalBinaryHash(const std::vector<float>& embedding, const std::vector<float>& projection_matrix, int bitsize, int dimension);
uint64_t CalSecondHash(const std::vector<float>& embedding);
uint64_t CalFnvA1Hash(const char* str);

//
// Embedding ( for test )
//
std::vector<float> GenerateEmbedding(int dimension);
bool CreateEmbeddingFile(const std::string& strFile, const std::vector<float>& embedding, const std::string& strArrayName);
bool GenerateEmbeddingFile(const std::string& strFile, const std::string& strArrayName, int dimension);
bool LoadEmbeddingFile(const std::string& strFile, const std::string& strArrayName, std::vector<float>& embedding);

//---------------------------------------------------------
// Class ProjectionMatrix
//---------------------------------------------------------
// [NOTE]
// ProjectionMatrix is created solely as a singleton object.
// This ensures automatic initialization upon library loading
//  while avoiding the Static Initialization Order Fiasco(SIOF).
//
class ProjectionMatrix
{
	private:
		static const char*	DEFAULT_BUILTIN_PROJECTION_MATRIX_NAME;
		static const char*	DEFAULT_BUILTIN_PROJECTION_MATRIX_VERSION;

		static const char*	DEFAULT_CUSTOM_PROJECTION_MATRIX_NAME;
		static const char*	DEFAULT_CUSTOM_PROJECTION_MATRIX_VERSION;

		static const char*	BUILTIN_KEY_PROJECTION_MATRIX_NAME;
		static const char*	BUILTIN_KEY_PROJECTION_MATRIX_VERSION;
		static const char*	BUILTIN_KEY_PROJECTION_MATRIX_BITSIZE;
		static const char*	BUILTIN_KEY_PROJECTION_MATRIX_DIMENSION;
		static const char*	BUILTIN_KEY_PROJECTION_MATRIX_DATA;

		static const char*	CUSTOM_KEY_PROJECTION_MATRIX_NAME;
		static const char*	CUSTOM_KEY_PROJECTION_MATRIX_VERSION;
		static const char*	CUSTOM_KEY_PROJECTION_MATRIX_BITSIZE;
		static const char*	CUSTOM_KEY_PROJECTION_MATRIX_DIMENSION;
		static const char*	CUSTOM_KEY_PROJECTION_MATRIX_DATA;

		//
		// Projection Matrix Datas
		//
		std::string			PMatrixName;
		std::string			PMatrixVersion;
		int					PMatrixBitsize;
		int					PMatrixDimension;
		std::vector<float>	PMatrixVector;

	private:
		//
		// Get singleton object
		//
		static ProjectionMatrix& Get();

		//
		// Export Projection Matrix
		//
		static std::string ExportProjectionMatrixAsCArray(const std::vector<float>& promatrix, int bitsize, int dimension, const char* name = nullptr, const char* version = nullptr, bool builtin = false);

		//
		// Constructor / Destructor
		//
		ProjectionMatrix();
		~ProjectionMatrix();

		//
		// Initialize / Load / Set Projection Matrix
		//
		bool LoadBuiltinProjectionMatrix();
		bool SetCustomProjectionMatrix(const char* file);

		//
		// Calculation : Binary Hash / Checksum
		//
		std::vector<uint8_t> MakeBinaryHash(const std::vector<float>& embedding, int bitsize = 0);
		float MakeChecksum(void);
		std::string MakeChecksumString(void);
		uint64_t MakeSecondHash(void);
		std::string MakeSecondHashString(void);

	public:
		static constexpr int	MAX_MATRIX_BITSIZE		= (1 << 12);
		static constexpr int	MAX_MATRIX_DIMENSION	= (1 << 16);

		//
		// Utilities for Generating Projection Matrix
		//
		static std::vector<float> GenerateProjectionMatrix(int bitsize, int dimension);

		static std::string CreateProjectionMatrixAsCArray(int bitsize, int dimension, const char* name = nullptr, const char* version = nullptr, bool builtin = false);
		static bool CreateProjectionMatrixFileAsCArray(const char* file, int bitsize, int dimension, const char* name = nullptr, const char* version = nullptr, bool builtin = false);
		static bool ExportProjectionMatrixFileAsCArray(const char* file, bool builtin = false);

		//
		// Load Custom Projection Matrix
		//
		static bool LoadCustomProjectionMatrix(const char* file);

		//
		// Calculation : Binary Hash / Checksum
		//
		static std::vector<uint8_t> GetBinaryHash(const std::vector<float>& embedding, int bitsize = 0);
		static float GetChecksum(void);
		static std::string GetChecksumString(void);
		static uint64_t GetSecondHash(void);
		static std::string GetSecondHashString(void);

		//
		// Common Utilitis
		//
		static std::string TrimWithoutComment(const std::string& str);
		static bool GetKeywordValue(const std::string& str, const std::string& keyword, bool trim_quotes, std::string& found);
		static bool ParseAndSetFloatValues(const std::string& str, std::vector<float>& datalist, bool& found_end_block);

		//
		// Method used in the test
		//
		static std::string GetProjectionMatrixName();
		static std::string GetProjectionMatrixVersion();
		static int GetProjectionMatrixBitsize();
		static int GetProjectionMatrixDimension();
		static const std::vector<float>& GetProjectionMatrix();
};

#endif	// K2HVECMATH_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
