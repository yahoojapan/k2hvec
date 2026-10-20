---
layout: contents
language: en-us
title: Developer
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: developerja.html
lang_opp_word: To Japanese
prev_url: build.html
prev_string: Build
top_url: index.html
top_string: TOP
next_url: environments.html
next_string: Environments
---

<!-- =========================================================== -->
# For developer

#### [C API](#CAPI)
[Debug family(C I/F)](#DEBUG)  
[Common Setting family(C I/F)](#SETTING)  
[Initialization family(C I/F)](#INITIALIZATION)  
[Attribute family(C I / F)](#ATTR)  
[Operation family(C I/F)](#OPERATION)  
#### [C++ API](#CPP)
[K2hVecDb class](#K2HVECDB)  

<!-- =========================================================== -->
***

## <a name="CAPI"> C API
It is an API for C language.  
Include the following header files when developing.  


 ```
#include <k2hvec/k2hvec.h>
 ```

When linking please specify the following as an option.
 ```
-lk2hvec
 ```

The functions for C language are explained below.

<!-- =========================================================== -->
***

### <a name="DEBUG"> Debug family(C I/F)
The K2HVEC library can output messages to check internal operation and API operation.
This function group is a group of functions for controlling message output

#### Format
- void k2hvec_bump_debug_level(void)
- void k2hvec_set_debug_level_silent(void)
- void k2hvec_set_debug_level_error(void)
- void k2hvec_set_debug_level_warning(void)
- void k2hvec_set_debug_level_message(void)
- void k2hvec_set_debug_level_dump(void)
- bool k2hvec_set_debug_file(const char* filepath)
- bool k2hvec_unset_debug_file(void)
- bool k2hvec_load_debug_env(void)

#### Description
- k2hvec_bump_debug_level  
  Bump up message output level as non-output -> error -> warning -> information -> non-output.
- k2hvec_set_debug_level_silent  
  Set the message output level to no output.
- k2hvec_set_debug_level_error  
  Set the message output level to error.
- k2hvec_set_debug_level_warning  
  Set the message output level above the warning.
- k2hvec_set_debug_level_message  
  Set the message output level above the information.
- k2hvec_set_debug_level_dump  
  Set the message output level above the dump.
- k2hvec_set_debug_file  
  Specify the file to output message. If it is not set, it is output to stderr.
- k2hvec_unset_debug_file  
  Unset to output messages to stderr.
- k2hvec_load_debug_env  
  Read the environment variables VECDBGMODE, VECDBGFILE and set the message output and output destination according to the value.

#### Return Values
k2hvec_set_debug_file, k2hvec_unset_debug_file, k2hvec_load_debug_env will return true on success. If it fails, it returns false.

#### Note
For the environment variables VECDBGMODE and VECDBGFILE, see the [Environments](environments.html) in the manual.

#### Examples
 ```
k2hvec_unset_debug_file("/var/log/k2hvec/error.log");
k2hvec_set_debug_level_dump();
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="SETTING"> Common Setting family(C I/F)
A set of functions for configuring common parameters governing the behavior of the K2HVEC library.

#### Format
- bool k2hvec_set_default_score_limit(float val)
- void k2hvec_reset_default_score_limit(void)
- float k2hvec_get_default_score_limit(void)
- bool k2hvec_set_default_embedding_model(const char* pmodel)
- void k2hvec_reset_default_embedding_model(void)
- char* k2hvec_get_default_embedding_model(void)
- bool k2hvec_set_default_system_embedding(const float* pembedding, size_t length)
- void k2hvec_reset_default_system_embedding(void)
- float* k2hvec_get_default_system_embedding(size_t length)
- time_t k2hvec_set_default_expire(time_t expire)
- void k2hvec_reset_default_expire(void)
- time_t k2hvec_get_default_expire()
- void k2hvec_set_default_passphrase(const char* ppass)
- void k2hvec_reset_default_passphrase(void)
- char* k2hvec_get_default_passphrase()
- bool k2hvec_set_default_binary_hash_size(int bitsize)
- void k2hvec_reset_default_binary_hash_size(void)
- int k2hvec_get_default_binary_hash_size(void)
- bool k2hvec_set_default_neighbor_key_limit(size_t limit)
- void k2hvec_reset_default_neighbor_key_limit(void)
- size_t k2hvec_get_default_neighbor_key_limit(void)
- bool k2hvec_set_content_direct_limit(size_t limit)
- void k2hvec_reset_content_direct_limit(void)
- size_t k2hvec_get_content_direct_limit(void)

#### Description
- k2hvec_set_default_score_limit  
  Sets the common lower threshold value of Cosine Similarity for vector similarity search.
- k2hvec_reset_default_score_limit  
  Resets the common lower threshold value of Cosine Similarity for vector similarity search.
- k2hvec_get_default_score_limit  
  Get the common lower threshold value of Cosine Similarity for vector similarity search.
- k2hvec_set_default_embedding_model  
  Sets the common model name for vector embeddings(array of floating-point numbers).
- k2hvec_reset_default_embedding_model  
  Resets the common model name for vector embeddings.
- k2hvec_get_default_embedding_model  
  Gets the common model name for vector embeddings.
- k2hvec_set_default_system_embedding  
  Sets the common system model name(base prerequisite vector embedding).
- k2hvec_reset_default_system_embedding  
  Resets the common system model name.
- k2hvec_get_default_system_embedding  
  Gets the currently configured common system model name.
- k2hvec_set_default_expire  
  Sets the common expiration time(in seconds) applied when writing data.
- k2hvec_reset_default_expire  
  Resets the common expiration time setting.
- k2hvec_get_default_expire  
  Gets the currently configured common expiration time(in seconds).
- k2hvec_set_default_passphrase  
  Sets the common passphrase used for encrypted data storage.
- k2hvec_reset_default_passphrase  
  Resets the common passphrase used for encryption.
- k2hvec_get_default_passphrase  
  Gets the currently configured common passphrase for encryption.
- k2hvec_set_default_binary_hash_size  
  Specifies the bit length of the binary hash value derived from vector embeddings.
- k2hvec_reset_default_binary_hash_size  
  Resets the bit length setting of the binary hash value.
- k2hvec_get_default_binary_hash_size  
  Gets the currently configured bit length of the binary hash value.
- k2hvec_set_default_neighbor_key_limit  
  Sets the maximum number of neighbor keys allowed for vector similarity searches.
- k2hvec_reset_default_neighbor_key_limit  
  Resets the maximum number of neighbor keys allowed for vector similarity searches.
- k2hvec_get_default_neighbor_key_limit  
  Gets the maximum number of neighbor keys allowed for vector similarity searches
- k2hvec_set_content_direct_limit  
  Sets the maximum size for saving content text directly to the main key.
- k2hvec_reset_content_direct_limit  
  Reset the maximum size for saving content text directly to the main key.
- k2hvec_get_content_direct_limit  
  Get the maximum size for saving content text directly to the main key.

#### Parameters
- k2hvec_set_default_score_limit
    - float val  
      Specifies the common lower threshold value of Cosine Similarity.
- k2hvec_set_default_embedding_model
    - const char* pmodel  
      Specifies the common model name.
- k2hvec_set_default_system_embedding
    - float* pembedding  
      Specifies the array pointer for the base prerequisite vector embedding (system model name).
    - size_t length  
      Specifies the array length for the base prerequisite vector embedding (system model name).
- k2hvec_get_default_system_embedding
    - size_t* plength  
      Specifies the buffer to receive the array length of the retrieved base prerequisite vector embedding (system model name).
- k2hvec_set_default_expire
    - time_t expire  
      Specifies the common expiration time (in seconds).
- k2hvec_set_default_passphrase
    - const char* ppass  
      Specifies the common passphrase.
- k2hvec_set_default_binary_hash_size
    - int bitsize  
      Specifies the bit length setting for the binary hash value.
- k2hvec_set_default_neighbor_key_limit
    - size_t limit  
   Specifies the maximum number of neighbor keys allowed for vector similarity searches.
- k2hvec_set_content_direct_limit
    - size_t limit  
   Specifies the maximum size for saving content text directly to the main key.

#### Return Values
- k2hvec_set_default_score_limit  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_default_score_limit  
  Returns nothing.
- k2hvec_get_default_score_limit  
  Returns the common lower threshold value of Cosine Similarity for vector similarity search.
- k2hvec_set_default_embedding_model  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_default_embedding_model  
  Returns nothing.
- k2hvec_get_default_embedding_model  
  Returns the common model name for vector embeddings.
- k2hvec_set_default_system_embedding  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_default_system_embedding  
  Returns nothing.
- k2hvec_get_default_system_embedding  
  Returns a pointer to the array for the base prerequisite vector embedding(system model name), or `NULL` on failure. The returned pointer must be freed using `free()`.
- k2hvec_set_default_expire  
  Returns the previously configured expiration time (in seconds).
- k2hvec_reset_default_expire  
  Returns nothing.
- k2hvec_get_default_expire  
  Returns the currently configured expiration time (in seconds).
- k2hvec_set_default_passphrase  
  Returns nothing.
- k2hvec_reset_default_passphrase  
  Returns nothing.
- k2hvec_get_default_passphrase  
  Returns the currently configured common passphrase. The returned pointer must be freed using `free()`.
- k2hvec_set_default_binary_hash_size  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_default_binary_hash_size  
  Returns nothing.
- k2hvec_get_default_binary_hash_size  
  Returns the currently configured bit length of the binary hash value.
- k2hvec_set_default_neighbor_key_limit  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_default_neighbor_key_limit  
  Returns nothing.
- k2hvec_get_default_neighbor_key_limit  
  Returns the maximum number of neighbor keys allowed for vector similarity searches.
- k2hvec_set_content_direct_limit  
  Returns true if it succeeds. If it fails, it returns false.
- k2hvec_reset_content_direct_limit  
  Returns nothing.
- k2hvec_get_content_direct_limit  
  Returns the maximum size for saving content text directly to the main key.

#### Examples
 ```
if(k2hvec_set_default_score_limit(0.9f)){
    return false;
}

size_t limit = k2hvec_get_default_neighbor_key_limit();
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="INITIALIZATION"> Initialization family(C I/F)
A set of functions for initializing K2Hash (database files and in-memory) used by the K2HVEC library.

#### Format
- k2hvec_handle_t k2hvec_create(const char* file, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach(const char* file, bool isReadOnly, bool isCreate, bool isTempFile, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach_memory(int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach_k2hash(k2h_h k2hhandle)
- void k2hvec_delete(k2hvec_handle_t k2hvechandle)

#### Description
- k2hvec_create  
  Creates and attaches a K2Hash database file used by the K2HVEC library.
- k2hvec_attach  
  Attaches to an existing K2Hash database file(or new creating file) used by the K2HVEC library.
- k2hvec_attach_memory  
  Creates and attaches an in-memory K2Hash database used by the K2HVEC library.
- k2hvec_attach_k2hash  
  Attaches directly using an existing K2Hash handle for the K2HVEC library.
- k2hvec_delete  
  Detaches and destroys the K2Hash handle.

#### Parameters
- k2hvec_create
    - const char* file  
      Specifies the path to the K2Hash database file to be created.
    - bool isfullmapping  
      Specifies whether to map the K2Hash database file in full mapping mode or partial mapping mode.
    - int mask_bitcnt  
      Specifies the mask bit count for the K2Hash database file.
    - int cmask_bitcnt  
      Specifies the collision mask (cmask) bit count for the K2Hash database file.
    - int max_element_cnt  
      Specifies the element count for the K2Hash database file.
    - size_t pagesize  
      Specifies the page size for the K2Hash database file.
- k2hvec_attach
    - const char* file  
      Specifies the path to the K2Hash database file to be attached.
    - bool isReadOnly  
      Specifies whether to attach the K2Hash database file in read-only mode.
    - bool isCreate  
      Specifies whether to create the K2Hash database file if it does not exist.
    - bool isTempFile  
      Specifies whether to use a temporary K2Hash database file when creating a new file.
    - bool isfullmapping  
      Specifies whether to map the attached K2Hash database file in full mapping mode or partial mapping mode.
    - int mask_bitcnt  
      Specifies the mask bit count for the K2Hash database file when creating a new file.
    - int cmask_bitcnt  
      Specifies the collision mask (cmask) bit count for the K2Hash database file when creating a new file.
    - int max_element_cnt  
      Specifies the element count for the K2Hash database file when creating a new file.
    - size_t pagesize  
      Specifies the page size for the K2Hash database file when creating a new file.
- k2hvec_attach_memory
    - int mask_bitcnt  
      Specifies the mask bit count for the in-memory K2Hash database.
    - int cmask_bitcnt  
      Specifies the collision mask (cmask) bit count for the in-memory K2Hash database.
    - int max_element_cnt  
      Specifies the element count for the in-memory K2Hash database.
    - size_t pagesize  
      Specifies the page size for the in-memory K2Hash database.
- k2hvec_attach_k2hash
    - k2h_h k2hhandle  
      Specifies an already opened K2Hash handle.
- k2hvec_delete
    - k2hvec_handle_t k2hvechandle  
      Destroys the attached K2Hash handle.

#### Return Values
- k2hvec_create  
  Returns a K2HVEC handle on success, or `NULL` on failure.
- k2hvec_attach  
  Returns a K2HVEC handle on success, or `NULL` on failure.
- k2hvec_attach_memory  
  Returns a K2HVEC handle on success, or `NULL` on failure.
- k2hvec_attach_k2hash  
  Returns a K2HVEC handle on success, or `NULL` on failure.
- k2hvec_delete  
  Returns nothing.

#### Note
The returned K2HVEC handle (`k2hvec_handle_t`) after creation and attachment must be destroyed using `k2hvec_delete` when it is no longer needed.

#### Examples
 ```
k2hvec_handle_t hk2hvec = k2hvec_create("/tmp/my.k2h", true, 4, 8, 8, 1024);
if(!hk2hvec){
    return false;
}

k2hvec_delete(hk2hvec);
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="ATTR"> Attribute family(C I/F)
A set of functions for configuring and retrieving common parameters governing the attributes of the entire K2HVEC library.

#### Format
- bool k2hvec_set_attribute(k2hvec_handle_t k2hvechandle, const bool* is_mtime, const bool* is_defenc, const char* passfile, const bool* is_history, const time_t* expire)
- bool k2hvec_add_passphrase(k2hvec_handle_t k2hvechandle, const char* pass, bool is_default_encrypt)
- bool k2hvec_clear_attribute(k2hvec_handle_t k2hvechandle)

#### Description
- k2hvec_set_attribute  
  Configures settings for update timestamps, encryption passphrases(including whether to set it as the default passphrase), data update history usage, and expiration time(in seconds) for K2Hash.
- k2hvec_add_passphrase  
  Configures the encryption passphrase for K2Hash and specifies whether to set it as the default passphrase.
- k2hvec_clear_attribute  
  Resets K2Hash attributes (update timestamp usage, encryption passphrase settings, and expiration time) to their default values.

#### Parameters
- k2hvec_set_attribute
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const bool* is_mtime  
      Specifies a buffer indicating whether to use update timestamps. If this setting is not used, specify `NULL`.
    - const bool* is_defenc  
      Specifies a buffer indicating whether to set the encryption passphrase as default. If this setting is not used, specify `NULL`.
    - const char* passfile  
      Specifies the encryption passphrase to set. If this setting is not used, specify `NULL`.
    - const bool* is_history  
      Specifies a buffer indicating whether to enable data update history. If this setting is not used, specify `NULL`.
    - const time_t* expire  
      Specifies a buffer indicating the expiration time (in seconds). If this setting is not used, specify `NULL`.
- k2hvec_add_passphrase
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const char* pass  
      Specifies the encryption passphrase to set.
    - bool is_default_encrypt  
      Specifies whether to set the encryption passphrase as the default passphrase.
- k2hvec_clear_attribute
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.

#### Return Values
These functions will return true on success. If it fails, it returns false.

#### Examples
 ```
bool        is_mtime   = true;
bool        is_defenc  = true;
const char* passfile   = "mypassphrase";
bool        is_history = true;
time_t      expire     = 120;
if(!k2hvec_set_attribute(hk2hvec, &is_mtime, &is_defenc, passfile, &is_history, &expire)){
    return false;
}

if(k2hvec_clear_attribute(hk2hvec)){
    return false;
}
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="OPERATION"> Operation family(C I/F)
A set of functions for performing basic data operations, such as writing and reading, in K2HVEC (K2Hash).

#### Format
- bool k2hvec_set(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel, const float* psysembedding, size_t sysemlength, const char* ppass, const time_t* pexpire)
- char* k2hvec_get_k2hash_key(const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength)
- float* k2hvec_get_embedding(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass, size_t* plength)
- char* k2hvec_get_content(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass)
- char* k2hvec_get_embedding_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
- bool k2hvec_remove_embedding(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, bool mainkeyonly, const char* pemmodel, const float* psysembedding, size_t sysemlength)
- bool k2hvec_remove_key(k2hvec_handle_t k2hvechandle, const char* pkey, bool mainkeyonly)
- char* k2hvec_search_key(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
- char* k2hvec_search_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)

#### Description
- k2hvec_set  
  Sets a vector embedding (floating-point array) and its corresponding value in K2HVEC (K2Hash).
- k2hvec_get_k2hash_key  
  Converts a vector embedding (floating-point array) into a K2Hash key name.
- k2hvec_get_embedding  
  Retrieves a vector embedding (floating-point array) by specifying a K2Hash key name.
- k2hvec_get_content  
  Retrieves a value by specifying a K2Hash key name.
- k2hvec_get_embedding_content  
  Retrieves a value by specifying a vector embedding (floating-point array).
- k2hvec_remove_embedding  
  Deletes data by specifying a K2Hash key name.
- k2hvec_remove_key  
  Deletes data by specifying a vector embedding (floating-point array).
- k2hvec_search_key  
  Performs a vector similarity search using a vector embedding (floating-point array) and retrieves the key names.
- k2hvec_search_content  
  Performs a vector similarity search using a vector embedding (floating-point array) and retrieves the values.

#### Parameters
- k2hvec_set
    - k2hvec_handle_t k2hvechandle  
         Specifies the attached K2HVEC handle.
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - const char* pcontent  
      Specifies a value(contents).
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
    - const char* ppass  
      Specifies the encryption passphrase. If `NULL` is specified, the common passphrase is used.
    - const time_t* pexpire  
      Specifies the expiration time for the data to be set. If `NULL` is specified, the common expiration time is used.
- k2hvec_get_k2hash_key
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
- k2hvec_get_embedding
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const char* pkey  
      Specifies key name.
    - bool checkattr  
      Specifies whether to check K2Hash attributes during retrieval.
    - const char* ppass  
      Specifies the passphrase for decryption. If `NULL` is specified, the common passphrase is used.
    - size_t* plength  
      Specifies a buffer to store the length of the retrieved vector embedding (floating-point array).
- k2hvec_get_content
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const char* pkey  
      Specifies key name.
    - bool checkattr  
      Specifies whether to check K2Hash attributes during retrieval.
    - const char* ppass  
      Specifies the passphrase for decryption. If `NULL` is specified, the common passphrase is used.
- k2hvec_get_embedding_content
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
    - bool checkattr  
      Specifies whether to check K2Hash attributes during retrieval.
    - const char* ppass  
      Specifies the passphrase for decryption. If `NULL` is specified, the common passphrase is used.
- k2hvec_remove_key
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const char* pkey  
      Specifies key name.
    - bool mainkeyonly  
      Specifies whether to retrieve only the primary keys.
- k2hvec_remove_embedding
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - bool mainkeyonly  
      Specifies whether to remove only the primary keys.
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
- k2hvec_search_key
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - float score  
      Specifies the lower threshold value of Cosine Similarity for vector similarity search.
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
    - bool checkattr  
      Specifies whether to check K2Hash attributes during retrieval.
    - const char* ppass  
      Specifies the passphrase for decryption. If `NULL` is specified, the common passphrase is used.
- k2hvec_search_content
    - k2hvec_handle_t k2hvechandle  
      Specifies the attached K2HVEC handle.
    - const float* pembedding  
      Specifies a vector embedding (floating-point array).
    - size_t emlength  
      Specifies a vector embedding (floating-point array) length.
    - float score  
      Specifies the lower threshold value of Cosine Similarity for vector similarity search.
    - const char* pemmodel  
      Specifies the model name. If `NULL` is specified, the common model name is used.
    - const float* psysembedding  
      Specifies an array of system vector embeddings. If `NULL` is specified, the common system vector embeddings is used.
    - size_t sysemlength  
      Specifies the length of the array of system vector embeddings.
    - bool checkattr  
      Specifies whether to check K2Hash attributes during retrieval.
    - const char* ppass  
      Specifies the passphrase for decryption. If `NULL` is specified, the common passphrase is used.

#### Return Values
- k2hvec_set  
  These functions will return true on success. If it fails, it returns false.
- k2hvec_get_k2hash_key  
  Returns the K2Hash key name.
- k2hvec_get_embedding  
  Returns a pointer to the retrieved vector embedding (floating-point array). Returns NULL on failure.
- k2hvec_get_content  
  Returns a pointer to the value(content). Returns NULL on failure.
- k2hvec_get_embedding_content  
  Returns a pointer to the value(content). Returns NULL on failure.
- k2hvec_remove_embedding  
  These functions will return true on success. If it fails, it returns false.
- k2hvec_remove_key  
  These functions will return true on success. If it fails, it returns false.
- k2hvec_search_key  
  Returns a pointer to the value(content). Returns NULL on failure.
- k2hvec_search_content  
  Returns a pointer to the value(content). Returns NULL on failure.

#### Examples
 ```
k2hvec_handle_t hk2hvec = k2hvec_create("/tmp/my.k2h", true, 4, 8, 8, 1024);
if(!hk2hvec){
    return false;
}

float[] embedding = {...};
char[]  content   = "...";

if(!k2hvec_set(hk2hvec, embedding, sizeof(embedding) / sizeof(float), content, NULL, NULL, 0UL, NULL, NULL)){
    k2hvec_delete(hk2hvec);
    return false;
}

char* pResConent = k2hvec_get_embedding_content(hk2hvec, embedding, sizeof(embedding) / sizeof(float), content, NULL, NULL, 0UL, true, NULL);
if(!pResConent){
    k2hvec_delete(hk2hvec);
    return false;
}

char* pKey = k2hvec_search_key(hk2hvec, , embedding, sizeof(embedding) / sizeof(float), 0.9f, content, NULL, NULL, 0UL, true, NULL);
if(!pKey){
    free(pResConent);
    k2hvec_delete(hk2hvec);
    return false;
}

if(!k2hvec_remove_key(hk2hvec, pKey, false)){
    free(pKey);
    free(pResConent);
    k2hvec_delete(hk2hvec);
    return false;
}

free(pKey);
free(pResConent);
k2hvec_delete(hk2hvec);

return false;
 ```

<!-- =========================================================== -->
***

## <a name="CPP"> C++ API
This is the I/F of the C ++ language API for using the K2HVEC library.
<br />
<br />
Include the following header file at development time.
 ```
#include <k2hvec/k2hvec.h>
#include <k2hvec/k2hvecdb.h>
 ```
<br />
When linking please specify the following as an option.
 ```
-lk2hvec
 ```
<br />
The functions for the C ++ language are explained below.

<!-- =========================================================== -->
***

### <a name="K2HVECDB"> K2hVecDb class

#### Description
This class handles operations and implementation for K2HVEC data.  
All K2HVEC data operations are performed through an instance of this class.  
In C++, the basic workflow for managing K2HVEC data requires creating an instance of this class before carrying out any operations.  
An instance can be created by opening (attaching to) a K2Hash file or in-memory storage.  
Once operations are complete, ensure that you close (detach) the instance.

#### Class methods
- bool K2hVecDb::SetScoreLimit(float val)
- void K2hVecDb::ResetScoreLimit(void)
- float K2hVecDb::GetScoreLimit(void)
<br />
- bool K2hVecDb::SetEmbeddingModel(const std::string& model)
- void K2hVecDb::SetEmbeddingModel(const char* pmodel)
- void K2hVecDb::ResetEmbeddingModel(void)
- std::string K2hVecDb::GetEmbeddingModel(void)
<br />
- void K2hVecDb::SetSysEmbedding(const std::vector<float>& embedding)
- bool K2hVecDb::SetSysEmbedding(const float* pembedding, size_t length)
- const std::vector<float>& K2hVecDb::GetSysEmbedding(void)
- void K2hVecDb::ResetSysEmbedding(void)
<br />
- time_t K2hVecDb::SetExpire(time_t val)
- void K2hVecDb::ResetExpire(void)
- time_t K2hVecDb::GetExpire(void)
<br />
- std::string K2hVecDb::SetCryptPass(const std::string& pass)
- void K2hVecDb::SetCryptPass(const char* ppass)
- void K2hVecDb::ResetCryptPass(void)
- std::string K2hVecDb::GetCryptPass(void)
<br />
- bool K2hVecDb::SetBinHashSize(int bitsize)
- void K2hVecDb::ResetBinHashBitSize(void)
- int K2hVecDb::GetBinHashSize(void)
<br />
- bool K2hVecDb::SetNeighborKeysLimit(size_t limit)
- void K2hVecDb::ResetNeighborKeysLimit(void)
- size_t K2hVecDb::GetNeighborKeysLimit(void)
<br />
- size_t SetContentDirectLimit(size_t limit)
- void ResetContentDirectLimit(void)
- size_t GetContentDirectLimit(void)
<br />
- bool K2hVecDb::GetK2hashKey(std::string& k2hkey, const std::vector<float>& embedding, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr)

#### Class Method Descriptions
- K2hVecDb::SetScoreLimit  
  Sets the common lower threshold value of Cosine Similarity for vector similarity search.
- K2hVecDb::ResetScoreLimit  
  Resets the common lower threshold value of Cosine Similarity for vector similarity search.
- K2hVecDb::GetScoreLimit  
  Gets the common lower threshold value of Cosine Similarity for vector similarity search.
- K2hVecDb::SetEmbeddingModel  
  Sets the common model name for vector embeddings(array of floating-point numbers).
- K2hVecDb::ResetEmbeddingModel  
  Resets the common model name for vector embeddings.
- K2hVecDb::GetEmbeddingModel  
  Gets the common model name for vector embeddings.
- K2hVecDb::SetSysEmbedding  
  Sets the common system model name(base prerequisite vector embedding).
- K2hVecDb::GetSysEmbedding  
  Gets the currently configured common system model name.
- K2hVecDb::ResetSysEmbedding  
  Resets the common system model name.
- K2hVecDb::SetExpire  
  Sets the common expiration time(in seconds) applied when writing data.
- K2hVecDb::ResetExpire  
  Resets the common expiration time setting.
- K2hVecDb::GetExpire  
  Gets the currently configured common expiration time(in seconds).
- K2hVecDb::SetCryptPass  
  Sets the common passphrase used for encrypted data storage.
- K2hVecDb::ResetCryptPass  
  Resets the common passphrase used for encryption.
- K2hVecDb::GetCryptPass  
  Gets the currently configured common passphrase for encryption.
- K2hVecDb::SetBinHashSize  
  Specifies the bit length of the binary hash value derived from vector embeddings.
- K2hVecDb::ResetBinHashBitSize  
  Resets the bit length setting of the binary hash value.
- K2hVecDb::GetBinHashSize  
  Gets the currently configured bit length of the binary hash value.
- K2hVecDb::SetNeighborKeysLimit  
  Sets the maximum number of neighbor keys allowed for vector similarity searches.
- K2hVecDb::ResetNeighborKeysLimit  
  Resets the maximum number of neighbor keys allowed for vector similarity searches.
- K2hVecDb::GetNeighborKeysLimit  
  Gets the maximum number of neighbor keys allowed for vector similarity searches
- K2hVecDb::SetContentDirectLimit  
  Sets the maximum size for saving content text directly to the main key.
- K2hVecDb::ResetContentDirectLimit  
  Reset the maximum size for saving content text directly to the main key.
- K2hVecDb::GetContentDirectLimit  
  Get the maximum size for saving content text directly to the main key.
- K2hVecDb::GetK2hashKey  
  Converts a vector embedding (floating-point array) into a K2Hash key name.

#### Methods
- K2hVecDb(void)
- explicit K2hVecDb(K2HShm* pk2h)
- virtual ~K2hVecDb(void)
<br />
- bool Clean(void)
- bool IsAttached(void) const
<br />
- bool SetK2hash(K2HShm* pk2h)
- bool CreateK2hash(const char* file, bool isfullmapping = true, int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE)
- bool AttachK2hash(const char* file, bool isReadOnly, bool isCreate = true, bool isTempFile = false, bool isfullmapping = true, int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE)
- bool AttachK2hashMem(int mask_bitcnt = K2HShm::DEFAULT_MASK_BITCOUNT, int cmask_bitcnt = K2HShm::DEFAULT_COLLISION_MASK_BITCOUNT, int max_element_cnt = K2HShm::DEFAULT_MAX_ELEMENT_CNT, size_t pagesize = K2HShm::MIN_PAGE_SIZE) { return AttachK2hash(nullptr, false, true, false, true, mask_bitcnt, cmask_bitcnt, max_element_cnt, pagesize) }
<br />
- bool SetCommonAttribute(const bool* is_mtime = nullptr, const bool* is_defenc = nullptr, const char* passfile = nullptr, const bool* is_history = nullptr, const time_t* expire = nullptr)
- bool AddAttrCryptPass(const char* pass, bool is_default_encrypt = false)
- bool CleanCommonAttribute(void)
- std::string GetCommonAttributeInformation() const
<br />
- bool Set(const std::vector<float>& embedding, const std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, const std::string* pstrpass = nullptr, const time_t* pexpire = nullptr)
- bool Set(const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, const char* ppass = nullptr, const time_t* pexpire = nullptr)
<br />
- bool Get(const std::string& strKey, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Get(const std::string& strKey, std::vector<float>& em, std::string& content, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Get(const std::string& strKey, std::vector<float>& em, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Get(const std::string& strKey, std::string& content, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Get(const std::vector<float>& embedding, std::vector<float>& em, std::vector<float>& sysem, std::string& content, std::string& model, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Get(const std::vector<float>& embedding, std::string& content, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const
- char* Get(const float* pembedding, size_t emlength, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, bool checkattr = true, const char* ppass = nullptr) const
<br />
- bool Remove(const std::vector<float>& embedding, bool mainkeyonly = false, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr) const
- bool Remove(const float* pembedding, size_t emlength, bool mainkeyonly = false, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL) const
- bool Remove(const std::string& strKey, bool mainkeyonly = false) const
<br />
- bool Search(const std::vector<float>& embedding, std::string& res_key, std::vector<float>& res_em, std::vector<float>& res_sysem, std::string& res_content, std::string& res_model, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Search(const std::vector<float>& embedding, std::string& res_key, std::string& res_content, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const
- bool Search(const std::vector<float>& embedding, std::string& res_key, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const std::string* pstrmodel = nullptr, const std::vector<float>* psysembedding = nullptr, bool checkattr = true, const std::string* pstrpass = nullptr) const
- char* Search(const float* pembedding, size_t emlength, float score = K2hVecDb::USE_COMMON_SCORE_LIMIT, const char* pemmodel = nullptr, const float* psysembedding = nullptr, size_t sysemlength = 0UL, bool checkattr = true, const char* ppass = nullptr) const
<br />

#### Method Descriptions
- K2hVecDb  
  Constructor. Attaches directly to a K2Hash file (or in-memory storage) by specifying a pointer to a K2HShm object.
- ~K2hVecDb  
  Destructor. Detaches from K2Hash unless it was attached directly to a K2Hash file (or in-memory storage).
- Clean  
  Detaches from K2Hash, except when attached directly to a K2Hash file (or in-memory storage).
- IsAttached  
  Checks whether attached to K2Hash.
- SetK2hash  
  Attaches directly to a K2Hash file (or in-memory storage) by specifying a pointer to a K2HShm object.
- CreateK2hash  
  Creates and attaches to a K2Hash database file used by the K2HVEC library.
- AttachK2hash  
  Attaches to (or creates) a K2Hash database file used by the K2HVEC library.
- AttachK2hashMem  
  Creates and attaches to an in-memory K2Hash database used by the K2HVEC library.
- SetCommonAttribute  
  Configures settings for update timestamps, encryption passphrases (including whether to set it as the default passphrase), data update history usage, and expiration time (in seconds) for K2Hash.
- AddAttrCryptPass  
  Configures the encryption passphrase for K2Hash and specifies whether to set it as the default passphrase.
- CleanCommonAttribute  
  Resets K2Hash attributes (update timestamp usage, encryption passphrase settings, and expiration time) to their default values.
- GetCommonAttributeInformation  
  Retrieves K2Hash attribute information (update timestamp usage, encryption passphrase settings, and expiration time) as a string.
- Set  
  Sets a value in K2HVEC (K2Hash) by specifying a vector embedding (floating-point array) or a K2Hash key name.
- Get  
  Retrieves a value by specifying a vector embedding (floating-point array) or a K2Hash key name.
- Remove  
  Deletes data by specifying a vector embedding (floating-point array) or a K2Hash key name.
- Search  
  Performs a vector similarity search using a vector embedding (floating-point array) and retrieves the values.

#### Method Return Values
Refer to each function prototype.
For prototypes returning a bool value, true is returned on success, and false is returned on failure.

#### Note
Refer to the corresponding C API functions for the parameters of each method.

#### Examples
 ```
K2hVec k2hvecobj;
if(!k2hvecobj.AttachK2hashMem()){
    exit(EXIT_FAILURE);
}

std::vector<float> embedding = {...};
std::string        content   = "...";
if(!k2hvec.Set(embedding. content)){
    exit(EXIT_FAILURE);
}

std::vector<float> sysem;
std::string        model;
std::string        strKeyContent;
std::string        strContent;
if(!k2hvec.Get(strKeyContent, embedding, sysem, strContent, model)){
    exit(EXIT_FAILURE);
}

std::vector<float> tgEmbedding = {...};
std::string        strFoundKey;
std::vector<float> strFoundEmbedding;
std::vector<float> FoundSysEmbedding;
std::string        strFoundContent;
std::string        strFoundModel;
if(!k2hvec.Search(tgEmbedding, strFoundKey, strFoundEmbedding, FoundSysEmbedding, strFoundContent, strFoundModel)){
    exit(EXIT_FAILURE);
    exit(-1);
}

if(!k2hvec.Remove(strFoundKey)){
    exit(EXIT_FAILURE);
}
exit(EXIT_SUCCESS);
 ```
