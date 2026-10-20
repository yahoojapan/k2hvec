---
layout: contents
language: ja
title: Usage
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: usage.html
lang_opp_word: To English
prev_url: featureja.html
prev_string: Feature
top_url: indexja.html
top_string: TOP
next_url: buildja.html
next_string: Build
---

# 使い方
K2HVECの実行環境を整え、動作確認する方法を示します。

## 1. 利用環境構築
**K2HVEC** をご利用の環境にインストールするには、2つの方法があります。  
ひとつは、[packagecloud.io](https://packagecloud.io/)から **K2HVEC** のパッケージをダウンロードし、インストールする方法です。  
もうひとつは、ご自身で **K2HVEC** をソースコードからビルドし、インストールする方法です。  
これらの方法について、以下に説明します。

### パッケージを使ったインストール
**K2HVEC** は、誰でも利用できるように[packagecloud.io - AntPickax stable repository](https://packagecloud.io/antpickax/stable/)で[パッケージ](https://packagecloud.io/app/antpickax/stable/search?q=k2hvec)を公開しています。  
**K2HVEC** のパッケージは、主要な Linux ディストリビューション（Debian、Ubuntu、Rocky Linux、Fedora、Alpine Linux）向けに Debianパッケージ、RPMパッケージ、APKパッケージで公開しています。  
お使いのOSによりインストール方法が異なりますので、以下の手順を確認してインストールしてください。  

#### DebianベースLinuxの利用者は、以下の手順に従ってください。
```
$ sudo apt-get update -y
$ sudo apt-get install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.deb.sh | sudo bash
$ sudo apt-get install k2hvec
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
$ sudo apt-get install k2hvec-dev
```

#### RPMベースのLinuxの場合は、以下の手順に従ってください。
```
$ sudo dnf makecache
$ sudo dnf install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo dnf install k2hvec
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
$ sudo dnf install k2hvec-devel
```

#### ALPINE Linuxの場合は、以下の手順に従ってください。
```
# apk update
# apk add curl
# curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.alpine.sh | sh
# apk add k2hvec
```
開発者向けパッケージをインストールする場合は、以下のパッケージをインストールしてください。
```
# apk add k2hvec-dev
```

#### 上記以外のOS
上述したOS以外をお使いの場合は、パッケージが準備されていないため、直接インストールすることはできません。  
この場合には、後述の[ソースコード](https://github.com/yahoojapan/k2hvec)からビルドし、インストールするようにしてください。

### ソースコードからビルド・インストール
**K2HASH** を[ソースコード](https://github.com/yahoojapan/k2hvec)からビルドし、インストールする方法は、[ビルド](https://k2hvec.antpick.ax/buildja.html)を参照してください。

## 2. k2hveclinetoolの実行
K2HVEC のインストール（またはソースからのビルド）が完了したら、付属の **k2hveclinetool** を実行して動作を確認してみましょう。  

### 実行
```
$ k2hveclinetool
```
_起動オプションは全てデフォルトのまま（未指定）で起動します。_

### コマンド：create_embedding/set/search
k2hveclinetool を実行すると、対話型のコマンドプロンプトが起動し、コマンドを対話形式で実行できます。  
（なお、コマンドファイルを指定して非対話形式で一括実行することも可能です。）  

以下の実行例では、**create_embedding**（ダミー Embedding データの生成）を実行したのち、**set** コマンドでデータを登録し、同じキー（Embedding）を使って **search** コマンドで類似度検索を行っています。
```
$ k2hveclinetool

-------------------------------------------------------------------------------
                  Command Line Tool for K2HVEC and K2HASH

      K2HVEC is a high-performance vector database for fast vector search,
                  built on top of the K2HASH key-value store.
-------------------------------------------------------------------------------
                           K2Hash Infomration

    Strage type                             : On memory
    Access mode                             : Read/Write
    Data mapping type                       : Full mapping
    Key Index Mask count(mask)              : 8
    Collision Key Index Mask count(cmask)   : 4
    Max Element count(element count)        : 32
    Page size                               : 128
-------------------------------------------------------------------------------
                     Projection Matrix in K2HVEC library

    Name                                    : builtin
    Version                                 : 1.0
    Dimension size                          : 4096
    Bit count(size)                         : 48 bit
-------------------------------------------------------------------------------
   Common configuration(Binary hash / Index keys / Direct Content Limits)

    Calaculate Binary hash bit size         : 24
    Search neighbor key upper limit         : 2600
    Upper limit for saving content directly : no limit
-------------------------------------------------------------------------------

K2HVEC > create_embedding
[SUCCEED] Generate and set it as Preloaded embedding data.
    Dimension size  : 4096
    Checksum        : 1.0f
    FNV-1a hash     : BA4214EC63F55A0A
K2HVEC > set
[SUCCEED] Set embedding data to k2hash
    Expire period time : not set
    Passphrase         : not set
    K2hash main key    : builtin@1.0/101111011001110011001000-BA4214EC63F55A0A
K2HVEC > search
Search Embedding data:
    Data source               : Preloaded embedding data
    Dimension size            : 4096
    Checksum                  : 1.0f
    FNV-1a hash               : BA4214EC63F55A0A
Search Score Limit            : 0.0f

-> Search Result              : Found
    Cosine Similarity value   : 1.0f

Found embedding data information:
    K2hash main key           : builtin@1.0/101111011001110011001000-BA4214EC63F55A0A
    Embedding        count    : 4096
                     checksum : 1.0f
                     2nd hash : BA4214EC63F55A0A
    Embedding model           : unknown_model
    Value length              : 0
          FNV-1a hash         : 0xCBF29CE484222325
          Content prefix      : <empty>
K2HVEC >
```

### その他のコマンド
**k2hveclinetool** の対話型プロンプトで **help** コマンドを実行すると、利用可能なコマンド一覧が表示されます。  
用途やテスト内容に応じて、必要なコマンドを実行してください。

### 終了方法
対話セッションを終了するには **quit**（または **exit**）コマンドを実行します。対話型プロンプトが終了し、**k2hveclinetool** が終了します。
