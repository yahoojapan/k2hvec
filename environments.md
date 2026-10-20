---
layout: contents
language: en-us
title: Environments
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: environmentsja.html
lang_opp_word: To Japanese
prev_url: developer.html
prev_string: Developer
top_url: index.html
top_string: TOP
next_url: tools.html
next_string: Tools
---

# Environment
The **K2HVEC** library reads the following environment variables:

- **VECDBGMODE**  
Specifies the log output level for the K2HVEC library. Allowed values are `silent`, `err`, `wan`, `info`, and `dump`. Messages are logged according to the specified level.
- **VECDBGFILE**  
Specifies the destination file path for the K2HVEC library log output.

**Note:** If logging configurations are set directly via the C/C++ API, the API settings will take precedence.
