---
layout: contents
language: en-us
title: Tools
short_desc: K2HVEC - Vector Database and Search built on K2HASH
lang_opp_file: tools_k2hveclinetoolja.html
lang_opp_word: To Japanese
prev_url:
prev_string:
top_url: tools.html
top_string: Tools
next_url: 
next_string: 
---

# k2hveclinetool
This is a command-line tool (interactive / command execution) for intuitively operating and testing database manipulations and vector searches.  
It allows you to interactively operate and verify almost all features of the K2HVEC library, as well as execute commands automatically in batch mode by loading a command file.

This section covers the startup options and available commands.

- **k2hveclinetool Startup Options**  
Explains the various command-line options available when launching the tool.
- **k2hveclinetool Commands (Prompt Input Commands)**  
Explains the commands that can be executed within the interactive prompt.

## k2hveclinetool Startup Options
**k2hveclinetool** is an interactive tool that presents a command-input prompt upon launch.  
By passing the `--help` (or `-h`) option as a startup option, you can display help messages for all available options.

### k2hveclinetool Help
```
$ k2hveclinetool --help

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

Common configuration(Binary hash / Index keys / Direct Content Limits):
    --binary_hash_bit_size(-bs) <bit>     Specifies the number of bits for the binary hash calculation result
    --neighbor_key_limit(-nkl) <limit>    Specifies the maximum number of neighbor keys allowed for vector similarity searches
    --content_direct_limit(-cdl) <limit>  Specifies the maximum size for saving content text directly to the main key

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
    common_config(cc) <...>               Set or Print Binary Hash bit count, neighbor keys limit and content direct limit.
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

### k2hveclinetool interface options

| Option                                | Description |
| ------------------------------------- | ----------- |
| \--help(-h)                           | Displays the help message for startup options |
| \--library_version(-libver)           | Displays the versions of the K2HVEC library and related libraries |
| \--history_max(-hmax) <count>         | Sets the maximum number of command history items retained in the prompt |
| \--lap_time(-lap)                     | Enables or disables the display of elapsed execution time for each command |
| \--no_welcome_message(-no_welcome)    | Suppresses initial startup messages (such as banner text) when launching k2hveclinetool |
| \--run_file(-run) <file path>         | Loads a command file containing prompt commands and executes them automatically in batch mode |
| \-- <prompt commands...>              | Executes the argument following this option directly as a prompt command |
| \--file(-f) <file path>               | Specifies the path to the K2Hash database file used by the K2HVEC library |
| \--temporary_file(-t)                 | Uses an automatically deleted temporary file for the K2Hash database |
| \--memory(-m)                         | Constructs the K2Hash database in memory (In-Memory mode) for use by the K2HVEC library |
| \--mask(-msk) <bit count>             | Specifies the number of mask bits used when creating the K2Hash database |
| \--collision_mask(-cmask) <bit count> | Specifies the number of collision mask (cmask) bits used when creating the K2Hash database |
| \--element_count(-elcnt) <count>      | Specifies the initial element count used when creating the K2Hash database |
| \--page_size(-page) <size>            | Specifies the page size used when creating the K2Hash database |
| \--full_mapping(-full)                | Opens the K2Hash file (or memory) in full mapping mode (enabled by default for In-Memory mode; disabled by default for files) |
| \--read_only(-r)                      | Opens the K2Hash database in read-only mode |
| \--debug_level(-d) <debug mode>       | Specifies the debug log output level (Allowed values: `SILENT` / `SLT`, `ERROR` / `ERR`, `WARNING` / `WAN`, `MESSAGE` / `MSG`, `DUMP` / `DMP`) |
| \--debug_logfile(-dlog) <file path>   | Specifies the destination file path for debug log output (defaults to standard output/error if unspecified) |
| \--binary_hash_bit_size(-bs) <bit>    | Specifies the number of bits for the calculated binary hash result |
| \--neighbor_key_limit(-nkl) <limit>   | Specifies the maximum number of neighbor keys allowed for vector similarity searches |
| \--content_direct_limit(-cdl) <limit> | Specifies the maximum size for saving content text directly to the main key |

## k2hveclinetool Commands (Prompt Input Commands)
This section describes the commands available within the **k2hveclinetool** interactive prompt.

### Help
```
K2HVEC > help

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
    common_config(cc) <...>               Set or Print Binary Hash bit count, neighbor keys limit and content direct limit.
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

### Prompt Input Commands

| Option                                | Description |
| ------------------------------------- | ----------- |
| help(h) <command>                     | Print command detail |
| quit(q)                               | Quit(exit) the dialogue and terminate the program (alternative command: exit(e)) |
| shell(sh)                             | Escape to a shell (alternative command: !) |
| echo <strings...>                     | Print argument string (alternative command: print) |
| sleep <second>                        | Wait second (alternative command: wait) |
| library_version(libver)               | Displays the library version (alternative command: libversion) |
| history_max(hmax) <count>             | Set maximum count for prompt command history (alternative command: hismax) |
| history(his)                          | Display all command history (alternative command: history_log) |
| save_history(save) <file>             | Save all command history to the file (alternative command: save_his) |
| debug_level(d) <debug level>          | Specifies the debug level (alternative command: debug) |
| debug_logfile(dlog) <file>            | Specifies the file for debug log output (alternative command: debug_log) |
| common_attribute(attr) <...>          | Set and Print common expire and passphrase for data in k2hash. |
| common_config(cc) <...>               | Set or Print Binary Hash bit count, the maximum number of neighbor keys, and the maximum size for saving content text directly to the main key. |
| last_status($?)                       | Print the exit status of the previous prompt command(0: success, 1: failure) |
| lap_time(lap) <"on" \| "off"> <...>   | Toggle disable/enable the display of the elapsed time for command execution |
| run <file>                            | Load a file containing prompt commands and execute those commands |
| break                                 | Specify stop to run even if an error occurs while executing the prompt commands(default) |
| nobreak                               | Specify continue to run even if an error occurs while executing the prompt commands |
| load_projection_matrix(pm) <file>     | Load and set the Projection Matrix data |
| load_embedding(em) <file>             | Load and set the Embedding data as Preloaded data |
| create_projection_matrix(cpm) <...>   | Create new Projection Matrix, and either load it or save it to file (alternative command: create_pm) |
| create_embedding(cem) <...>           | Create embedding data, and either load it as preloaded data or save it to file (alternative command: create_em) |
| information(i) <pm \| em>             | Display information of Projection Matrix or preloaded embeddings data (alternative command: info, status) |
| dump(dmp) <pm \| em> <...>            | Dump and save projection matrix or preloaded embeddings data |
| create_k2hash_key(key) <file>         | Create and print a K2Hash key for embedding data (alternative command: k2h_key, k2hkey) |
| calculate_binary_hash(cbh) <...>      | Calculate Binary hash value (alternative command: calculate_bh, cal_bh) |
| set(s) <...>                          | Set embedding data to K2Hash (alternative command: push, put) |
| get(g) <...>                          | Get raw embedding data from K2Hash to file (alternative command: print, p) |
| remove(rm) <...>                      | Remove embedding data in K2Hash (alternative command: delete, del) |
| read_summary(r) <...>                 | Read summary embedding data in K2Hash (alternative command: read) |
| search(find) <...>                    | Run vector search for the embedding data in K2Hash and find the closest data (alternative command: f) |
| calculate_cos(cos) <key> <...>        | Calculate Cosine Similarity value (alternative command: calculate_cosin_similarity, cal_cos) |
