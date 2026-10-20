---
layout: contents
language: ja
title: Build
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: build.html
lang_opp_word: To English
prev_url: usageja.html
prev_string: Usage
top_url: indexja.html
top_string: TOP
next_url: developerja.html
next_string: Developer
---

# ビルド
**K2HVEC** のビルドは以下の手順で行います。

1. ビルド環境の構築
2. ソースコードからの **K2HVEC** のビルド
3. **K2HVEC** のインストール
4. 動作確認

## 1. ビルド環境の構築
**K2HVEC** のビルドには、[K2HASH](https://k2hash.antpick.ax/indexja.html) をはじめとする依存ライブラリおよびヘッダーファイルが必要です。  
これらを用意する方法として、以下の 2 通りの手順を提供しています。環境に合わせて選択してください。

- **packagecloud.io からパッケージをインストール**  
[packagecloud.io](https://packagecloud.io/antpickax/stable) からビルド済みのパッケージを利用してインストールを行います。ソースからのビルドは不要です。
- **GitHub からソースコードを取得してビルド**  
[GitHub](https://github.com/yahoojapan) から依存ライブラリのソースコードを取得し、手動でビルドおよびインストールを行います。

### packagecloud.io から各依存ライブラリとヘッダーファイルをインストール
[packagecloud.io - AntPickax stable repository](https://packagecloud.io/antpickax/stable) から K2HASH ライブラリおよびヘッダーファイルをインストールする方法については、以下を参照してください。
- [K2HASH のインストール方法](https://k2hash.antpick.ax/usageja.html)

### GitHub から各依存ライブラリとヘッダーファイルをインストール
手動ビルドの詳細については、以下のドキュメントを参照してください。
- [K2HASH のビルド方法](https://k2hash.antpick.ax/buildja.html)

### その他の依存パッケージのインストール
ビルドを実行する Linux ディストリビューションに応じて、開発に必要な各種パッケージをあらかじめインストールしてください。  
必要パッケージの詳細は [ostypevars.sh](https://github.com/yahoojapan/k2hvec/blob/master/.github/workflows/ostypevars.sh) を参照してください。

## 2. ソースコードからの K2HVEC のビルド
以下は、**K2HVEC** をソースコードからビルドする手順です。

### ソースコードの取得
GitHub から [K2HVEC ソースコード](https://github.com/yahoojapan/k2hvec) をクローン（またはダウンロード）します。

```
$ git clone https://github.com/yahoojapan/k2hvec.git
```

### ビルドの実行
[GNU Automake](https://www.gnu.org/software/automake/) を使用して **K2HVEC** をビルドします。  
Debian ベースの Linux システムをご利用の場合は、以下のコマンドを実行してください。
```
$ git clone https://github.com/yahoojapan/k2hvec.git
$ cd k2hvec
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

## 3. K2HVEC のインストール
ビルドが正常に完了したら、以下の手順でインストールを行います。
```
$ cd k2hvec
$ sudo make install
```

## 4. 動作確認
**K2HVEC** が正常にインストールされたか確認するには、付属の **k2hveclinetool** または **k2hvecpmtool** を実行して動作を確認してください。
```
$ k2hveclinetool -h

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

Binary Hash and Index keys Options:
    --binary_hash_bit_size(-bs) <bit>     Specifies the number of bits for the binary hash calculation result
    --neighbor_key_limit(-nkl) <limit>    Specifies the maximum number of neighbor keys allowed for vector similarity searches

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
    search_config(sc) <...>               Set or Print Binary Hash bit count and the maximum number of neighbor keys.
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
