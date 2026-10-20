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
#include <cstdlib>

#include "k2hveccommon.h"
#include "k2hvec.h"
#include "k2hvecdb.h"
#include "k2hvecutil.h"
#include "k2hvecdbg.h"

//---------------------------------------------------------
// Aliases
//---------------------------------------------------------
//
// Internal binding for the opaque handle.
//
// By defining "struct k2hvec_handle" as a derived class of K2hVecDb in C++,
// the handle can be directly converted and used as a K2hVecDb pointer
// without requiring explicit 'reinterpret_cast' operations.
//
struct k2hvec_handle : public K2hVecDb {};

//---------------------------------------------------------
// Functions - Debug
//---------------------------------------------------------
void k2hvec_bump_debug_level(void)
{
	::BumpupK2hvecDbgMode();
}

void k2hvec_set_debug_level_silent(void)
{
	::SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_SILENT);
}

void k2hvec_set_debug_level_error(void)
{
	::SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_ERR);
}

void k2hvec_set_debug_level_warning(void)
{
	::SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_WARN);
}

void k2hvec_set_debug_level_message(void)
{
	::SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_MSG);
}

void k2hvec_set_debug_level_dump(void)
{
	::SetK2hvecDbgMode(K2hvecDbgMode::VECDBG_DUMP);
}

bool k2hvec_set_debug_file(const char* filepath)
{
	bool result;
	if(VECEMPTYSTR(filepath)){
		result = ::UnsetK2hvecDbgFile();
	}else{
		result = ::SetK2hvecDbgFile(filepath);
	}
	return result;
}

bool k2hvec_unset_debug_file(void)
{
	return ::UnsetK2hvecDbgFile();
}

bool k2hvec_load_debug_env(void)
{
	return ::LoadK2hvecDbgEnv();
}

//---------------------------------------------------------
// Functions - Common default variables
//---------------------------------------------------------
//
// Utility
//
static float* DupFloatArray(const std::vector<float>& farray, size_t& length)
{
	if(farray.empty()){
		length = 0;
		return nullptr;
	}
	length			= farray.size();
	size_t	bytes	= length * sizeof(float);
    float*	pFloat	= static_cast<float*>(std::malloc(bytes));
	if(!pFloat){
		ERR_VECPRN("Fatal error : Coult not allocate memory.");
		length = 0;
		return nullptr;
	}
	std::memcpy(pFloat, farray.data(), bytes);

	return pFloat;
}

//
// Set/Get Common variables
//
bool k2hvec_set_default_score_limit(float val)
{
	return K2hVecDb::SetScoreLimit(val);
}

void k2hvec_reset_default_score_limit(void)
{
	K2hVecDb::ResetScoreLimit();
}

float k2hvec_get_default_score_limit(void)
{
	return K2hVecDb::GetScoreLimit();
}

bool k2hvec_set_default_embedding_model(const char* pmodel)
{
	if(VECEMPTYSTR(pmodel)){
		ERR_VECPRN("pmodel parameter is nullptr.");
		return false;
	}
	return K2hVecDb::SetEmbeddingModel(std::string(pmodel));
}

void k2hvec_reset_default_embedding_model(void)
{
	K2hVecDb::ResetEmbeddingModel();
}

char* k2hvec_get_default_embedding_model(void)
{
	std::string	strEmbeddingModel = K2hVecDb::GetEmbeddingModel();
	if(strEmbeddingModel.empty()){
		return nullptr;
	}
	return strdup(strEmbeddingModel.c_str());
}

bool k2hvec_set_default_system_embedding(const float* pembedding, size_t length)
{
	return K2hVecDb::SetSysEmbedding(pembedding, length);
}

void k2hvec_reset_default_system_embedding(void)
{
	K2hVecDb::ResetSysEmbedding();
}

float* k2hvec_get_default_system_embedding(size_t* plength)
{
	size_t	tmpLength	= 0UL;
	size_t*	ptmpLength;
	if(plength){
		ptmpLength = plength;
	}else{
		ptmpLength = &tmpLength;
	}

	const std::vector<float>*					psysembedding	= nullptr;
	std::shared_ptr<const std::vector<float>>	SysEmPtr		= K2hVecDb::GetSysEmbedding();
	if(SysEmPtr){
		psysembedding = SysEmPtr.get();
	}
	if(!psysembedding){
		return nullptr;
	}
	return DupFloatArray(*psysembedding, *ptmpLength);
}

time_t k2hvec_set_default_expire(time_t expire)
{
	return K2hVecDb::SetExpire(expire);
}

void k2hvec_reset_default_expire(void)
{
	return K2hVecDb::ResetExpire();
}

time_t k2hvec_get_default_expire()
{
	return K2hVecDb::GetExpire();
}

void k2hvec_set_default_passphrase(const char* ppass)
{
	K2hVecDb::SetCryptPass(ppass);
}

void k2hvec_reset_default_passphrase(void)
{
	K2hVecDb::ResetCryptPass();
}

char* k2hvec_get_default_passphrase()
{
	std::string	strPass = K2hVecDb::GetCryptPass();
	if(strPass.empty()){
		return nullptr;
	}
	return strdup(strPass.c_str());
}

bool k2hvec_set_default_binary_hash_size(int bitsize)
{
	return K2hVecDb::SetBinHashSize(bitsize);
}

void k2hvec_reset_default_binary_hash_size(void)
{
	K2hVecDb::ResetBinHashBitSize();
}

int k2hvec_get_default_binary_hash_size(void)
{
	return K2hVecDb::GetBinHashSize();
}

bool k2hvec_set_default_neighbor_key_limit(size_t limit)
{
	return K2hVecDb::SetNeighborKeysLimit(limit);
}

void k2hvec_reset_default_neighbor_key_limit(void)
{
	K2hVecDb::ResetNeighborKeysLimit();
}

size_t k2hvec_get_default_neighbor_key_limit(void)
{
	return K2hVecDb::GetNeighborKeysLimit();
}

bool k2hvec_set_content_direct_limit(size_t limit)
{
	return K2hVecDb::SetContentDirectLimit(limit);
}

void k2hvec_reset_content_direct_limit(void)
{
	K2hVecDb::ResetContentDirectLimit();
}

size_t k2hvec_get_content_direct_limit(void)
{
	return K2hVecDb::GetContentDirectLimit();
}

//---------------------------------------------------------
// Functions - Initialize
//---------------------------------------------------------
k2hvec_handle_t k2hvec_create(const char* file, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
{
	k2hvec_handle_t	k2hvec = new(std::nothrow) k2hvec_handle();		// as same as K2hVecDb()

	if(!k2hvec->CreateK2hash(file, isfullmapping, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize)){
		ERR_VECPRN("Failed to initialize k2hvec(create new k2hash file).");
		delete k2hvec;
		return nullptr;
	}
	return k2hvec;
}

k2hvec_handle_t k2hvec_attach(const char* file, bool isReadOnly, bool isCreate, bool isTempFile, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
{
	k2hvec_handle_t	k2hvec = new(std::nothrow) k2hvec_handle();		// as same as K2hVecDb()

	if(!k2hvec->AttachK2hash(file, isReadOnly, isCreate, isTempFile, isfullmapping, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize)){
		ERR_VECPRN("Failed to initialize k2hvec(attach k2hash file or memory).");
		delete k2hvec;
		return nullptr;
	}
	return k2hvec;
}

k2hvec_handle_t k2hvec_attach_memory(int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
{
	k2hvec_handle_t	k2hvec = new(std::nothrow) k2hvec_handle();		// as same as K2hVecDb()

	if(!k2hvec->AttachK2hashMem(mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize)){
		ERR_VECPRN("Failed to initialize k2hvec(attach memory).");
		delete k2hvec;
		return nullptr;
	}
	return k2hvec;
}

k2hvec_handle_t k2hvec_attach_k2hash(k2h_h k2hhandle)
{
	K2HShm*	pk2hobj = reinterpret_cast<K2HShm*>(k2hhandle);
	if(!pk2hobj){
		ERR_VECPRN("Invalid k2hash object handle.");
		return nullptr;
	}

	k2hvec_handle_t	k2hvec = new(std::nothrow) k2hvec_handle();		// as same as K2hVecDb()

	if(!k2hvec->SetK2hash(pk2hobj)){
		ERR_VECPRN("Failed to attach k2hash object(handle).");
		delete k2hvec;
		return nullptr;
	}
	return k2hvec;
}

void k2hvec_delete(k2hvec_handle_t k2hvechandle)
{
	if(k2hvechandle){
		delete k2hvechandle;
	}
}

//---------------------------------------------------------
// Functions - Attributes (not recommended)
//---------------------------------------------------------
bool k2hvec_set_attribute(k2hvec_handle_t k2hvechandle, const bool* is_mtime, const bool* is_defenc, const char* passfile, const bool* is_history, const time_t* expire)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	return k2hvechandle->SetCommonAttribute(is_mtime, is_defenc, passfile, is_history, expire);
}

bool k2hvec_add_passphrase(k2hvec_handle_t k2hvechandle, const char* pass, bool is_default_encrypt)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	return k2hvechandle->AddAttrCryptPass(pass, is_default_encrypt);
}

bool k2hvec_clear_attribute(k2hvec_handle_t k2hvechandle)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	return k2hvechandle->CleanCommonAttribute();
}

//---------------------------------------------------------
// Functions - Set/Get/Remove/Search
//---------------------------------------------------------
//
// Set: embedding & content -> k2hash
//
bool k2hvec_set(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel, const float* psysembedding, size_t sysemlength, const char* ppass, const time_t* pexpire)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	return k2hvechandle->Set(pembedding, emlength, pcontent, pemmodel, psysembedding, sysemlength, ppass, pexpire);
}

//
// Get: embedding -> k2hash key(string)
//
char* k2hvec_get_k2hash_key(const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength)
{
	// Check parameters
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0){
		ERR_VECPRN("Empty embedding data is specified.");
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

	// Get key
	std::string	k2hkey;
	if(!K2hVecDb::GetK2hashKey(k2hkey, Embedding, pstrmodel, pSysEmbeddingVec)){
		return nullptr;
	}
	return strdup(k2hkey.c_str());
}

//
// Get: k2hash key(string) -> embedding data(on k2hash)
//
float* k2hvec_get_embedding(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass, size_t* plength)
{
	// Check parameters
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return nullptr;
	}
	if(VECEMPTYSTR(pkey)){
		ERR_VECPRN("key parameter is nullptr.");
		return nullptr;
	}
	std::string			strpass;
	const std::string*	pstrpass = nullptr;
	if(!VECEMPTYSTR(ppass)){
		strpass		= ppass;
		pstrpass	= &strpass;
	}
	size_t	tmpLength	= 0UL;
	size_t*	ptmpLength;
	if(plength){
		ptmpLength = plength;
	}else{
		ptmpLength = &tmpLength;
	}

	// Get
	std::vector<float>	embedding;
	std::string			content;
	if(!k2hvechandle->Get(pkey, embedding, content, checkattr, pstrpass)){
		return nullptr;
	}

	return DupFloatArray(embedding, *ptmpLength);
}

//
// Get: k2hash key(string) -> content data(on k2hash)
//
char* k2hvec_get_content(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass)
{
	// Check parameters
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return nullptr;
	}
	if(VECEMPTYSTR(pkey)){
		ERR_VECPRN("key parameter is nullptr.");
		return nullptr;
	}
	std::string			strpass;
	const std::string*	pstrpass = nullptr;
	if(!VECEMPTYSTR(ppass)){
		strpass		= ppass;
		pstrpass	= &strpass;
	}

	// Get
	std::vector<float>	embedding;
	std::string			content;
	if(!k2hvechandle->Get(pkey, embedding, content, checkattr, pstrpass)){
		return nullptr;
	}
	return strdup(content.c_str());
}

//
// Get: embedding data -> content data(on k2hash)
//
char* k2hvec_get_embedding_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return nullptr;
	}
	return k2hvechandle->Get(pembedding, emlength, pemmodel, psysembedding, sysemlength, checkattr, ppass);
}

//
// Remove: embedding data -> remove it
//
bool k2hvec_remove_embedding(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, bool mainkeyonly, const char* pemmodel, const float* psysembedding, size_t sysemlength)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	return k2hvechandle->Remove(pembedding, emlength, mainkeyonly, pemmodel, psysembedding, sysemlength);
}

//
// Remove: k2hash key(string) -> remove it
//
bool k2hvec_remove_key(k2hvec_handle_t k2hvechandle, const char* pkey, bool mainkeyonly)
{
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	if(VECEMPTYSTR(pkey)){
		ERR_VECPRN("key parameter is nullptr.");
		return false;
	}
	return k2hvechandle->Remove(pkey, mainkeyonly);
}

//
// Utility Search: embedding -> all
//
static bool k2hvec_search_raw(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
{
	// Check parameters
	if(!k2hvechandle){
		ERR_VECPRN("k2hvec handle parameter is nullptr.");
		return false;
	}
	std::vector<float>	Embedding;
	if(!pembedding || emlength == 0){
		ERR_VECPRN("Empty embedding data is specified.");
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

	std::string			strpass;
	const std::string*	pstrpass = nullptr;
	if(!VECEMPTYSTR(ppass)){
		strpass		= ppass;
		pstrpass	= &strpass;
	}

	// Search
	if(!k2hvechandle->Search(Embedding, res_key, res_em, res_sysem, res_content, res_model, score, pstrmodel, pSysEmbeddingVec, checkattr, pstrpass)){
		return false;
	}
	return true;
}

//
// Search: embedding -> k2hash key(string)
//
char* k2hvec_search_key(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
{
	// Search
	std::string			res_key;
	std::vector<float>	res_em;
	std::vector<float>	res_sysem;
	std::string			res_content;
	std::string			res_model;
	if(!k2hvec_search_raw(k2hvechandle, pembedding, emlength, res_key, res_em, res_sysem, res_content, res_model, score, pemmodel, psysembedding, sysemlength, checkattr, ppass)){
		return nullptr;
	}
	return strdup(res_key.c_str());
}

//
// Search: embedding -> content
//
char* k2hvec_search_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
{
	// Search
	std::string			res_key;
	std::vector<float>	res_em;
	std::vector<float>	res_sysem;
	std::string			res_content;
	std::string			res_model;
	if(!k2hvec_search_raw(k2hvechandle, pembedding, emlength, res_key, res_em, res_sysem, res_content, res_model, score, pemmodel, psysembedding, sysemlength, checkattr, ppass)){
		return nullptr;
	}
	return strdup(res_content.c_str());
}

//---------------------------------------------------------
// Functions - Version
//---------------------------------------------------------
extern char k2hvec_commit_hash[];

void k2hvec_print_version(FILE* stream)
{
	static const char format[] =
		"\n"
		"K2hvec Version %s (commit: %s)\n"
		"\n"
		"Copyright(C) 2026 LY Corporation.\n"
		"\n"
		"K2HVEC is a high-performance vector database for fast vector search,\n"
		"built on top of the K2HASH key-value store.\n"
		"\n";

	if(!stream){
		stream = stdout;
	}
	fprintf(stream, format, VERSION, k2hvec_commit_hash);

	k2h_print_version(stream);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
