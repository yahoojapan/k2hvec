---
layout: contents
language: ja
title: Tools
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: tools_k2hvecpmtool.html
lang_opp_word: To English
prev_url:
prev_string:
top_url: toolsja.html
top_string: Tools
next_url: 
next_string: 
---

# k2hvecpmtool
データの保管や類似度検索の内部処理で使用する Projection Matrix（射影行列）の生成・更新・読み込みを行うための専用ツールです。

## ヘルプ
```
$ k2hvecpmtool --help

Generate Projection Matrix data tool

Usage: k2hvecpmtool --bit_size(-bs) <number> --dimension_size(-ds) <number> --name(-n) <string> --version(-v) <string> --output(-o) <file path> --debug(-d) --sourcecode(-s)
       k2hvecpmtool --help(-h)

Options:
  --help(-h)                      display this usage.
  --bit_size(-bs) <number>        specify the number of bits(size) in the range of 16 to 512(default: 48)
  --dimension_size(-ds) <number>  specify the number of dimension in the range of 512 to 8192(default: 4096)
  --name(-n) <string>             specify Projection Matrix array name(default: "pm")
  --version(-v) <string>          specify Projection Matrix array version(default: "1.0")
  --output(-o) <file path>        specify output file(this option is not specified, put stdout)
  --debug(-d)                     print debug messages
  --sourcecode(-s)                specify output format for source code(use internal developer only)
```

## k2hvecpmtoolオプション
k2hvecpmtoolの起動オプションを説明します。

| オプション                      | 説明 |
| ------------------------------- | ---- |
| \--help(-h)                     | 使用方法（Usage）およびオプション一覧を表示します |
| \--bit_size(-bs) <number>       | バイナリハッシュのビット数（サイズ）を 16 から 512 の範囲で指定します（デフォルト: `48`） |
| \--dimension_size(-ds) <number> | ベクトルの次元数を 512 から 8192 の範囲で指定します（デフォルト: `4096`） |
| \--name(-n) <string>            | Projection Matrix（射影行列）の配列名を指定します（デフォルト: `"pm"`） |
| \--version(-v) <string>         | Projection Matrix（射影行列）の配列バージョンを指定します（デフォルト: `"1.0"`） |
| \--output(-o) <file path>       | 出力先のファイルパスを指定します（未指定の場合は標準出力へ出力） |
| \--debug(-d)                    | デバッグ用の詳細メッセージ画面に出力します |
| \--sourcecode(-s)               | ソースコード形式の出力フォーマットを指定します（※ 内部開発者専用の隠しオプション） |
