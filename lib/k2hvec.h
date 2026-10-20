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

#ifndef	K2HVEC_H
#define	K2HVEC_H

#include <k2hash/k2hash.h>

#include "k2hveccommon.h"
#include "k2hvectypes.h"

DECL_EXTERN_C_START

//---------------------------------------------------------
// Functions - Debug
//---------------------------------------------------------
// k2hvec_bump_debug_level					Bumpup debugging level(silent -> error -> warning -> messages ->...)
// k2hvec_set_debug_level_silent			Set silent for debugging level
// k2hvec_set_debug_level_error				Set error for debugging level
// k2hvec_set_debug_level_warning			Set warning for debugging level
// k2hvec_set_debug_level_message			Set message for debugging level
// k2hvec_set_debug_level_dump				Set dump for debugging level
// k2hvec_set_debug_file					Set file path for debugging message
// k2hvec_unset_debug_file					Unset file path for debugging message to stderr(default)
// k2hvec_load_debug_env					Set debugging level and file path by loading environment.
//
extern void k2hvec_bump_debug_level(void);
extern void k2hvec_set_debug_level_silent(void);
extern void k2hvec_set_debug_level_error(void);
extern void k2hvec_set_debug_level_warning(void);
extern void k2hvec_set_debug_level_message(void);
extern void k2hvec_set_debug_level_dump(void);
extern bool k2hvec_set_debug_file(const char* filepath);
extern bool k2hvec_unset_debug_file(void);
extern bool k2hvec_load_debug_env(void);

//---------------------------------------------------------
// Functions - Common default variables
//---------------------------------------------------------
// k2hvec_set_default_score_limit					Set the default limit of cosine approximation value for vector search
// k2hvec_reset_default_score_limit					Reset the default limit of cosine approximation value for vector search
// k2hvec_get_default_score_limit					Get the default limit of cosine approximation value for vector search
// 
// k2hvec_set_default_embedding_model				Set the default embedding model string
// k2hvec_reset_default_embedding_model				Reset(clear) the default embedding model string
// k2hvec_get_default_embedding_model				Get the default embedding model string
// 
// k2hvec_set_default_system_embedding				Set the default system embedding data
// k2hvec_reset_default_system_embedding			Reset(clear) the default system embedding data
// k2hvec_get_default_system_embedding				Get current default system embedding data
// 
// k2hvec_set_default_expire						Set the default expire piriod for storing data to k2hash
// k2hvec_reset_default_expire						Reset(clear) the default expire piriod for storing data to k2hash
// k2hvec_get_default_expire						Get current default expire piriod for storing data to k2hash
// 
// k2hvec_set_default_passphrase					Set the default crypt passphrase for storing data to k2hash
// k2hvec_reset_default_passphrase					Reset(clear) the default crypt passphrase for storing data to k2hash
// k2hvec_get_default_passphrase					Get current default crypt passphrase for storing data to k2hash
// 
// k2hvec_set_default_binary_hash_size				Set the default binary hash bit size
// k2hvec_reset_default_binary_hash_size			Reset the default binary hash bit size
// k2hvec_get_default_binary_hash_size				Get current default binary hash bit size
// 
// k2hvec_set_default_neighbor_key_limit			Set the maximum number of neighbor keys allowed for vector similarity searches
// k2hvec_reset_default_neighbor_key_limit			Reset the maximum number of neighbor keys allowed for vector similarity searches
// k2hvec_get_default_neighbor_key_limit			Get the maximum number of neighbor keys allowed for vector similarity searches
//
// k2hvec_set_content_direct_limit					Sets the maximum size for saving content text directly to the main key
// k2hvec_reset_content_direct_limit				Reset the maximum size for saving content text directly to the main key
// k2hvec_get_content_direct_limit					Get the maximum size for saving content text directly to the main key
//
extern bool k2hvec_set_default_score_limit(float val);
extern void k2hvec_reset_default_score_limit(void);
extern float k2hvec_get_default_score_limit(void);

extern bool k2hvec_set_default_embedding_model(const char* pmodel);
extern void k2hvec_reset_default_embedding_model(void);
extern char* k2hvec_get_default_embedding_model(void);

extern bool k2hvec_set_default_system_embedding(const float* pembedding, size_t length);
extern void k2hvec_reset_default_system_embedding(void);
extern float* k2hvec_get_default_system_embedding(size_t* plength);

extern time_t k2hvec_set_default_expire(time_t expire);
extern void k2hvec_reset_default_expire(void);
extern time_t k2hvec_get_default_expire();

extern void k2hvec_set_default_passphrase(const char* ppass);
extern void k2hvec_reset_default_passphrase(void);
extern char* k2hvec_get_default_passphrase();

extern bool k2hvec_set_default_binary_hash_size(int bitsize);
extern void k2hvec_reset_default_binary_hash_size(void);
extern int k2hvec_get_default_binary_hash_size(void);

extern bool k2hvec_set_default_neighbor_key_limit(size_t limit);
extern void k2hvec_reset_default_neighbor_key_limit(void);
extern size_t k2hvec_get_default_neighbor_key_limit(void);

extern bool k2hvec_set_content_direct_limit(size_t limit);
extern void k2hvec_reset_content_direct_limit(void);
extern size_t k2hvec_get_content_direct_limit(void);

//---------------------------------------------------------
// Functions - Initialize
//---------------------------------------------------------
// k2hvec_create							Create K2hVec handle with creating k2hash file/memory
// k2hvec_attach							Create K2hVec handle with attaching k2hash file or creating memory
// k2hvec_attach_memory						Create K2hVec handle with attaching memory
// k2hvec_attach_k2hash						Create K2hVec handle with attaching k2hash object handle
// k2hvec_delete							Destroy(free) K2hVec handle
//
extern k2hvec_handle_t k2hvec_create(const char* file, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize);
extern k2hvec_handle_t k2hvec_attach(const char* file, bool isReadOnly, bool isCreate, bool isTempFile, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize);
extern k2hvec_handle_t k2hvec_attach_memory(int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize);
extern k2hvec_handle_t k2hvec_attach_k2hash(k2h_h k2hhandle);
extern void k2hvec_delete(k2hvec_handle_t k2hvechandle);

//---------------------------------------------------------
// Functions - Attributes (not recommended)
//---------------------------------------------------------
// k2hvec_set_attribute						Set attributes for accessing k2hash
// k2hvec_add_passphrase					Add crypt passphrase
// k2hvec_clear_attribute					Clear all attributes for accessing k2hash
//
extern bool k2hvec_set_attribute(k2hvec_handle_t k2hvechandle, const bool* is_mtime, const bool* is_defenc, const char* passfile, const bool* is_history, const time_t* expire);
extern bool k2hvec_add_passphrase(k2hvec_handle_t k2hvechandle, const char* pass, bool is_default_encrypt);
extern bool k2hvec_clear_attribute(k2hvec_handle_t k2hvechandle);

//---------------------------------------------------------
// Functions - Set/Get/Remove/Search
//---------------------------------------------------------
// k2hvec_set								Set data(content) by embedding
//
// k2hvec_get_k2hash_key					Get k2hash key
//
// k2hvec_get_embedding						Get embedding data by k2hash key
// k2hvec_get_content						Get content by k2hash content
// k2hvec_get_embedding_content				Get content by embedding data
//
// k2hvec_remove_embedding					Remove data(content, etc) by embedding data
// k2hvec_remove_key						Remove data(content, etc) by k2hash key
//
// k2hvec_search_key						Vector Search k2hash key by embedding data
// k2hvec_search_content					Vector Search conetnt by embedding data
//
extern bool k2hvec_set(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel, const float* psysembedding, size_t sysemlength, const char* ppass, const time_t* pexpire);

extern char* k2hvec_get_k2hash_key(const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength);

extern float* k2hvec_get_embedding(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass, size_t* plength);
extern char* k2hvec_get_content(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass);
extern char* k2hvec_get_embedding_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass);

extern bool k2hvec_remove_embedding(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, bool mainkeyonly, const char* pemmodel, const float* psysembedding, size_t sysemlength);
extern bool k2hvec_remove_key(k2hvec_handle_t k2hvechandle, const char* pkey, bool mainkeyonly);

extern char* k2hvec_search_key(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass);
extern char* k2hvec_search_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass);

//---------------------------------------------------------
// Version
//---------------------------------------------------------
extern void k2hvec_print_version(FILE* stream);

DECL_EXTERN_C_END

#endif	// K2HVEC_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
