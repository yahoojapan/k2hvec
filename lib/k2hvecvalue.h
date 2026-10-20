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

#ifndef	K2HVECVALUE_H
#define	K2HVECVALUE_H

#include <vector>
#include <cstdint>
#include <cstddef>

#include <k2hash/k2hshm.h>

//---------------------------------------------------------
// K2hVecValue Class
//---------------------------------------------------------
class K2hVecValue
{
	protected:
		// Content Text placement type
		enum class ContentValueType{
			DIRECT,															// The content text is saved in main key
			INDIRECT														// The content text is saved in another key(subkey)
		};

		// Value load and set state
		enum class ValueState{
			UNSET,															// Value has not been set yet.
			UNLOADED_CONTENT_SET,											// Value info is set, but content text is not loaded(indirect).
			FULLY_SET														// Value is fully set including content text.
		};

		static constexpr const char*	INDIRECT_CONTENTS_KEY_SUFFIX	= "-CONTENTS";
		static constexpr const char*	INDIRECT_TYPE_CONTENTS			= "\x01_INDIRECT_CONTENTS_\x01";				// "^A...^A"
		static constexpr size_t			INDIRECT_TYPE_CONTENTS_LENGTH	= sizeof("\x01_INDIRECT_CONTENTS_\x01") - 1;	// = strlen(INDIRECT_TYPE_CONTENTS)

		std::vector<float>	Embedding;										// embedding for user prompt
		std::vector<float>	SysEmbedding;									// embedding for system prompt
		std::string			ContentText;									// response string from LLM
		std::string			EmbeddingModel;									// embedding model string
		ValueState			ValueStatus			= ValueState::UNSET;		// Indicates the current set/load state of the value(UNSET, UNLOADED_CONTENT_SET, FULLY_SET)
		ContentValueType	ContentType			= ContentValueType::DIRECT;	// Value type for main key

	protected:
		static uint64_t GetXorCombinationCount(size_t total_bit_cnt, size_t r_bit_cnt);
		static uint64_t GetTotalXorCombinationCount(size_t total_bit_cnt, size_t r_bit_cnt);
		static std::vector<std::string> GetXorBinaryHashStrings(const std::string& strBinaryHash, const std::string& strPrefix, size_t r_bit_cnt);
		static bool RawMakeKeyStrings(const std::vector<float>& embedding, std::string* pmainkey, size_t limitcount, std::vector<std::vector<std::string>>* pneighborkeys);
		static std::string MakeIndirectContentKeyString(const std::string& mainkey);
		static bool RemoveNeighborKey(K2HShm* pK2hobj, const std::string& key);

		bool Initialize(unsigned char* pbytes, size_t length);
		bool LoadAndMatch(K2HShm* pK2hobj, const std::string& key, const std::vector<float>& em, float score, float& cosval, const std::string& model, const std::vector<float>* pSysEmbedding = nullptr, bool checkattr = true, const char* encpass = nullptr);

	public:
		static size_t GetLimitXorBit(size_t total_bit_cnt, size_t limit_cnt);
		static std::vector<std::vector<std::string>> GetTotalXorBinaryHashStrings(const std::string& strBinaryHash, const std::string& strPrefix, size_t r_bit_cnt);
		static bool DeriveNeighborKeys(const std::string& strMainkey, std::vector<std::vector<std::string>>& NeighborKeys);
		static bool DeriveMainNeighborKey(const std::string& strMainkey, std::string& strNeighborKey);
		static bool Remove(K2HShm* pK2hobj, const std::string& key, bool mainkeyonly = true);

		K2hVecValue();
		K2hVecValue(const std::vector<float>& em, const std::string& content, const std::string& model, const std::vector<float>* psysem = nullptr);
		~K2hVecValue();

		// Initialize/etc
		bool empty(void) const;
		void clear(void);
		bool Initialize(const std::vector<float>& em, const std::string& content, const std::string& model, const std::vector<float>* psysem = nullptr);

		// Operation K2Hash : Set/Get/Search
		bool Set(K2HShm* pK2hobj, const std::string* pPass = nullptr, const time_t* pexpire = nullptr);
		bool Load(const K2HShm* pK2hobj, const std::string& key, bool checkattr = true, const std::string* pPass = nullptr, bool force_content_load = false);
		bool LoadContentText(const K2HShm* pK2hobj, bool checkattr = true, const char* encpass = nullptr);
		bool Get(const K2HShm* pK2hobj, const std::string& key, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr = true, const std::string* pPass = nullptr);
		bool Search(K2HShm* pK2hobj, const std::vector<float>& em, float score, const std::string& model, const std::vector<float>* pSysEmbedding = nullptr, bool checkattr = true, const char* encpass = nullptr, bool force_content_load = true);

		// K2Hash Attributes(Key name)
		bool MakeMainKeyStrings(std::string& mainkey, std::string& neighborkey) const;
		bool MakeMainKeyString(std::string& mainkey) const;
		bool MakeNeighborKeys(std::vector<std::vector<std::string>>& neighborkeys) const;

		// Get data information (use internally)
		size_t GetEmbeddingLength(void) const;
		size_t GetSysEmbeddingLength(void) const;
		size_t GetContentTextInMainKeyLength(void) const;
		size_t GetEmbeddingModelLength(void) const;

		// Get member data
		bool GetEmbedding(std::vector<float>& em) const;
		bool GetSysEmbedding(std::vector<float>& sysem) const;
		bool GetContentText(std::string& content, const K2HShm* pK2hobj = nullptr, bool checkattr = true, const char* encpass = nullptr);
		bool GetEmbeddingModel(std::string& model) const;

		// Get member data referance
		//
		// [NOTE]
		// this method does not check whether the data is actually held.
		//
		const std::vector<float>& GetEmbedding() const { return Embedding; }
		const std::vector<float>& GetSysEmbedding() const { return SysEmbedding; }
		const std::string& GetContentText() const { return ContentText; }
		const std::string& GetEmbeddingModel() const { return EmbeddingModel; }

		// Copy data(used only by the internal RawK2hVecValue)
		void CopyEmbedding(float* dest, size_t length) const;
		void CopySysEmbedding(float* dest, size_t length) const;
		void CopyContentTextInMainKey(char* dest, size_t length) const;
		void CopyEmbeddingModel(char* dest, size_t length) const;

		// Retrieve content text(NOTE: clear internal datas of object)
		bool MoveEmbedding(std::vector<float>& em);
		bool MoveSysEmbedding(std::vector<float>& sysem);
		bool MoveContentText(std::string& content);
		bool MoveEmbeddingModel(std::string& model);
};

#endif	// K2HVECVALUE_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
