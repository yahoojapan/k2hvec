---
layout: contents
language: ja
title: Tools
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: tools_k2hveclinetool.html
lang_opp_word: To English
prev_url:
prev_string:
top_url: toolsja.html
top_string: Tools
next_url: 
next_string: 
---

# k2hveclinetool
データベースの操作やベクトル検索を直感的に実行・テストできるコマンドラインツール（対話型 / コマンド実行）です。  
K2HVEC ライブラリのほぼすべての機能を対話形式で操作・検証できるほか、コマンドファイルを読み込んで自動実行（バッチ処理）することも可能です。

本セクションでは、起動オプションについて解説します。

- **k2hveclinetool の起動オプション**  
ツール起動時に指定可能な各種コマンドラインオプションについて説明します。
- **[k2hveclinetool のコマンド（プロンプト入力コマンド）**  
対話型プロンプト内で実行可能なコマンドについて説明します。

## k2hveclinetool の起動オプション
**k2hveclinetool** は対話型のツールであり、起動するとコマンド入力用プロンプトが表示されます。  
起動オプションに **--help**（または **-h**）を指定することで、利用可能な全オプションのヘルプメッセージを確認できます。

### ヘルプ
```
$ k2hveclinetool --help

Usage: k2hveclinetool [options]
       k2hveclinetool [options] -- <prompt commands>

Process Options:
    --help(-h)                            Print help
    --library_version(-libver)            Displays the library version
    --history_max(-hmax) <count>          Set maximum count for prompt command history
    --lap_time(-lap)                      Enable the display of the elapsed time for command execution
    --no_welcome_message(-no_welcome)     Suppress startup message
    --run_file(-run) <file path>          Load a file containing prompt commands and execute those commands
    -- <prompt commands...>               Parameters following this option are executed as prompt commands

K2hash/K2hvec Options:
    --file(-f) <file path>                Specify the K2Hash file
    --temporary_file(-t)                  Create a K2Hash using a temporary file and use it
    --memory(-m)                          Create a K2Hash in memory and use it.
    --mask(-msk) <bit count>              Specifies the number of mask bits when creating a K2Hash
    --collision_mask(-cmask) <bit count>  Specifies the number of collision mask(cmask) bits when creating a K2Hash
    --element_count(-elcnt) <count>       Specify the element count when creating a K2Hash
    --page_size(-page) <size>             Specifies the page size when creating a K2Hash
    --full_mapping(-full)                 K2Hash is fully mapping mode(in-memory K2Hash is enabled this as default. otherwise, the default is not)
    --read_only(-r)                       K2Hash file is used in read-only mode
    --debug_level(-d) <debug mode>        Specifies the debug level: SILENT(SLT), ERROR(ERR), WARNING(WAN), MESSAGE(MSG), or DUMP(DMP)
    --debug_logfile(-dlog) <file path>    Specifies the file for debug log output(default is stdout)

Common configuration(Binary hash / Index keys / Direct Content Limits):
    --binary_hash_bit_size(-bs) <bit>     Specifies the number of bits for the binary hash calculation result
    --neighbor_key_limit(-nkl) <limit>    Specifies the maximum number of neighbor keys allowed for vector similarity searches
    --content_direct_limit(-cdl) <limit>  Specifies the maximum size for saving content text directly to the main key

Prompt Commands:
    help(h) <command>                     Print command detail
    quit(q)                               Quit(exit) the dialogue and terminate the program (alternative command: exit(e))
    shell(sh)                             Escape to a shell (alternative command: !)
    echo <strings...>                     Print argument string (alternative command: print)
    sleep <second>                        Wait second (alternative command: wait)

    library_version(libver)               Displays the library version (alternative command: libversion)
    history_max(hmax) <count>             Set maximum count for prompt command history (alternative command: hismax)
    history(his)                          Display all command history (alternative command: history_log)
    save_history(save) <file>             Save all command history to the file (alternative command: save_his)
    debug_level(d) <debug level>          Specifies the debug level (alternative command: debug)
    debug_logfile(dlog) <file>            Specifies the file for debug log output (alternative command: debug_log)

    common_attribute(attr) <...>          Set and Print common expire and passphrase for data in k2hash.
    common_config(cc) <...>               Set or Print Binary Hash bit count, neighbor keys limit and content direct limit.
    last_status($?)                       Print the exit status of the previous prompt command(0: success, 1: failure)
    lap_time(lap) <"on" | "off"> <...>    Toggle disable/enable the display of the elapsed time for command execution
    run <file>                            Load a file containing prompt commands and execute those commands
    break                                 Specify stop to run even if an error occurs while executing the prompt commands(default)
    nobreak                               Specify continue to run even if an error occurs while executing the prompt commands

    load_projection_matrix(pm) <file>     Load and set the Projection Matrix data
    load_embedding(em) <file>             Load and set the Embedding data as Preloaded data
    create_projection_matrix(cpm) <...>   Create new Projection Matrix, and either load it or save it to file (alternative command: create_pm)
    create_embedding(cem) <...>           Create embedding data, and either load it as preloaded data or save it to file (alternative command: create_em)
    information(i) <pm | em>              Display information of Projection Matrix or preloaded embeddings data (alternative command: info, status)
    dump(dmp) <pm | em> <...>             Dump and save projection matrix or preloaded embeddings data
    create_k2hash_key(key) <file>         Create and print a K2Hash key for embedding data (alternative command: k2h_key, k2hkey)
    calculate_binary_hash(cbh) <...>      Calculate Binary hash value (alternative command: calculate_bh, cal_bh)

    set(s) <...>                          Set embedding data to K2Hash (alternative command: push, put)
    get(g) <...>                          Get raw embedding data from K2Hash to file (alternative command: print, p)
    remove(rm) <...>                      Remove embedding data in K2Hash (alternative command: delete, del)
    read_summary(r) <...>                 Read summary embedding data in K2Hash (alternative command: read)
    search(find) <...>                    Run vector search for the embedding data in K2Hash and find the closest data (alternative command: f)
    calculate_cos(cos) <key> <...>        Calculate Cosine Similarity value (alternative command: calculate_cosin_similarity, cal_cos)

  *) Please use "help <command>" for details about each command.
```

### オプション説明

| オプション                            | 説明 |
| ------------------------------------- | ---- |
| \--help(-h)                           | 起動オプションのヘルプメッセージを表示します |
| \--library_version(-libver)           | K2HVEC ライブラリおよび関連ライブラリのバージョンを表示します |
| \--history_max(-hmax) <count>         | プロンプトにおけるコマンド履歴の最大保持数を設定します |
| \--lap_time(-lap)                     | 各コマンド実行にかかった経過時間の表示 / 非表示を設定します |
| \--no_welcome_message(-no_welcome)    | k2hveclinetool 起動時の初期メッセージ（バナー）の表示を無効化します |
| \--run_file(-run) <file path>         | 指定したコマンドファイル（プロンプトコマンドを記述したファイル）を読み込み、自動実行（バッチ処理）します |
| \-- <prompt commands...>              | 本オプションに続けて指定した引数を、プロンプトコマンドとして直接実行します |
| \--file(-f) <file path>               | K2HVEC ライブラリが使用する K2Hash データベースファイルのパスを指定します |
| \--temporary_file(-t)                 | K2HVEC ライブラリが使用する K2Hash ファイルとして、自動削除される一時ファイル（Temporary file）を使用します |
| \--memory(-m)                         | K2HVEC ライブラリが使用する K2Hash データベースをメモリ上に構築（In-Memory）します |
| \--mask(-msk) <bit count>             | K2Hash データベース作成時のマスクビット（mask bit）数を指定します |
| \--collision_mask(-cmask) <bit count> | K2Hash データベース作成時の衝突マスク（cmask）ビット数を指定します |
| \--element_count(-elcnt) <count>      | K2Hash データベース作成時の初期要素数（element count）を指定します |
| \--page_size(-page) <size>            | K2Hash データベース作成時のページサイズ（page size）を指定します |
| \--full_mapping(-full)                | K2Hash ファイル（またはメモリ）をフルマッピングモードで開きます（インメモリの場合はデフォルトで有効、ファイルの場合はデフォルトで無効） |
| \--read_only(-r)                      | K2Hash データベースを読み取り専用（Read-Only）モードで開きます |
| \--debug_level(-d) <debug mode>       | デバッグ出力のログレベルを指定します（指定可能値: `SILENT` / `SLT`、`ERROR` / `ERR`、`WARNING` / `WAN`、`MESSAGE` / `MSG`、`DUMP` / `DMP`） |
| \--debug_logfile(-dlog) <file path>   | デバッグログの出力先ファイルパスを指定します（未指定時は標準エラー/標準出力に出力） |
| \--binary_hash_bit_size(-bs) <bit>    | 計算するバイナリハッシュ結果のビット数を指定します |
| \--neighbor_key_limit(-nkl) <limit>   | ベクトル類似度検索で許可する近傍キーの最大数を指定します |
| \--content_direct_limit(-cdl) <limit> | コンテンツテキストをメインキーに直接保存する際の最大サイズを設定します |

## k2hveclinetool コマンド（プロンプト入力コマンド）
**k2hveclinetool** の対話型プロンプト内で使用できる各種コマンドについて解説します。

### ヘルプ
```
K2HVEC > help

Prompt Commands:
    help(h) <command>                     Print command detail
    quit(q)                               Quit(exit) the dialogue and terminate the program (alternative command: exit(e))
    shell(sh)                             Escape to a shell (alternative command: !)
    echo <strings...>                     Print argument string (alternative command: print)
    sleep <second>                        Wait second (alternative command: wait)

    library_version(libver)               Displays the library version (alternative command: libversion)
    history_max(hmax) <count>             Set maximum count for prompt command history (alternative command: hismax)
    history(his)                          Display all command history (alternative command: history_log)
    save_history(save) <file>             Save all command history to the file (alternative command: save_his)
    debug_level(d) <debug level>          Specifies the debug level (alternative command: debug)
    debug_logfile(dlog) <file>            Specifies the file for debug log output (alternative command: debug_log)

    common_attribute(attr) <...>          Set and Print common expire and passphrase for data in k2hash.
    common_config(cc) <...>               Set or Print Binary Hash bit count, neighbor keys limit and content direct limit.
    last_status($?)                       Print the exit status of the previous prompt command(0: success, 1: failure)
    lap_time(lap) <"on" | "off"> <...>    Toggle disable/enable the display of the elapsed time for command execution
    run <file>                            Load a file containing prompt commands and execute those commands
    break                                 Specify stop to run even if an error occurs while executing the prompt commands(default)
    nobreak                               Specify continue to run even if an error occurs while executing the prompt commands

    load_projection_matrix(pm) <file>     Load and set the Projection Matrix data
    load_embedding(em) <file>             Load and set the Embedding data as Preloaded data
    create_projection_matrix(cpm) <...>   Create new Projection Matrix, and either load it or save it to file (alternative command: create_pm)
    create_embedding(cem) <...>           Create embedding data, and either load it as preloaded data or save it to file (alternative command: create_em)
    information(i) <pm | em>              Display information of Projection Matrix or preloaded embeddings data (alternative command: info, status)
    dump(dmp) <pm | em> <...>             Dump and save projection matrix or preloaded embeddings data
    create_k2hash_key(key) <file>         Create and print a K2Hash key for embedding data (alternative command: k2h_key, k2hkey)
    calculate_binary_hash(cbh) <...>      Calculate Binary hash value (alternative command: calculate_bh, cal_bh)

    set(s) <...>                          Set embedding data to K2Hash (alternative command: push, put)
    get(g) <...>                          Get raw embedding data from K2Hash to file (alternative command: print, p)
    remove(rm) <...>                      Remove embedding data in K2Hash (alternative command: delete, del)
    read_summary(r) <...>                 Read summary embedding data in K2Hash (alternative command: read)
    search(find) <...>                    Run vector search for the embedding data in K2Hash and find the closest data (alternative command: f)
    calculate_cos(cos) <key> <...>        Calculate Cosine Similarity value (alternative command: calculate_cosin_similarity, cal_cos)

  *) Please use "help <command>" for details about each command.
```

## プロンプトコマンド説明

| コマンド                              | 説明 |
| ------------------------------------- | ---- |
| help(h) <command>                     | 指定したコマンドの詳細情報（ヘルプ）を表示します |
| quit(q)                               | 対話モードを終了し、プログラムを終了します（エイリアス: `exit`, `e`） |
| shell(sh)                             | 一時的にシステムシェルへ切り替えます（エイリアス: `!`） |
| echo <strings...>                     | 指定した引数文字列を画面に出力します（エイリアス: `print`） |
| sleep <second>                        | 指定した秒数だけ処理を一時停止します（エイリアス: `wait`） |
| library_version(libver)               | 利用中のK2HVECライブラリおよび依存ライブラリのバージョンを表示します（エイリアス: `libversion`） |
| history_max(hmax) <count>             | プロンプトのコマンド履歴の最大保持数を設定します（エイリアス: `hismax`） |
| history(his)                          | これまでに実行したすべてのコマンド履歴を表示します（エイリアス: `history_log`） |
| save_history(save) <file>             | すべてのコマンド履歴をファイルへ保存します（エイリアス: `save_his`） |
| debug_level(d) <debug level>          | ログのデバッグレベルを設定・変更します（エイリアス: `debug`） |
| debug_logfile(dlog) <file>            | デバッグログを出力するファイルパスを指定します（エイリアス: `debug_log`） |
| common_attribute(attr) <...>          | 保存するデータ（K2Hash 内データ）に適用する共通の有効期限（Expire）およびパスフレーズを設定・表示します |
| common_config(cc) <...>               | バイナリハッシュのビット数、ベクトル検索インデックスキー、コンテンツテキストをメインキーに直接保存する際の最大サイズの設定を閲覧・更新します |
| search_config(sc) <...>               | ベクトル類似度検索で許可する近傍キーの最大数を設定・表示します |
| last_status($?)                       | 直前に実行したプロンプトコマンドのステータスコードを表示します（`0`: 成功, `1`: 失敗） |
| lap_time(lap) <"on" \| "off"> <...>   | コマンド実行にかかった経過時間（Elapsed time）の表示 / 非表示を切り替えます |
| run <file>                            | プロンプトコマンドが記述されたファイルを読み込み、バッチ実行します |
| break                                 | スクリプト実行中にエラーが発生した場合、処理を中断します（デフォルト） |
| nobreak                               | スクリプト実行中にエラーが発生した場合でも、後続の処理を継続します |
| load_projection_matrix(pm) <file>     | Projection Matrix（射影行列）データを読み込んでツールのメモリに設定します |
| load_embedding(em) <file>             | 事前ロードデータ（Preloaded data）としてベクトル埋め込み（浮動小数点数の配列）を読み込んで設定します |
| create_projection_matrix(cpm) <...>   | 新しい Projection Matrix（射影行列） を生成し、メモリへ読み込むかファイルへ保存します（エイリアス: `create_pm`） |
| create_embedding(cem) <...>           | 新しいベクトル埋め込み（浮動小数点数の配列）を生成し、事前ロードデータとして読み込むかファイルへ保存します（エイリアス: `create_em`） |
| information(i) <pm \| em>             | Projection Matrix（射影行列） または事前ロード済みベクトル埋め込み（浮動小数点数の配列）の詳細情報を表示します（エイリアス: `info`, `status`） |
| dump(dmp) <pm \| em> <...>            | Projection Matrix（射影行列） または事前ロード済みベクトル埋め込み（浮動小数点数の配列）をダンプし、ファイルへ保存します |
| create_k2hash_key(key) <file>         | ベクトル埋め込み（浮動小数点数の配列）用の K2Hash キーを生成して画面に表示します（エイリアス: `k2h_key`, `k2hkey`） |
| calculate_binary_hash(cbh) <...>      | 入力データに対するバイナリハッシュ値を計算して表示します（エイリアス: `calculate_bh`, `cal_bh`） |
| set(s) <...>                          | K2Hash にベクトル埋め込み（浮動小数点数の配列）を登録・更新します（エイリアス: `push`, `put`） |
| get(g) <...>                          | K2Hash からベクトル埋め込み（浮動小数点数の配列）を取得し、ファイルへ出力します（エイリアス: `print`, `p`） |
| remove(rm) <...>                      | K2Hash 内の指定されたベクトル埋め込み（浮動小数点数の配列）を削除します（エイリアス: `delete`, `del`） |
| read_summary(r) <...>                 | K2Hash からベクトル埋め込み（浮動小数点数の配列）のサマリー情報を読み込みます（エイリアス: `read`） |
| search(find) <...>                    | K2Hash 内のベクトル埋め込み（浮動小数点数の配列）に対してベクトル検索を実行し、最も類似度の高いデータを検索します（エイリアス: `f`） |
| calculate_cos(cos) <key> <...>        | 2つのベクトル間のコサイン類似度（Cosine Similarity）を計算します（エイリアス: `calculate_cosin_similarity`, `cal_cos`） |
