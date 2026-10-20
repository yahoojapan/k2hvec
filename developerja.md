---
layout: contents
language: ja
title: Developer
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: developer.html
lang_opp_word: To English
prev_url: buildja.html
prev_string: Build
top_url: indexja.html
top_string: TOP
next_url: environmentsja.html
next_string: Environments
---

<!-- =========================================================== -->
# 開発者向け

#### [C言語インタフェース](#CAPI)
[デバッグ関連(C I/F)](#DEBUG)  
[設定関連(C I/F)](#SETTING)  
[初期化関連(C I/F)](#INITIALIZATION)  
[属性関連(C I/F)](#ATTR)  
[操作関連(C I/F)](#OPERATION)  
#### [C++言語インタフェース](#CPP)
[K2hVecDbクラス](#K2HVECDB)  

<!-- =========================================================== -->
***

## <a name="CAPI"> C言語インタフェース
C言語用のインタフェースです。

開発時には以下のヘッダファイルをインクルードしてください。
 ```
#include <k2hvec/k2hvec.h>
 ```

リンク時には以下をオプションとして指定してください。
 ```
-lk2hvec
 ```

以下にC言語用の関数の説明をします。

<!-- =========================================================== -->
***

### <a name="DEBUG"> デバッグ関連(C I/F)
K2HVECライブラリは、内部動作およびAPIの動作の確認をするためにメッセージ出力を行うことができます。
本関数群は、メッセージ出力を制御するための関数群です。

#### 書式
- void k2hvec_bump_debug_level(void)
- void k2hvec_set_debug_level_silent(void)
- void k2hvec_set_debug_level_error(void)
- void k2hvec_set_debug_level_warning(void)
- void k2hvec_set_debug_level_message(void)
- void k2hvec_set_debug_level_dump(void)
- bool k2hvec_set_debug_file(const char* filepath)
- bool k2hvec_unset_debug_file(void)
- bool k2hvec_load_debug_env(void)

#### 説明
- k2hvec_bump_debug_level  
  メッセージ出力レベルを、非出力→エラー→ワーニング→インフォメーション→ダンプ→非出力・・・とBump upします。
- k2hvec_set_debug_level_silent  
  メッセージ出力レベルを、出力しないようにします。
- k2hvec_set_debug_level_error
  メッセージ出力レベルを、エラーにします。
- k2hvec_set_debug_level_warning  
  メッセージ出力レベルを、ワーニング以上にします。
- k2hvec_set_debug_level_message  
  メッセージ出力レベルを、インフォメーション以上にします。
- k2hvec_set_debug_level_dump  
  メッセージ出力レベルを、ダンプ以上にします。
- k2hvec_set_debug_file  
  メッセージを出力するファイルを指定します。設定されていない場合には stderr に出力します。
- k2hvec_unset_debug_file  
  メッセージを stderr に出力するように戻します。
- k2hvec_load_debug_env  
  環境変数 VECDBGMODE、VECDBGFILE を読み込み、その値にしたがってメッセージ出力、出力先を設定します。

#### 返り値
k2hvec_set_debug_file, k2hvec_unset_debug_file, k2hvec_load_debug_env は成功した場合には、true を返します。失敗した場合には false を返します。

#### 注意
環境変数 VECDBGMODE、VECDBGFILE については、[環境変数](environmentsja.html)を参照してください。

#### サンプル
 ```
k2hvec_unset_debug_file("/var/log/k2hvec/error.log");
k2hvec_set_debug_level_dump();
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="SETTING"> 設定関連(C I/F) 
K2HVEC ライブラリ全体の動作に関する共通パラメータ（基本設定）を設定する関数群です。

#### 書式
- bool k2hvec_set_default_score_limit(float val)
- void k2hvec_reset_default_score_limit(void)
- float k2hvec_get_default_score_limit(void)
- bool k2hvec_set_default_embedding_model(const char* pmodel)
- void k2hvec_reset_default_embedding_model(void)
- char* k2hvec_get_default_embedding_model(void)
- bool k2hvec_set_default_system_embedding(const float* pembedding, size_t length)
- void k2hvec_reset_default_system_embedding(void)
- float* k2hvec_get_default_system_embedding(size_t* plength)
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

#### 説明
- k2hvec_set_default_score_limit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値を設定します。
- k2hvec_reset_default_score_limit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値をリセットします。
- k2hvec_get_default_score_limit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値を取得します。
- k2hvec_set_default_embedding_model  
  ベクトル埋め込み（浮動小数点数配列）の共通モデル名を設定します。
- k2hvec_reset_default_embedding_model  
  ベクトル埋め込みの共通モデル名をリセットします。
- k2hvec_get_default_embedding_model  
  ベクトル埋め込みの共通モデル名を設定します。
- k2hvec_set_default_system_embedding  
  ベースとなる前提のベクトル埋め込み（システムモデル名）を設定します。
- k2hvec_reset_default_system_embedding  
  共通システムモデル名をリセットします。
- k2hvec_get_default_system_embedding  
  設定されている共通システムモデル名を取得します。
- k2hvec_set_default_expire  
  データ登録時に適用される共通の有効期限（秒数）を設定します。
- k2hvec_reset_default_expire  
  共通の有効期限設定をリセットします。
- k2hvec_get_default_expire  
  現在設定されている共通の有効期限（秒数）を取得します。
- k2hvec_set_default_passphrase  
  データを暗号化保存する際に使用する共通パスフレーズを設定します。
- k2hvec_reset_default_passphrase  
  暗号化用の共通パスフレーズをリセットします。
- k2hvec_get_default_passphrase  
  現在設定されている暗号化用の共通パスフレーズを取得します。
- k2hvec_set_default_binary_hash_size  
  ベクトル埋め込みから導出されるバイナリハッシュ値のビット長を指定します。
- k2hvec_reset_default_binary_hash_size  
  バイナリハッシュ値のビット長設定をリセットします。
- k2hvec_get_default_binary_hash_size  
  現在設定されているバイナリハッシュ値のビット長を取得します。
- k2hvec_set_default_neighbor_key_limit  
  ベクトル類似度検索で許可する近傍キーの最大数を設定します。
- k2hvec_reset_default_neighbor_key_limit  
  ベクトル類似度検索で許可する近傍キーの最大数をリセットします。
- k2hvec_get_default_neighbor_key_limit  
  ベクトル類似度検索で許可する近傍キーの最大数を取得します。
- k2hvec_set_content_direct_limit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズを設定します。
- k2hvec_reset_content_direct_limit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズをリセットします。
- k2hvec_get_content_direct_limit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズを取得します。

#### パラメータ
- k2hvec_set_default_score_limit
    - float val  
      コサイン類似度の共通下限値を指定します。
- k2hvec_set_default_embedding_model
    - const char* pmodel  
      共通モデル名を指定します。
- k2hvec_set_default_system_embedding
    - float* pembedding  
      ベースとなる前提のベクトル埋め込み（システムモデル名）の配列ポインターを指定します。
- k2hvec_set_default_system_embedding
    - size_t length  
      ベースとなる前提のベクトル埋め込み（システムモデル名）の配列長を指定します。
    - size_t* plength  
      取得するベースとなる前提のベクトル埋め込み（システムモデル名）の配列長が返されるバッファーを指定します。
- k2hvec_set_default_expire
    - time_t expire  
      共通の有効期限（秒数）を指定します。
- k2hvec_set_default_passphrase
    - const char* ppass  
      共通パスフレーズを指定します。
- k2hvec_set_default_binary_hash_size
    - int bitsize  
      バイナリハッシュ値のビット長設定を指定します。
- k2hvec_set_default_neighbor_key_limit
    - size_t limit  
      ベクトル類似度検索で許可する近傍キーの最大数を指定します。
- k2hvec_set_content_direct_limit
    - size_t limit  
      コンテンツテキストをメインキーに直接保存する際の最大サイズを指定します。

#### 返り値
- k2hvec_set_default_score_limit  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_default_score_limit  
  何も返しません。
- k2hvec_get_default_score_limit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値を返します。
- k2hvec_set_default_embedding_model  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_default_embedding_model  
  何も返しません。
- k2hvec_get_default_embedding_model  
  ベクトル埋め込みの共通モデル名を返します。
- k2hvec_set_default_system_embedding  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_default_system_embedding  
  何も返しません。
- k2hvec_get_default_system_embedding  
  ベースとなる前提のベクトル埋め込み（システムモデル名）の配列ポインターを返します。失敗した場合は、`NULL`を返します。返されたポインターは `free()` で解放する必要があります。
- k2hvec_set_default_expire  
  直前に設定されていた有効期限（秒数）を返します。
- k2hvec_reset_default_expire  
  何も返しません。
- k2hvec_get_default_expire  
  設定されている有効期限（秒数）を返します。
- k2hvec_set_default_passphrase  
  何も返しません。
- k2hvec_reset_default_passphrase  
  何も返しません。
- k2hvec_get_default_passphrase  
  設定されている共通パスフレーズを返します。返されたポインターは `free()` で解放する必要があります。
- k2hvec_set_default_binary_hash_size  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_default_binary_hash_size  
  何も返しません。
- k2hvec_get_default_binary_hash_size  
  設定されているバイナリハッシュ値のビット長を返します。
- k2hvec_set_default_neighbor_key_limit  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_default_neighbor_key_limit  
  何も返しません。
- k2hvec_get_default_neighbor_key_limit  
  設定されているベクトル類似度検索で許可する近傍キーの最大数を返します。
- k2hvec_set_content_direct_limit  
  成功した場合には、true を返します。失敗した場合には false を返します。
- k2hvec_reset_content_direct_limit  
  何も返しません。
- k2hvec_get_content_direct_limit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズを返します。

#### サンプル
 ```
if(k2hvec_set_default_score_limit(0.9f)){
    return false;
}

size_t limit = k2hvec_get_default_neighbor_key_limit();
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="INITIALIZATION"> 初期化関連(C I/F) 
K2HVEC ライブラリが使用する K2Hash（データベースファイルおよびインメモリ）の初期化に関する関数群です。

#### 書式
- k2hvec_handle_t k2hvec_create(const char* file, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach(const char* file, bool isReadOnly, bool isCreate, bool isTempFile, bool isfullmapping, int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach_memory(int mask_bitcnt, int cmask_bitcnt, int max_element_cnt, size_t pagesize)
- k2hvec_handle_t k2hvec_attach_k2hash(k2h_h k2hhandle)
- void k2hvec_delete(k2hvec_handle_t k2hvechandle)

#### 説明
- k2hvec_create  
  K2HVEC ライブラリが使用する K2Hash データベースファイルを作成し、アタッチします。
- k2hvec_attach  
  K2HVEC ライブラリが使用する K2Hash データベースファイルにアタッチ（もしくは生成）します。
- k2hvec_attach_memory  
  K2HVEC ライブラリが使用する K2Hash インメモリを作成し、アタッチします。
- k2hvec_attach_k2hash  
  K2HVEC ライブラリが使用する K2Hash ハンドルに直接アタッチします。
- k2hvec_delete  
  K2Hash ハンドルを破棄します。

#### パラメータ
- k2hvec_create
    - const char* file  
      作成するK2Hash データベースファイルのパスを指定します。
    - bool isfullmapping  
      K2Hash データベースファイルをフルマッピングモードか部分マッピングモードを指定します。
    - int mask_bitcnt  
      K2Hash データベースファイルのmaskビット数を指定します。
    - int cmask_bitcnt  
      K2Hash データベースファイルのcmaskビット数を指定します。
    - int max_element_cnt  
      K2Hash データベースファイルのelement数を指定します。
    - size_t pagesize  
      K2Hash データベースファイルのpageサイズを指定します。
- k2hvec_attach
    - const char* file  
      アタッチするK2Hash データベースファイルのパスを指定します。
    - bool isReadOnly  
      アタッチするK2Hash データベースファイルを読み取り専用とするかどうかを指定します。
    - bool isCreate  
      アタッチするK2Hash データベースファイルを作成するか否かを指定します。
    - bool isTempFile  
      生成する場合に一時的なK2Hash データベースファイルを使用するように指定します。
    - bool isfullmapping  
      アタッチするK2Hash データベースファイルをフルマッピングモードか部分マッピングモードを指定します。
    - int mask_bitcnt  
      生成する場合にK2Hash データベースファイルのmaskビット数を指定します。
    - int cmask_bitcnt  
      生成する場合にK2Hash データベースファイルのcmaskビット数を指定します。
    - int max_element_cnt  
      生成する場合にK2Hash データベースファイルのelement数を指定します。
    - size_t pagesize  
      生成する場合にK2Hash データベースファイルのpageサイズを指定します。
- k2hvec_attach_memory
    - int mask_bitcnt  
      インメモリK2Hash データベースのmaskビット数を指定します。
    - int cmask_bitcnt  
      インメモリK2Hash データベースのcmaskビット数を指定します。
    - int max_element_cnt  
      インメモリK2Hash データベースのelement数を指定します。
    - size_t pagesize  
      インメモリK2Hash データベースのpageサイズを指定します。
- k2hvec_attach_k2hash
    - k2h_h k2hhandle  
      すでにオープンしているK2HVECハンドルを指定します。
- k2hvec_delete
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを破棄します。

#### 返り値
- k2hvec_create  
  成功した場合は、K2HVECハンドルを返します。失敗した場合は、`NULL`を返します。
- k2hvec_attach  
  成功した場合は、K2HVECハンドルを返します。失敗した場合は、`NULL`を返します。
- k2hvec_attach_memory  
  成功した場合は、K2HVECハンドルを返します。失敗した場合は、`NULL`を返します。
- k2hvec_attach_k2hash  
  成功した場合は、K2HVECハンドルを返します。失敗した場合は、`NULL`を返します。
- k2hvec_delete  
  何も返しません。

#### 注意
生成、アタッチした後に返されるK2HVECハンドル（`k2hvec_handle_t`）は、不要になったら`k2hvec_delete`で破棄してください。

#### サンプル
 ```
k2hvec_handle_t hk2hvec = k2hvec_create("/tmp/my.k2h", true, 4, 8, 8, 1024);
if(!hk2hvec){
    return false;
}

k2hvec_delete(hk2hvec);
 ```

<!-- ----------------------------------------------------------- -->
***

### <a name="ATTR"> 属性関連(C I/F) 
K2HVEC ライブラリ全体の属性に関する共通パラメータ（基本設定）を設定・参照する関数群です。

#### 書式
- bool k2hvec_set_attribute(k2hvec_handle_t k2hvechandle, const bool* is_mtime, const bool* is_defenc, const char* passfile, const bool* is_history, const time_t* expire)
- bool k2hvec_add_passphrase(k2hvec_handle_t k2hvechandle, const char* pass, bool is_default_encrypt)
- bool k2hvec_clear_attribute(k2hvec_handle_t k2hvechandle)

#### 説明
- k2hvec_set_attribute  
  K2Hashに更新時刻の利用設定、暗号化のパスフレーズの設定とその設定値をデフォルトのパスフレーズとするかどうか、データ更新履歴の利用可否、有効期限（秒数）の設定をします。
- k2hvec_add_passphrase  
  K2Hashに暗号化のパスフレーズの設定とその設定値をデフォルトのパスフレーズとするかどうかを設定します。
- k2hvec_clear_attribute  
  K2Hashの属性（更新時刻の利用設定、暗号化のパスフレーズの設定、有効期限）を初期値に戻します。

#### パラメータ
- k2hvec_set_attribute
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const bool* is_mtime  
      更新時刻の利用設定を指定したバッファを指定します。この設定をしない場合には、`NULL`を指定します。
    - const bool* is_defenc  
      設定する暗号化のパスフレーズをデフォルトとするか否かを示すバッファを指定します。この設定をしない場合には、`NULL`を指定します。
    - const char* passfile  
      設定する暗号化のパスフレーズを指定します。この設定をしない場合には、`NULL`を指定します。
    - const bool* is_history  
      データ更新履歴の利用可否を示すバッファを指定します。この設定をしない場合には、`NULL`を指定します。
    - const time_t* expire  
      有効期限（秒数）を示すバッファを指定します。この設定をしない場合には、`NULL`を指定します。
- k2hvec_add_passphrase
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const char* pass  
      設定する暗号化のパスフレーズを指定します。
    - bool is_default_encrypt  
      設定する暗号化のパスフレーズをデフォルトとするかどうかを指定します。
- k2hvec_clear_attribute
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。

#### 返り値
成功した場合は、true を返します。失敗した場合には false を返します。

#### サンプル
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

### <a name="OPERATION"> 操作関連(C I/F) 
K2HVEC（K2Hash）に対するデータの書き込み・読み出しなどの基本操作を行う関数群です。

#### 書式
- bool k2hvec_set(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pcontent, const char* pemmodel, const float* psysembedding, size_t sysemlength, const char* ppass, const time_t* pexpire)
- char* k2hvec_get_k2hash_key(const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength)
- float* k2hvec_get_embedding(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass, size_t* plength)
- char* k2hvec_get_content(k2hvec_handle_t k2hvechandle, const char* pkey, bool checkattr, const char* ppass)
- char* k2hvec_get_embedding_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
- bool k2hvec_remove_key(k2hvec_handle_t k2hvechandle, const char* pkey, bool mainkeyonly)
- bool k2hvec_remove_embedding(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, bool mainkeyonly, const char* pemmodel, const float* psysembedding, size_t sysemlength)
- char* k2hvec_search_key(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)
- char* k2hvec_search_content(k2hvec_handle_t k2hvechandle, const float* pembedding, size_t emlength, float score, const char* pemmodel, const float* psysembedding, size_t sysemlength, bool checkattr, const char* ppass)

#### 説明
- k2hvec_set  
  ベクトル埋め込み（浮動小数点数配列）と値をK2HVEC（K2Hash）に設定します。
- k2hvec_get_k2hash_key  
  ベクトル埋め込み（浮動小数点数配列）をK2Hashのキー名に変換します。
- k2hvec_get_embedding  
  K2Hashのキー名を指定して、ベクトル埋め込み（浮動小数点数配列）を取得します。
- k2hvec_get_content  
  K2Hashのキー名を指定して値を取得します。
- k2hvec_get_embedding_content  
  ベクトル埋め込み（浮動小数点数配列）を指定して値を取得します。
- k2hvec_remove_embedding  
  K2Hashのキー名を指定して削除します。
- k2hvec_remove_key  
  ベクトル埋め込み（浮動小数点数配列）を指定して削除します。
- k2hvec_search_key  
  ベクトル埋め込み（浮動小数点数配列）を指定してベクトル類似度検索し、キー名を取得します。
- k2hvec_search_content  
  ベクトル埋め込み（浮動小数点数配列）を指定してベクトル類似度検索し、値を取得します。

#### パラメータ
- k2hvec_set
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - const char* pcontent  
      値（コンテンツ）を指定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
    - const char* ppass  
      暗号化のパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。
    - const time_t* pexpire  
      設定するデータの有効期限を設定します。`NULL`を指定した場合は共通の有効期限が使われます。
- k2hvec_get_k2hash_key
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
- k2hvec_get_embedding
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const char* pkey  
      キー名を指定します。
    - bool checkattr  
      取得するときにK2Hashの属性のチェックをするか否かを指定します。
    - const char* ppass  
      復号するためのパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。
    - size_t* plength  
      取得するベクトル埋め込み（浮動小数点数配列）の長さを格納するバッファーを指定します。
- k2hvec_get_content
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const char* pkey  
      キー名を指定します。
    - bool checkattr  
      取得するときにK2Hashの属性のチェックをするか否かを指定します。
    - const char* ppass  
      復号するためのパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。
- k2hvec_get_embedding_content
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
    - bool checkattr  
      取得するときにK2Hashの属性のチェックをするか否かを指定します。
    - const char* ppass  
      復号するためのパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。
- k2hvec_remove_key
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const char* pkey  
      キー名を指定します。
    - bool mainkeyonly  
      メインのキーのみを取得するか否かを指定します。
- k2hvec_remove_embedding
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - bool mainkeyonly  
      メインのキーのみを削除するか否かを指定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
- k2hvec_search_key
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - float score  
      ベクトル類似度検索で使用するコサイン類似度の下限値を設定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
    - bool checkattr  
      取得するときにK2Hashの属性のチェックをするか否かを指定します。
    - const char* ppass  
      復号するためのパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。
- k2hvec_search_content
    - k2hvec_handle_t k2hvechandle  
      アタッチしているK2HVECハンドルを指定します。
    - const float* pembedding  
      ベクトル埋め込み（浮動小数点数配列）を指定します。
    - size_t emlength  
      ベクトル埋め込み（浮動小数点数配列）の長さを指定します。
    - float score  
      ベクトル類似度検索で使用するコサイン類似度の下限値を設定します。
    - const char* pemmodel  
      モデル名を指定します。`NULL`を指定した場合は共通モデル名が使われます。
    - const float* psysembedding  
      ベクトル埋め込み（システムモデル名）の配列を指定します。 `NULL`を指定した場合は共通ベクトル埋め込み（システムモデル名）が使われます。
    - size_t sysemlength  
      ベクトル埋め込み（システムモデル名）の配列長を指定します。
    - bool checkattr  
      取得するときにK2Hashの属性のチェックをするか否かを指定します。
    - const char* ppass  
      復号するためのパスフレーズを指定します。 `NULL`を指定した場合は共通パスフレーズが使われます。

#### 返り値
- k2hvec_set  
  成功した場合は、true を返します。失敗した場合には false を返します。
- k2hvec_get_k2hash_key  
  K2Hashキー名を返します。
- k2hvec_get_embedding  
  取得したベクトル埋め込み（浮動小数点数配列）へのポインターを返します。 失敗した場合は、`NULL`を返します。
- k2hvec_get_content  
  取得した値へのポインターを返します。 失敗した場合は、`NULL`を返します。
- k2hvec_get_embedding_content  
  取得した値へのポインターを返します。 失敗した場合は、`NULL`を返します。
- k2hvec_remove_embedding  
  成功した場合は、true を返します。失敗した場合には false を返します。
- k2hvec_remove_key  
  成功した場合は、true を返します。失敗した場合には false を返します。
- k2hvec_search_key  
  検出した値へのポインターを返します。 失敗した場合は、`NULL`を返します。
- k2hvec_search_content  
  検出した値へのポインターを返します。 失敗した場合は、`NULL`を返します。

#### サンプル
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

## <a name="CPP"> C++言語インタフェース
K2HVECライブラリを利用するための、C++言語APIのI/Fです。  

 
開発時には以下のヘッダファイルをインクルードしてください。
 ```
#include <k2hvec/k2hvec.h>
#include <k2hvec/k2hvecdb.h>
 ```
 
リンク時には以下をオプションとして指定してください。
 ```
-lk2hvec
 ```
 
以下にC++言語用の関数の説明をします。

<!-- =========================================================== -->
***

### <a name="K2HVECDB"> K2hVecDbクラス

#### 説明
K2HVEC データの操作および実装を行うクラスです。本クラスのインスタンスを通じて、K2HVEC データの操作を実行します。  
C++ 言語における一連のデータ操作は、本クラスのインスタンスを作成してから行います。  
インスタンスは、K2Hash ファイルまたはオンメモリ構造をオープン（アタッチ）して生成します。  
操作が完了した後は、必ずクローズ（デタッチ）してください。

#### クラスメソッド
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

#### クラスメソッド説明
- K2hVecDb::SetScoreLimit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値を設定します。
- K2hVecDb::ResetScoreLimit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値をリセットします。
- K2hVecDb::GetScoreLimit  
  ベクトル類似度検索で使用するコサイン類似度の共通下限値を取得します。
- K2hVecDb::SetEmbeddingModel  
  ベクトル埋め込み（浮動小数点数配列）の共通モデル名を設定します。
- K2hVecDb::ResetEmbeddingModel  
  ベクトル埋め込みの共通モデル名をリセットします。
- K2hVecDb::GetEmbeddingModel  
  ベクトル埋め込みの共通モデル名を取得します。
- K2hVecDb::SetSysEmbedding  
  ベースとなる前提のベクトル埋め込み（システムモデル名）を設定します。
- K2hVecDb::GetSysEmbedding  
  設定されている共通システムモデル名を取得します。
- K2hVecDb::ResetSysEmbedding  
  共通システムモデル名をリセットします。
- K2hVecDb::SetExpire  
  データ登録時に適用される共通の有効期限（秒数）を設定します。
- K2hVecDb::ResetExpire  
  共通の有効期限設定をリセットします。
- K2hVecDb::GetExpire  
  現在設定されている共通の有効期限（秒数）を取得します。
- K2hVecDb::SetCryptPass  
  データを暗号化保存する際に使用する共通パスフレーズを設定します。
- K2hVecDb::ResetCryptPass  
  暗号化用の共通パスフレーズをリセットします。
- K2hVecDb::GetCryptPass  
  現在設定されている暗号化用の共通パスフレーズを取得します。
- K2hVecDb::SetBinHashSize  
  ベクトル埋め込みから導出されるバイナリハッシュ値のビット長を指定します。
- K2hVecDb::ResetBinHashBitSize  
  バイナリハッシュ値のビット長設定をリセットします。
- K2hVecDb::GetBinHashSize  
  現在設定されているバイナリハッシュ値のビット長を取得します。
- K2hVecDb::SetNeighborKeysLimit  
  ベクトル類似度検索で許可する近傍キーの最大数を設定します。
- K2hVecDb::ResetNeighborKeysLimit  
  ベクトル類似度検索で許可する近傍キーの最大数をリセットします。
- K2hVecDb::GetNeighborKeysLimit  
  ベクトル類似度検索で許可する近傍キーの最大数を取得します。
- K2hVecDb::SetContentDirectLimit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズを設定します。
- K2hVecDb::ResetContentDirectLimit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズをリセットします。
- K2hVecDb::GetContentDirectLimit  
  コンテンツテキストをメインキーに直接保存する際の最大サイズを取得します。
- K2hVecDb::GetK2hashKey  
  ベクトル埋め込み（浮動小数点数配列）をK2Hashのキー名に変換します。

#### メソッド
- K2hVecDb(void)
- K2hVecDb(K2HShm* pk2h)
- ~K2hVecDb(void)
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

#### メソッド説明
- K2hVecDb  
  コンストラクターです。K2HShmオブジェクトのポインターを指定して、直接K2Hashファイル（インメモリ）に直接アタッチできます。
- ~K2hVecDb  
  デストラクターです。直接K2Hashファイル（インメモリ）に直接アタッチした場合を除き、K2Hashのデタッチもします。
- Clean  
  K2Hashからデタッチします。ただし直接K2Hashファイル（インメモリ）に直接アタッチした場合を除きます。
- IsAttached  
  K2Hashにアタッチしているか確認します。
- SetK2hash  
  K2HShmオブジェクトのポインターを指定して、直接K2Hashファイル（インメモリ）にアタッチします。
- CreateK2hash  
  K2HVEC ライブラリが使用する K2Hash データベースファイルを作成し、アタッチします。
- AttachK2hash  
  K2HVEC ライブラリが使用する K2Hash データベースファイルにアタッチ（もしくは生成）します。
- AttachK2hashMem  
  K2HVEC ライブラリが使用する K2Hash インメモリを作成し、アタッチします。
- SetCommonAttribute  
  K2Hashに更新時刻の利用設定、暗号化のパスフレーズの設定とその設定値をデフォルトのパスフレーズとするかどうか、データ更新履歴の利用可否、有効期限（秒数）の設定をします。
- AddAttrCryptPass  
  K2Hashに暗号化のパスフレーズの設定とその設定値をデフォルトのパスフレーズとするかどうかを設定します。
- CleanCommonAttribute  
  K2Hashの属性（更新時刻の利用設定、暗号化のパスフレーズの設定、有効期限）を初期値に戻します。
- GetCommonAttributeInformation  
  K2Hashの属性（更新時刻の利用設定、暗号化のパスフレーズの設定、有効期限）情報を文字列で取得します。
- Set  
  ベクトル埋め込み（浮動小数点数配列）もしくはK2Hashキー名を指定してと値をK2HVEC（K2Hash）に設定します。
- Get  
  ベクトル埋め込み（浮動小数点数配列）もしくはK2Hashキー名を指定して値を取得します。
- Remove  
  ベクトル埋め込み（浮動小数点数配列）もしくはK2Hashキー名を指定して削除します。
- Search  
  ベクトル埋め込み（浮動小数点数配列）を指定してベクトル類似度検索し、値を取得します。

#### メソッド返り値
個々のプロトタイプを参照してください。
bool値の返り値のプロトタイプは、成功した場合には、true を返します。失敗した場合には false を返します。

#### 注意
各メソッドの引数は、同様のC APIの関数を参照してください。

#### サンプル
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
