---
layout: contents
language: en-us
title: Tools
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: tools_k2hvecpmtoolja.html
lang_opp_word: To Japanese
prev_url:
prev_string:
top_url: tools.html
top_string: Tools
next_url: 
next_string: 
---

# k2hvecpmtool
A dedicated tool for generating, updating, and loading the Projection Matrix used internally during data storage and similarity search processing.

## Help
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

## k2hvecpmtool interface options

| Option                          | Description |
| ------------------------------- | ----------- |
| \--help(-h)                     | Displays usage instructions and the list of available options. |
| \--bit_size(-bs) <number>       | Specifies the bit count (size) within the range of 16 to 512 (default: `48`). |
| \--dimension_size(-ds) <number> | Specifies the number of vector dimensions within the range of 512 to 8192 (default: `4096`). |
| \--name(-n) <string>            | Specifies the array name for the Projection Matrix (default: `"pm"`). |
| \--version(-v) <string>         | Specifies the array version for the Projection Matrix (default: `"1.0"`). |
| \--output(-o) <file path>       | Specifies the destination file path (outputs to `stdout` if this option is omitted). |
| \--debug(-d)                    | Prints detailed debug log messages. |
| \--sourcecode(-s)               | Specifies the output format for source code generation (intended for internal developer use only). |
