---
layout: contents
language: en-us
title: Usage
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: usageja.html
lang_opp_word: To Japanese
prev_url: feature.html
prev_string: Feature
top_url: index.html
top_string: TOP
next_url: build.html
next_string: Build
---

# Usage
After building **K2HVEC**, you can check the operation by the following procedure.

## 1. Creating a usage environment
There are two ways to install **K2HVEC** in your environment.  
One is to download and install the package of **K2HVEC** from [packagecloud.io](https://packagecloud.io/).  
The other way is to build and install **K2HVEC** from source code yourself.  
These methods are described below.  

### Installing packages
The **K2HVEC** publishes [packages](https://packagecloud.io/app/antpickax/stable/search?q=k2hvec) in [packagecloud.io - AntPickax stable repository](https://packagecloud.io/antpickax/stable) so that anyone can use it.  
**K2HVEC** packages are available for major Linux distributions(Debian, Ubuntu, Rocky Linux, Fedora, and Alpine Linux) in Debian, RPM, and APK formats.  
Since the installation method differs depending on your OS, please check the following procedure and install it.  

#### For Debian-based Linux distributions users, follow the steps below:
```
$ sudo apt-get update -y
$ sudo apt-get install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.deb.sh | sudo bash
$ sudo apt-get install k2hvec
```
To install the developer package, please install the following package.
```
$ sudo apt-get install k2hvec-dev
```

#### For RPM-based Linux distributions users, follow the steps below:
```
$ sudo dnf makecache
$ sudo dnf install curl -y
$ curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.rpm.sh | sudo bash
$ sudo dnf install k2hvec
```
To install the developer package, please install the following package.
```
$ sudo dnf install k2hvec-devel
```

#### For ALPINE Linux distributions users, follow the steps below:
```
# apk update
# apk add curl
# curl -s https://packagecloud.io/install/repositories/antpickax/stable/script.alpine.sh | sh
# apk add k2hvec
```
To install the developer package, please install the following package.
```
# apk add k2hvec-dev
```

#### Other OS
If you are not using the above OS, packages are not prepared and can not be installed directly.  
In this case, build from the [source code](https://github.com/yahoojapan/k2hvec) described below and install it.

### Build and install from source code
For details on how to build and install **K2HVEC** from [source code](https://github.com/yahoojapan/k2hvec), please see [Build](https://k2hvec.antpick.ax/build.html).

## 2. Bundled Tool: k2hveclinetool
Once you have installed(or built from source) K2HVEC, try running the bundled **k2hveclinetool** to verify its operation.

### Run k2hveclinetool
```
$ k2hveclinetool
```
_The application starts with all startup options left at their defaults(unspecified)._

### Run commands: Create dummy embedding data / Set / Search
Running **k2hveclinetool** launches an interactive command prompt where you can execute commands interactively.  
(Non-interactive execution is also supported by passing a command file.)  

The following example demonstrates generating dummy embedding data with **create_embedding**, storing a entry using **set**, and then performing a similarity search using **search** with the same key (embedding).  
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

## 4. Other testing and exiting  
You can see interactive command list by "help" on prompt, and you can try to test those commands.  
If you want to quit k2hlinetool, you can input "quit" on prompt.


### Other Commands
Executing the **help** command at the **k2hveclinetool** interactive prompt will display a list of available commands.  
Please run any command you wish to test.

### Exiting
To end the interactive session, execute the **quit** (or **exit**) command. This will terminate the interactive prompt and exit **k2hveclinetool**.
