---
layout: contents
language: en-us
title: Feature
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: featureja.html
lang_opp_word: To Japanese
prev_url: home.html
prev_string: Overview
top_url: index.html
top_string: TOP
next_url: usage.html
next_string: Usage
---

# Feature

## Installation
K2HVEC provides official packages for major Linux distributions(Debian, Ubuntu, Rocky Linux, Fedora, and Alpine Linux).  
You can easily install it from [packagecloud.io](https://packagecloud.io/antpickax/stable) without building from source code.

For other operating systems and environments, you can [build from source](https://k2hvec.antpick.ax/build.html).

## Key-Value Flexibility
K2HVEC stores and retains pairs of vector embeddings(used as keys) and their associated arbitrary user data(values).

- **No Length Restrictions:**  
There are no limits on the array length(dimensions) of vector embeddings or the size of user data.
- **Flexible Storage Engine:**  
Depending on your use case, you can choose between in-memory storage(high-speed, volatile) or file-backed storage(large-capacity, persistent).

## Fast Vector Search
Performs high-speed and accurate vector similarity searches using specified input vector embeddings.

## Full Inheritance of K2HASH Features
Inherits all native capabilities of the underlying [K2HASH](https://github.com/yahoojapan/k2hash) engine.  
You can directly apply features such as **data encryption**, **expiration limits (TTL)**, and **custom attributes** to the stored user data.

## Bundled Tools
K2HVEC is provided as a C/C++ library, with its features accessible via APIs.
In addition to the core library, the following tools are included:

- **CLI Tool (Interactive / Command Execution):**  
A command-line tool for intuitively operating, testing, and performing vector searches on the database from an external interface.
- **Projection Matrix Management Tool:**  
A dedicated tool for generating and loading the Projection Matrix used internally during data storage and similarity search processing.
