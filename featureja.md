---
layout: contents
language: ja
title: Feature
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: feature.html
lang_opp_word: To English
prev_url: homeja.html
prev_string: Overview
top_url: indexja.html
top_string: TOP
next_url: usageja.html
next_string: Usage
---

# 特徴

## インストール
K2HVEC は、主要な Linux ディストリビューション（Debian、Ubuntu、Rocky Linux、Fedora、Alpine Linux）向けに公式パッケージを提供しています。
[packagecloud.io](https://packagecloud.io/antpickax/stable) からソースコードをビルドすることなく簡単にインストール可能です。

上記以外の OS や環境であっても、ソースコードから[ビルド](https://k2hvec.antpick.ax/buildja.html)して利用できます。

## キーとデータの柔軟性
ベクトル埋め込み（Embedding）をキーとし、それに関連付けられた任意のユーザーデータ（Value）をペアとして格納・保持します。

- **長さ制限なし:**  
ベクトル埋め込みの配列長（次元数）およびユーザーデータのサイズに制限はありません。
- **保存形式の選択:**  
用途に応じて、オンメモリ（高速・揮発性）またはファイル（大容量・永続性）へのデータ保管を選択できます。

## 高速なベクトル検索
入力されたベクトル埋め込み（Embedding）を用いて、高速かつ正確なベクトル類似度検索を実行します。

## K2HASH 機能のフル継承
基盤である [K2HASH](https://github.com/yahoojapan/k2hash) のネイティブ機能をすべて継承しています。
格納するユーザーデータに対して、**データの暗号化**、**有効期限（TTL）設定**、**カスタム属性の付与** などをそのまま適用できます。

## 付属ツール
K2HVEC は C/C++ ライブラリとして提供され、各種 API を通じて機能を呼び出します。また、ライブラリ本体に加えて以下のツールを提供しています。

- **CLI ツール（対話型 / コマンド実行）**  
データベースの操作やベクトル検索を、外部から直感的に実行・テストできるコマンドラインツールです。
- **Projection Matrix（射影行列）管理ツール**  
データの保管や類似度検索の内部処理で使用する Projection Matrix（射影行列）の生成・読み込みを行うための専用ツールです。
