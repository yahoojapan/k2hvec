---
layout: contents
language: ja
title: Overview
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: home.html
lang_opp_word: To English
prev_url: 
prev_string: 
top_url: indexja.html
top_string: TOP
next_url: featureja.html
next_string: Feature
---

# K2HVEC
**K2HVEC** は、Key-Value Store（KVS）ライブラリ [K2HASH](https://github.com/yahoojapan/k2hash) を基盤として構築された、高性能なKVSベースのベクトルデータベースです。  
AIや機械学習モデルによって生成されたベクトル埋め込み（浮動小数点数の配列）をキーとして任意のデータを格納し、高速かつ高精度なベクトル類似度検索を実現します。

[K2HASH](https://github.com/yahoojapan/k2hash) が持つ優れた特徴（機能・性能・各種属性）をすべて継承しているため、暗号化や有効期限（TTL）管理などを備えた安全かつ高機能なベクトル検索環境を提供します。

## 背景・開発目的
近年、AI（LLM等）の活用が急速に広まる一方で、API呼び出しにおける**レスポンス遅延**やリクエスト増加に伴う**コスト**が重要視されています。  

この課題を解決するため、AIが生成した「ベクトル埋め込み（浮動小数点数の配列）」とそれに対応する「応答結果（任意データ）」をペアで一時的または恒久的に保存・キャッシュし、高速なベクトル類似度検索によって過去のレスポンスを素早く再利用するアプローチを考案しました。
このシステムを実現するにあたり、以下の要件を満たす必要がありました。

- **大容量・可変長データの高速処理:**  
返却されるデータ（値）はサイズが大きく可変であるため、大容量データの保持と高速な読み書き能力が求められる。
- **エンタープライズレベルのデータ管理機能:**  
保存データに対するセキュリティ（暗号化）や、一定期間でデータを自動削除・更新する仕組み（有効期限 / TTL）が不可欠である。

そこで、すでに実績のある高機能KVS [K2HASH](https://github.com/yahoojapan/k2hash) を基盤として活用することで、大容量データの高速な入出力と属性管理（暗号化・TTL等）を継承した条件を満たすベクトルデータベースとして **K2HVEC**（Vector Database and Search built on K2HASH）を開発しました。
