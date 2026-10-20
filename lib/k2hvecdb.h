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

#ifndef	K2HVECDB_H
#define	K2HVECDB_H

#include <vector>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <mutex>
#include <atomic>
#include <memory>

#include <k2hash/k2hash.h>
#include <k2hash/k2hshm.h>

//---------------------------------------------------------
// K2hVecDb Class
//---------------------------------------------------------
//
// [NOTE]
// DEFAULT_NEIGHBOR_LIMIT represents the default maximum number
// of candidate binary hashes to retrieve during a search.
// The default value is set to 2600.
// This value defines the maximum cumulative count of generated
// combinations when flipping X bits (XOR transformation) of a
// binary hash bit-string.
// At 2600 candidates, a 10-bit binary hash can be completely
// searched across all possible combinations(full brute-force
// search).
// Depending on the binary hash bit length, the maximum trainable
// bit-flip depth(X) automatically adjusts to remain within this
// limit: allowing up to 4-bit flips for 12-15 bits, 3-bit flips
// for 16-22 bits, and 2-bit flips for 23 bits and above.
//
class K2hVecValue;

class K2hVecDb
{
	protected:
		static constexpr int	DEFAULT_BH_BITSIZE			= 24;
		static constexpr size_t	DEFAULT_NEIGHBOR_LIMIT		= 2600;				// see, NOTE
		static constexpr size_t	MAX_EMBEDDING_DATA_COUNT	= (1ULL << 32);

		static std::mutex									ValLock;
		static std::string									EmbeddingModel;		// Name of the model used to generate the embedding vector data(ex. "text-embedding-3-small")
		static std::shared_ptr<const std::vector<float>>	SysEmbedding;		// System embedding data(float array)
		static std::atomic<float>							ScoreLimit;			// Cosine similarity threshold(ex. 0.90)
		static std::atomic<time_t>							Expire;				// Expiration time for data(in seconds). -1 indicates no expiration.
		static std::string									CryptPass;			// Encrypt passphrase for setting data.
		static std::atomic<int>								BinHashBitSize;		// Binary hash Bit size(count)
		static std::atomic<size_t>							NeighborKeysLimit;	// Upper limit count for Neighbor keys
		static std::atomic<size_t>							DirectLimit;		// Upper limit for saving content text directly to the main key

		K2HShm												LocalK2hobj;		// Local k2hash object
		K2HShm*												pK2hobj;			// External or Local k2hassh object pointer

	protected:
		static std::vector<unsigned char> CvtFloatArrayToByteArray(const float* farray, size_t length);
		static std::vector<float> CvtByteArrayToFloatArray(const unsigned char* bytes, size_t length);

		bool Load(K2hVecValue& value, const std::string& strKey, bool checkattr = true, const std::string* pstrpass = nullptr, bool force_content_load = false) const;
		bool Search(K2hVecValue& value, const std::vector<float>& embedding, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass, bool force_content_load = false) const;

	public:
		static constexpr float			USE_COMMON_SCORE_LIMIT	= 0.0f;				// [NOTE] this means flag for ussing class variable
		static constexpr float			DEFAULT_SCORE_LIMIT		= 0.90f;
		static constexpr time_t			DEFAULT_NOTSET_EXPIRE	= -1;
		static const char*				DEFAULT_EMBEDDING_MODEL;
		static constexpr size_t			CONTENT_ONLY_DIRECT		= std::numeric_limits<size_t>::max();

		// Class Methods
		static bool SetScoreLimit(float val);
		static void ResetScoreLimit(void);
		static float GetScoreLimit(void);

		static bool SetEmbeddingModel(const std::string& model);
		static void SetEmbeddingModel(const char* pmodel);
		static void ResetEmbeddingModel(void);
		static std::string GetEmbeddingModel(void);

		static void SetSysEmbedding(const std::vector<float>& embedding);
		static bool SetSysEmbedding(const float* pembedding, size_t length);
		static std::shared_ptr<const std::vector<float>> GetSysEmbedding(void);
		static void ResetSysEmbedding(void);

		static time_t SetExpire(time_t val);
		static void ResetExpire(void);
		static time_t GetExpire(void);												// if returns K2hVecDb::DEFAULT_NOTSET_EXPIRE, it means not set expire.

		static std::string SetCryptPass(const std::string& pass);
		static void SetCryptPass(const char* ppass);
		static void ResetCryptPass(void);
		static std::string GetCryptPass(void);

		static bool SetBinHashSize(int bitsize);
		static void ResetBinHashBitSize(void);
		static int GetBinHashSize(void);

		static bool SetNeighborKeysLimit(size_t limit);
		static void ResetNeighborKeysLimit(void);
		static size_t GetNeighborKeysLimit(void);

		static size_t SetContentDirectLimit(size_t limit);
		static void ResetContentDirectLimit(void);
		static size_t GetContentDirectLimit(void);

		static bool GetK2hashKey(std::string& k2hkey, const std::vector<float>& embedding, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr);

		// Methods
		K2hVecDb(void);
		explicit K2hVecDb(K2HShm* pk2h);
		virtual ~K2hVecDb(void);

		// K2hash object - Initialize / Clean
		bool Clean(void);
		bool IsAttached(void) const;

		bool SetK2hash(K2HShm* pk2h);
		bool CreateK2hash(const char* file, bool isfullmapping = true, int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE);
		bool AttachK2hash(const char* file, bool isReadOnly, bool isCreate = true, bool isTempFile = false, bool isfullmapping = true, int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE);
		bool AttachK2hashMem(int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE) { return AttachK2hash(nullptr, false, true, false, true, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize); }

		// Set/Get k2hash attributes directly (not recommended)
		bool SetCommonAttribute(const bool* is_mtime = nullptr, const bool* is_defenc = nullptr, const char* passfile = nullptr, const bool* is_history = nullptr, const time_t* expire = nullptr);
		bool AddAttrCryptPass(const char* pass, bool is_default_encrypt = false);
		bool CleanCommonAttribute(void);
		std::string GetCommonAttributeInformation() const;

		// Set Embedding key and value
		bool Set(const std::vector<float>& embedding, const std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, const std::string* pstrpass = nullptr, const time_t* pexpire = nullptr);
		bool Set(const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, const char* ppass = nullptr, const time_t* pexpire = nullptr);

		// Get Content text(direct)
		bool Get(const std::string& strKey, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::vector<float>& em, std::string& content, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::vector<float>& em, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::string& strKey, std::string& content, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::vector<float>& embedding, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Get(const std::vector<float>& embedding, std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		char* Get(const float* pembedding, size_t emlength, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, bool checkattr = true, const char* ppass = nullptr) const;

		// Remove Embedding key and index keys
		bool Remove(const std::vector<float>& embedding, bool mainkeyonly = false, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr) const;
		bool Remove(const float* pembedding, size_t emlength, bool mainkeyonly = false, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL) const;
		bool Remove(const std::string& strKey, bool mainkeyonly = false) const;

		// Search Content text (by cosine similarity)
		bool Search(const std::vector<float>& embedding, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Search(const std::vector<float>& embedding, std::string& res_key, std::string& res_content, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		bool Search(const std::vector<float>& embedding, std::string& res_key, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const;
		char* Search(const float* pembedding, size_t emlength, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, bool checkattr = true, const char* ppass = nullptr) const;
};

#endif	// K2HVECDB_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
