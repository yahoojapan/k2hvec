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

#include <cstring>
#include <k2hash/k2hshm.h>

#include "k2hveccommon.h"
#include "k2hvecdb.h"
#include "k2hvecvalue.h"
#include "k2hvecdbg.h"

//---------------------------------------------------------
// Class Variable
//---------------------------------------------------------
const float		K2hVecDb::USE_COMMON_SCORE_LIMIT;
const float		K2hVecDb::DEFAULT_SCORE_LIMIT;
const time_t	K2hVecDb::DEFAULT_NOTSET_EXPIRE;
const char*		K2hVecDb::DEFAULT_EMBEDDING_MODEL	= "unknown_model";
const int		K2hVecDb::DEFAULT_BH_BITSIZE;
const size_t	K2hVecDb::DEFAULT_NEIGHBOR_LIMIT;

std::mutex									K2hVecDb::ValLock;
std::string									K2hVecDb::EmbeddingModel	= K2hVecDb::DEFAULT_EMBEDDING_MODEL;
std::shared_ptr<const std::vector<float>>	K2hVecDb::SysEmbedding		= nullptr;
std::atomic<float>							K2hVecDb::ScoreLimit{K2hVecDb::DEFAULT_SCORE_LIMIT};
std::atomic<time_t>							K2hVecDb::Expire{K2hVecDb::DEFAULT_NOTSET_EXPIRE};
std::string									K2hVecDb::CryptPass;
std::atomic<int>							K2hVecDb::BinHashBitSize{K2hVecDb::DEFAULT_BH_BITSIZE};
std::atomic<size_t>							K2hVecDb::NeighborKeysLimit{K2hVecDb::DEFAULT_NEIGHBOR_LIMIT};
std::atomic<size_t>							K2hVecDb::DirectLimit{K2hVecDb::CONTENT_ONLY_DIRECT};

//---------------------------------------------------------
// Class Methods (for Class variables)
//---------------------------------------------------------
bool K2hVecDb::SetScoreLimit(float val)
{
	if(val <= 0.0f || 1.0f < val){
		ERR_VECPRN("Score Limit(%.10g) must be range: 0.0f < limit <= 1.0f", val);
		return false;
	}
	K2hVecDb::ScoreLimit.store(val);
	return true;
}

void K2hVecDb::ResetScoreLimit(void)
{
	K2hVecDb::ScoreLimit.store(K2hVecDb::DEFAULT_SCORE_LIMIT);
}

float K2hVecDb::GetScoreLimit(void)
{
	return K2hVecDb::ScoreLimit.load();
}

bool K2hVecDb::SetEmbeddingModel(const std::string& model)
{
	if(model.empty()){
		return false;
	}
	std::lock_guard<std::mutex>	lock(ValLock);
	K2hVecDb::EmbeddingModel= model;
	return true;
}

void K2hVecDb::SetEmbeddingModel(const char* pmodel)
{
	if(VECEMPTYSTR(pmodel)){
		ResetEmbeddingModel();
	}else{
		SetEmbeddingModel(std::string(pmodel));
	}
}

void K2hVecDb::ResetEmbeddingModel(void)
{
	std::lock_guard<std::mutex>	lock(ValLock);
	K2hVecDb::EmbeddingModel = K2hVecDb::DEFAULT_EMBEDDING_MODEL;
}

std::string K2hVecDb::GetEmbeddingModel(void)
{
	std::lock_guard<std::mutex>	lock(ValLock);
	return K2hVecDb::EmbeddingModel;
}

void K2hVecDb::SetSysEmbedding(const std::vector<float>& embedding)
{
	auto newPtr = std::make_shared<const std::vector<float>>(embedding);
	std::atomic_store(&K2hVecDb::SysEmbedding, newPtr);
}

bool K2hVecDb::SetSysEmbedding(const float* pembedding, size_t length)
{
	if(pembedding && 0 < length && length <= K2hVecDb::MAX_EMBEDDING_DATA_COUNT){
		auto newPtr = std::make_shared<const std::vector<float>>(pembedding, pembedding + length);
		std::atomic_store(&K2hVecDb::SysEmbedding, newPtr);
	}else if(!pembedding && 0 == length){
		ResetSysEmbedding();
	}else{
		ERR_VECPRN("parameters are wrong.");
		return false;
	}
	return true;
}

std::shared_ptr<const std::vector<float>> K2hVecDb::GetSysEmbedding(void)
{
	return std::atomic_load(&K2hVecDb::SysEmbedding);
}

void K2hVecDb::ResetSysEmbedding(void)
{
	std::shared_ptr<const std::vector<float>>	nullPtr = nullptr;
    std::atomic_store(&K2hVecDb::SysEmbedding, nullPtr);
}

time_t K2hVecDb::SetExpire(time_t val)
{
	time_t	old = K2hVecDb::Expire;
	K2hVecDb::Expire.store(val);
	return old;
}

void K2hVecDb::ResetExpire(void)
{
	K2hVecDb::Expire.store(K2hVecDb::DEFAULT_NOTSET_EXPIRE);
}

time_t K2hVecDb::GetExpire(void)
{
	return K2hVecDb::Expire.load();
}

std::string K2hVecDb::SetCryptPass(const std::string& pass)
{
	std::lock_guard<std::mutex>	lock(ValLock);
	std::string	old = K2hVecDb::CryptPass;
	K2hVecDb::CryptPass= pass;
	return old;
}

void K2hVecDb::SetCryptPass(const char* ppass)
{
	if(VECEMPTYSTR(ppass)){
		ResetCryptPass();
	}else{
		SetCryptPass(std::string(ppass));
	}
}

void K2hVecDb::ResetCryptPass(void)
{
	std::lock_guard<std::mutex>	lock(ValLock);
	K2hVecDb::CryptPass.clear();
}

std::string K2hVecDb::GetCryptPass(void)
{
	std::lock_guard<std::mutex>	lock(ValLock);
	return K2hVecDb::CryptPass;
}

bool K2hVecDb::SetBinHashSize(int bitsize)
{
	if(bitsize <= 0){
		return false;
	}
	K2hVecDb::BinHashBitSize.store(bitsize);
	return true;
}

void K2hVecDb::ResetBinHashBitSize(void)
{
	K2hVecDb::BinHashBitSize.store(K2hVecDb::DEFAULT_BH_BITSIZE);
}

int K2hVecDb::GetBinHashSize(void)
{
	return K2hVecDb::BinHashBitSize.load();
}

bool K2hVecDb::SetNeighborKeysLimit(size_t limit)
{
	if(limit == 0){
		return false;
	}
	K2hVecDb::NeighborKeysLimit.store(limit);
	return true;
}

void K2hVecDb::ResetNeighborKeysLimit(void)
{
	K2hVecDb::NeighborKeysLimit.store(K2hVecDb::DEFAULT_NEIGHBOR_LIMIT);
}

size_t K2hVecDb::GetNeighborKeysLimit(void)
{
	return K2hVecDb::NeighborKeysLimit.load();
}

size_t K2hVecDb::SetContentDirectLimit(size_t limit)
{
	size_t	old = K2hVecDb::DirectLimit;
	K2hVecDb::DirectLimit.store(limit);
	return old;
}

void K2hVecDb::ResetContentDirectLimit(void)
{
	K2hVecDb::DirectLimit.store(K2hVecDb::CONTENT_ONLY_DIRECT);
}

size_t K2hVecDb::GetContentDirectLimit(void)
{
	return K2hVecDb::DirectLimit.load();
}

//---------------------------------------------------------
// Class Methods (Common)
//---------------------------------------------------------
//
// Convert float array to byte array(K2hash key)
//
std::vector<unsigned char> K2hVecDb::CvtFloatArrayToByteArray(const float* farray, size_t length)
{
	if(!farray || 0 == length){
		return {};
	}

	// Initialize byte array
	std::vector<unsigned char>	result(length * sizeof(float));		// float = 4 byte

	for(size_t pos = 0; pos < length; ++pos){
		// [NOTE]
		// Copy the bit pattern of a float variable directly into a uint32_t.
		//
		// Copying the bit pattern of a float variable directly to a uint32_t,
		// an implicit conversion from float to uint32_t will result in the
		// loss of the bit pattern,
		//
		uint32_t	floatbits;
		std::memcpy(&floatbits, &farray[pos], sizeof(float));

		// Convert host endian to big endian(network byte order)
		uint32_t	cvtbits = htobe32(floatbits);

		// Set directly to byte buffers
		std::memcpy(&result[pos * sizeof(float)], &cvtbits, sizeof(uint32_t));
	}
	return result;
}

//
// Convert byte array(K2hash key) to float array
//
std::vector<float> K2hVecDb::CvtByteArrayToFloatArray(const unsigned char* bytes, size_t length)
{
	if(!bytes || 0 == length){
		return {};
	}
	// To convert to float, the length must be a multiple of 4.
	if(0 != (length % sizeof(float))){
		return {};
	}

	// Initialize float array
	size_t				float_length = length / sizeof(float);
	std::vector<float>	result(float_length);

	for(size_t pos = 0; pos < float_length; ++pos){
		// Copy bytes array[4] to uint32_t
		uint32_t	rawbits;
		std::memcpy(&rawbits, &bytes[pos * sizeof(float)], sizeof(uint32_t));

		// Convert big endian(network byte order) to host endian
		uint32_t	floatbits = be32toh(rawbits);

		// Copy the byte[4] to float directly
		std::memcpy(&result[pos], &floatbits, sizeof(float));
	}
	return result;
}

bool K2hVecDb::GetK2hashKey(std::string& k2hkey, const std::vector<float>& embedding, const std::string* pstrmodel, const std::vector<float>* psysembedding)
{
	// Check parameters
	std::string	strModel;
	if(!pstrmodel || pstrmodel->empty()){
		strModel = K2hVecDb::GetEmbeddingModel();	// model must be not empty, so use default(class variable)
	}else{
		strModel = *pstrmodel;
	}

	std::shared_ptr<const std::vector<float>>	SysEmPtr;
	if(!psysembedding){								// nullptr means check by default(class variable)
		SysEmPtr = K2hVecDb::GetSysEmbedding();
		if(SysEmPtr){
			psysembedding = SysEmPtr.get();
		}
	}
	if(psysembedding && psysembedding->empty()){
		psysembedding = nullptr;				   // empty means no-check by system embedding.
	}

	// Make temporary K2hVecValue object and Generate key name
	K2hVecValue	value;
	if(!value.Initialize(embedding, "", strModel, psysembedding)){
		ERR_VECPRN("Cloud not make K2hVecValue object");
		return false;
	}

	// Get key
	if(!value.MakeMainKeyString(k2hkey)){
		ERR_VECPRN("Failed to generate k2hash key.");
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hVecDb::K2hVecDb(void) : LocalK2hobj(), pK2hobj(nullptr)
{
	MSG_VECPRN("K2HShm object was kept empty.");
}

K2hVecDb::K2hVecDb(K2HShm* pk2h) : LocalK2hobj(), pK2hobj(pk2h)
{
	if(!pk2h){
		ERR_VECPRN("pk2h parameter is NULL.");
	}else if(!pk2h->IsAttached()){
		ERR_VECPRN("K2HShm(pk2h) does not attached, so set pK2hobj to NULL.");
		pK2hobj = nullptr;
	}else{
		MSG_VECPRN("K2HShm object was provided via the constructor.");
	}
}

K2hVecDb::~K2hVecDb(void)
{
	Clean();
}

//---------------------------------------------------------
// Methods - K2hash object
//---------------------------------------------------------
bool K2hVecDb::Clean(void)
{
	if(pK2hobj){
		pK2hobj = nullptr;
	}
	if(LocalK2hobj.IsAttached()){
		if(!LocalK2hobj.Detach()){
			ERR_VECPRN("Local K2HShm object(LocalK2hobj) is attached, but failed to detach.");
			return false;
		}
	}
	return true;
}

bool K2hVecDb::IsAttached(void) const
{
	if(!pK2hobj){
		return false;
	}
	if(!pK2hobj->IsAttached()){
		return false;
	}
	return true;
}

bool K2hVecDb::SetK2hash(K2HShm* pk2h)
{
	if(nullptr == pk2h){
		ERR_VECPRN("pk2h is NULL.");
		return false;
	}
	if(!pk2h->IsAttached()){
		ERR_VECPRN("Specified k2hash object(pk2h) is not attached.");
		return false;
	}
	if(IsAttached()){
		MSG_VECPRN("Already K2HShm object(pk2h) is attached, so detach it.");
		if(!Clean()){
			return false;
		}
	}

	// Set k2hash object pointer
	pK2hobj = pk2h;

	return true;
}

bool K2hVecDb::CreateK2hash(const char* file, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
{
	if(!Clean()){
		return false;
	}

	// Create k2hash data in this object.
	if(!LocalK2hobj.Create(file, isfullmapping, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize)){
		ERR_VECPRN("Failed to create k2hash file/memory for this object.");

		// try to recover
		if(!Clean()){
			ERR_VECPRN("Failed to clean for recovering after failure of k2hash file/memory creation.");
		}
		return false;
	}

	// Set local k2hash object pointer
	pK2hobj = &LocalK2hobj;

	return true;
}

bool K2hVecDb::AttachK2hash(const char* file, bool isReadOnly, bool isCreate, bool isTempFile, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
{
	if(!Clean()){
		return false;
	}

	// Attach k2hash data in this object.
	if(!LocalK2hobj.Attach(file, isReadOnly, isCreate, isTempFile, isfullmapping, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize)){
		ERR_VECPRN("Failed to attach k2hash file/memory for this object.");

		// try to recover
		if(!Clean()){
			ERR_VECPRN("Failed to clean for recovering after failure of k2hash file/memory attaching.");
		}
		return false;
	}

	// Set local k2hash object pointer
	pK2hobj = &LocalK2hobj;

	return true;
}

//---------------------------------------------------------
// Methods - Set/Get attributes for K2hash
//---------------------------------------------------------
// [NOTE]
// These methods directly set and read values in the k2hash object. Use of these functions
// is not recommended.
//
bool K2hVecDb::SetCommonAttribute(const bool* is_mtime, const bool* is_defenc, const char* passfile, const bool* is_history, const time_t* expire)
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}
	return pK2hobj->SetCommonAttribute(is_mtime, is_defenc, passfile, is_history, expire, nullptr);
}

bool K2hVecDb::AddAttrCryptPass(const char* pass, bool is_default_encrypt)
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}
	return pK2hobj->AddAttrCryptPass(pass, is_default_encrypt);
}

bool K2hVecDb::CleanCommonAttribute(void)
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}
	return pK2hobj->CleanCommonAttribute();
}

std::string K2hVecDb::GetCommonAttributeInformation() const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return "";
	}
	std::stringstream	ss;
	pK2hobj->GetAttrInfos(ss);
	return ss.str();
}

//---------------------------------------------------------
// Methods - Set
//---------------------------------------------------------
//
// Set values to K2hash
//
bool K2hVecDb::Set(const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel, const float* psysembedding, size_t sysemlength, const char* ppass, const time_t* pexpire)
{
	// Check required parameters
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0 || K2hVecDb::MAX_EMBEDDING_DATA_COUNT < emlength){
		ERR_VECPRN("Empty or overflow embedding data is specified.");
		return false;
	}
	Embedding.assign(pembedding, pembedding + emlength);

	if(VECEMPTYSTR(pcontent)){
		ERR_VECPRN("Empty content text data is specified.");
		return false;
	}
	std::string	strContent(pcontent);

	// Check parameters
	std::string			strmodel;
	const std::string*	pstrmodel = nullptr;
	if(!VECEMPTYSTR(pemmodel)){
		strmodel	= pemmodel;
		pstrmodel	= &strmodel;
	}

	std::vector<float>			SysEmbeddingVec;
	const std::vector<float>*	pSysEmbeddingVec = nullptr;
	if(psysembedding){
		if(0 < sysemlength){
			SysEmbeddingVec.assign(psysembedding, psysembedding + sysemlength);
		}
		pSysEmbeddingVec = &SysEmbeddingVec;			// if length is 0, set empty array to pointer.
	}

	std::string			strPass;
	const std::string*	pstrPass = nullptr;
	if(ppass){
		if(!VECEMPTYSTR(ppass)){
			strPass		= ppass;
			pstrPass	= &strPass;						// if string length is 0, set empty string.
		}
	}

	// Set
	return Set(Embedding, strContent, pstrmodel, pSysEmbeddingVec, pstrPass, pexpire);
}

bool K2hVecDb::Set(const std::vector<float>& embedding, const std::string& content, const std::string* pstrmodel, const std::vector<float>* psysembedding, const std::string* pstrpass, const time_t* pexpire)
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Check required parameters
	if(embedding.empty()){
		ERR_VECPRN("Empty embedding data is specified.");
		return false;
	}

	// Check parameters
	std::string	tmpEmbeddingModel;
	if(!pstrmodel || pstrmodel->empty()){
		tmpEmbeddingModel	= K2hVecDb::GetEmbeddingModel();
		pstrmodel			= &tmpEmbeddingModel;	// model must be not empty, so use default(class variable)
	}

	std::shared_ptr<const std::vector<float>>	SysEmPtr;
	if(!psysembedding){								// nullptr means check by default(class variable)
		SysEmPtr = K2hVecDb::GetSysEmbedding();
		if(SysEmPtr){
			psysembedding = SysEmPtr.get();
		}
	}
	if(psysembedding && psysembedding->empty()){
		psysembedding = nullptr;				   // empty means no-check by system embedding.
	}

	std::string	tmpCryptPass;
	if(!pstrpass){
		if(!K2hVecDb::GetCryptPass().empty()){
			tmpCryptPass= K2hVecDb::GetCryptPass();
			pstrpass	= &tmpCryptPass;			// nullptr means using default crypt pass(class variable)
		}
	}

	const time_t*	ptmpexpire = nullptr;
	time_t			tmpexpire;
	if(!pexpire){
		if(0 < K2hVecDb::GetExpire()){
			tmpexpire 	= K2hVecDb::GetExpire();	// nullptr means using default expire(class variable). but if default is negative value, not set expire.
			ptmpexpire	= &tmpexpire;
		}
	}else{
		ptmpexpire = pexpire;
	}

	// Build value object
	K2hVecValue	vecvalue(embedding, content, *pstrmodel, psysembedding);
	if(vecvalue.empty()){
		ERR_VECPRN("Failed to construct the values to be set in K2Hash.");
		return false;
	}

	// Set values to k2hash
	if(!vecvalue.Set(pK2hobj, pstrpass, ptmpexpire)){
		ERR_VECPRN("Failed to set values to k2hash K2Hash.");
		return false;
	}

	return true;
}

//---------------------------------------------------------
// Get direct
//---------------------------------------------------------
//
// Load values as K2hVecValue
//
bool K2hVecDb::Load(K2hVecValue& value, const std::string& strKey, bool checkattr, const std::string* pstrpass, bool force_content_load) const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Parameters
	std::string	tmpCryptPass;
	if(!pstrpass){
		if(!K2hVecDb::GetCryptPass().empty()){
			tmpCryptPass= K2hVecDb::GetCryptPass();
			pstrpass	= &tmpCryptPass;				// nullptr means using default crypt pass(class variable)
		}
	}

	// Load
	if(!value.Load(pK2hobj, strKey, checkattr, pstrpass, force_content_load)){
		return false;
	}
	return true;
}

bool K2hVecDb::Get(const std::string& strKey, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr, const std::string* pstrpass) const
{
	// Load
	K2hVecValue	value;
	if(!Load(value, strKey, checkattr, pstrpass, true)){
		return false;
	}
	// Get
	if(	!value.MoveEmbedding(em)		||
		!value.MoveSysEmbedding(sysem)	||
		!value.MoveContentText(content)	||
		!value.MoveEmbeddingModel(model))
	{
		return false;
	}
	return true;
}

bool K2hVecDb::Get(const std::string& strKey, std::vector<float>& em, std::string& content, bool checkattr, const std::string* pstrpass) const
{
	// Load
	K2hVecValue	value;
	if(!Load(value, strKey, checkattr, pstrpass, true)){
		return false;
	}
	// Get
	if(!value.MoveEmbedding(em) || !value.MoveContentText(content)){
		return false;
	}
	return true;
}

bool K2hVecDb::Get(const std::string& strKey, std::vector<float>& em, bool checkattr, const std::string* pstrpass) const
{
	// Load
	K2hVecValue	value;
	if(!Load(value, strKey, checkattr, pstrpass, false)){
		return false;
	}
	// Get
	if(!value.MoveEmbedding(em)){
		return false;
	}
	return true;
}

bool K2hVecDb::Get(const std::string& strKey, std::string& content, bool checkattr, const std::string* pstrpass) const
{
	// Load
	K2hVecValue	value;
	if(!Load(value, strKey, checkattr, pstrpass, true)){
		return false;
	}
	//Get
	if(!value.MoveContentText(content)){
		return false;
	}
	return true;
}

bool K2hVecDb::Get(const std::vector<float>& embedding, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	// Make key
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, embedding, pstrmodel, psysembedding)){
		return false;
	}
	// Get all
	return Get(k2hkey.c_str(), em, sysem, content, model, checkattr, pstrpass);
}

bool K2hVecDb::Get(const std::vector<float>& embedding, std::string& content, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	// Get key
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, embedding, pstrmodel, psysembedding)){
		return false;
	}
	// Get content
	return Get(k2hkey.c_str(), content, checkattr, pstrpass);
}

char* K2hVecDb::Get(const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass) const
{
	// Check required parameters
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0 || K2hVecDb::MAX_EMBEDDING_DATA_COUNT < emlength){
		ERR_VECPRN("Empty or overflow embedding data is specified.");
		return nullptr;
	}
	Embedding.assign(pembedding, pembedding + emlength);

	std::string			strmodel;
	const std::string*	pstrmodel = nullptr;
	if(!VECEMPTYSTR(pemmodel)){
		strmodel	= pemmodel;
		pstrmodel	= &strmodel;
	}

	std::vector<float>			SysEmbeddingVec;
	const std::vector<float>*	pSysEmbeddingVec = nullptr;
	if(psysembedding){
		if(0 < sysemlength){
			SysEmbeddingVec.assign(psysembedding, psysembedding + sysemlength);
		}
		pSysEmbeddingVec = &SysEmbeddingVec;			// if length is 0, set empty array to pointer.
	}

	std::string			strpass;
	const std::string*	pstrpass = nullptr;
	if(!VECEMPTYSTR(ppass)){
		strpass		= ppass;
		pstrpass	= &strpass;
	}

	// Get content
	std::string	content;
	if(!Get(Embedding, content, pstrmodel, pSysEmbeddingVec, checkattr, pstrpass)){
		return nullptr;
	}
	return strdup(content.c_str());
}

//---------------------------------------------------------
// Remove
//---------------------------------------------------------
//
// Remove main key and index keys in K2Hash.
//
bool K2hVecDb::Remove(const float* pembedding, size_t emlength, bool mainkeyonly, const char* pemmodel, const float* psysembedding, size_t sysemlength) const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Check required parameters
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0 || K2hVecDb::MAX_EMBEDDING_DATA_COUNT < emlength){
		ERR_VECPRN("Empty or overflow embedding data is specified.");
		return false;
	}
	Embedding.assign(pembedding, pembedding + emlength);

	std::string			strmodel;
	const std::string*	pstrmodel = nullptr;
	if(!VECEMPTYSTR(pemmodel)){
		strmodel	= pemmodel;
		pstrmodel	= &strmodel;
	}

	std::vector<float>			SysEmbeddingVec;
	const std::vector<float>*	pSysEmbeddingVec = nullptr;
	if(psysembedding){
		if(0 < sysemlength){
			SysEmbeddingVec.assign(psysembedding, psysembedding + sysemlength);
		}
		pSysEmbeddingVec = &SysEmbeddingVec;			// if length is 0, set empty array to pointer.
	}

	// Remove key
	return Remove(Embedding, mainkeyonly, pstrmodel, pSysEmbeddingVec);
}

bool K2hVecDb::Remove(const std::vector<float>& embedding, bool mainkeyonly, const std::string* pstrmodel, const std::vector<float>* psysembedding) const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Get main key
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, embedding, pstrmodel, psysembedding)){
		return false;
	}

	// Remove key
	return Remove(k2hkey.c_str(), mainkeyonly);
}

bool K2hVecDb::Remove(const std::string& strKey, bool mainkeyonly) const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Remove keys
	if(!K2hVecValue::Remove(pK2hobj, strKey, mainkeyonly)){
		return false;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Vector Search
//---------------------------------------------------------
bool K2hVecDb::Search(K2hVecValue& value, const std::vector<float>& embedding, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass, bool force_content_load) const
{
	if(!IsAttached()){
		ERR_VECPRN("No attached k2hash object.");
		return false;
	}

	// Check parameters
	if(K2hVecDb::USE_COMMON_SCORE_LIMIT == score){
		score = K2hVecDb::GetScoreLimit();			// set default score limit(class variable)
	}

	std::string	tmpEmbeddingModel;
	if(!pstrmodel || pstrmodel->empty()){
		tmpEmbeddingModel	= K2hVecDb::GetEmbeddingModel();
		pstrmodel			= &tmpEmbeddingModel;	// model must be not empty, so use default(class variable)
	}

	std::shared_ptr<const std::vector<float>>	SysEmPtr;
	if(!psysembedding){								// nullptr means check by default(class variable)
		SysEmPtr = K2hVecDb::GetSysEmbedding();
		if(SysEmPtr){
			psysembedding = SysEmPtr.get();
		}
	}
	if(psysembedding && psysembedding->empty()){
		psysembedding = nullptr;				   // empty means no-check by system embedding.
	}

	std::string	tmpCryptPass;
	if(!pstrpass){
		tmpCryptPass= K2hVecDb::GetCryptPass();
		pstrpass	= &tmpCryptPass;				// nullptr means using default crypt pass(class variable)
	}else if(pstrpass->empty()){
		pstrpass = nullptr;							// empty means not crypt data
	}

	// Search
	if(!value.Search(pK2hobj, embedding, score, *pstrmodel, psysembedding, checkattr, (pstrpass ? pstrpass->c_str() : nullptr), force_content_load)){
		MSG_VECPRN("No cosine similarity for the embedding was found.");
		return false;
	}
	return true;
}

bool K2hVecDb::Search(const std::vector<float>& embedding, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	// Search
	K2hVecValue	value;
	if(!Search(value, embedding, score, pstrmodel, psysembedding, checkattr, pstrpass, true)){
		return false;
	}

	// Get
	if(	!value.MakeMainKeyString(res_key)	||
		!value.MoveEmbedding(res_em)		||
		!value.MoveSysEmbedding(res_sysem)	||
		!value.MoveContentText(res_content)	||
		!value.MoveEmbeddingModel(res_model))
	{
		ERR_VECPRN("Could not retrieve the values of the detected data(based on cosine similarity).");
		return false;
	}
	return true;
}

bool K2hVecDb::Search(const std::vector<float>& embedding, std::string& res_key, std::string& res_content, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	// Search
	K2hVecValue	value;
	if(!Search(value, embedding, score, pstrmodel, psysembedding, checkattr, pstrpass, true)){
		return false;
	}

	// Get content text
	if(!value.MakeMainKeyString(res_key) || !value.MoveContentText(res_content)){
		ERR_VECPRN("Could not retrieve the text content of the detected data (based on cosine similarity).");
		return false;
	}
	return true;
}

bool K2hVecDb::Search(const std::vector<float>& embedding, std::string& res_key, float score, const std::string* pstrmodel, const std::vector<float>* psysembedding, bool checkattr, const std::string* pstrpass) const
{
	// Search
	K2hVecValue	value;
	if(!Search(value, embedding, score, pstrmodel, psysembedding, checkattr, pstrpass, false)){
		return false;
	}

	// Get
	if(!value.MakeMainKeyString(res_key)){
		ERR_VECPRN("Could not make mainkey of the detected data(based on cosine similarity).");
		return false;
	}
	return true;
}

char* K2hVecDb::Search(const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass) const
{
	// Check required parameters
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0 || K2hVecDb::MAX_EMBEDDING_DATA_COUNT < emlength){
		ERR_VECPRN("Empty or overflow embedding data is specified.");
		return nullptr;
	}
	Embedding.assign(pembedding, pembedding + emlength);

	// Check parameters
	std::string			strmodel;
	const std::string*	pstrmodel = nullptr;
	if(!VECEMPTYSTR(pemmodel)){
		strmodel	= pemmodel;
		pstrmodel	= &strmodel;
	}

	std::vector<float>			SysEmbeddingVec;
	const std::vector<float>*	pSysEmbeddingVec = nullptr;
	if(psysembedding){
		if(0 < sysemlength){
			SysEmbeddingVec.assign(psysembedding, psysembedding + sysemlength);
		}
		pSysEmbeddingVec = &SysEmbeddingVec;			// if length is 0, set empty array to pointer.
	}

	std::string			strPass;
	const std::string*	pstrPass = nullptr;
	if(ppass){
		if(!VECEMPTYSTR(ppass)){
			strPass		= ppass;
		}
		pstrPass	= &strPass;							// if string length is 0, set empty string.
	}

	// Search content text
	std::string	content;
	if(!Search(Embedding, content, score, pstrmodel, pSysEmbeddingVec, checkattr, pstrPass)){
		return nullptr;
	}
	return strdup(content.c_str());
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
