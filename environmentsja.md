---
layout: contents
language: ja
title: Environments
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: environments.html
lang_opp_word: To English
prev_url: developerja.html
prev_string: Developer
top_url: indexja.html
top_string: TOP
next_url: toolsja.html
next_string: Tools
---

# 環境変数
**K2HVEC** ライブラリは、以下の環境変数を読み込みます。
- **VECDBGMODE**  
K2HVECライブラリのログ出力レベルを指定します。 指定できる値は、`silent` / `err` / `wan` / `info` / `dump` です。設定したレベルに応じたログメッセージが出力されます。
- **VECDBGFILE**  
K2HVEC ライブラリのログ出力先となるファイルパスを指定します。

※ C/C++ 言語の API から直接ログ設定が行われた場合は、API での設定が優先されます。
