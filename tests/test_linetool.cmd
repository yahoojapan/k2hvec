#
# K2HVEC
#
# Copyright 2026 LY Corporation.
#
# K2HVEC is a high-performance vector database for fast vector search,
# built on top of the K2HASH key-value store.
#
# For the full copyright and license information, please view
# the license file that was distributed with this source code.
#
# AUTHOR:   Takeshi Nakatani
# CREATE:   Tue, Jul 21 2026
# REVISION:
#

#==============================================================
# Run wth no break mode
#==============================================================
# [NOTE]
# As this is a prompt command file for testing, it executes to
# completion without stopping due to errors.
#
nobreak

#==============================================================
# help command
#==============================================================
echo "#========================================================"
echo "# Test help command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [help]'
echo "#----------------------------"
lap_time on
help; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help help]'
echo "#----------------------------"
lap_time on
help help; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help quit]'
echo "#----------------------------"
lap_time on
help quit; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help shell]'
echo "#----------------------------"
lap_time on
help shell; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help echo]'
echo "#----------------------------"
lap_time on
help echo; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help sleep]'
echo "#----------------------------"
lap_time on
help sleep; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help run]'
echo "#----------------------------"
lap_time on
help run; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help history_max]'
echo "#----------------------------"
lap_time on
help history_max; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help history]'
echo "#----------------------------"
lap_time on
help history; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help save_history]'
echo "#----------------------------"
lap_time on
help save_history; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help library_version]'
echo "#----------------------------"
lap_time on
help library_version; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help lap_time]'
echo "#----------------------------"
lap_time on
help lap_time; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help information]'
echo "#----------------------------"
lap_time on
help information; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help dump]'
echo "#----------------------------"
lap_time on
help dump; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help debug_level]'
echo "#----------------------------"
lap_time on
help debug_level; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help debug_logfile]'
echo "#----------------------------"
lap_time on
help debug_logfile; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help last_status]'
echo "#----------------------------"
lap_time on
help last_status; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help common_attribute]'
echo "#----------------------------"
lap_time on
help common_attribute; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help common_config]'
echo "#----------------------------"
lap_time on
help common_config; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help create_k2hash_key]'
echo "#----------------------------"
lap_time on
help create_k2hash_key; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help set]'
echo "#----------------------------"
lap_time on
help set; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help get]'
echo "#----------------------------"
lap_time on
help get; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help remove]'
echo "#----------------------------"
lap_time on
help remove; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help read_summary]'
echo "#----------------------------"
lap_time on
help read_summary; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help search]'
echo "#----------------------------"
lap_time on
help search; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help load_projection_matrix]'
echo "#----------------------------"
lap_time on
help load_projection_matrix; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help load_embedding]'
echo "#----------------------------"
lap_time on
help load_embedding; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help create_projection_matrix]'
echo "#----------------------------"
lap_time on
help create_projection_matrix; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help create_embedding]'
echo "#----------------------------"
lap_time on
help create_embedding; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help calculate_binary_hash]'
echo "#----------------------------"
lap_time on
help calculate_binary_hash; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [help calculate_cos]'
echo "#----------------------------"
lap_time on
help calculate_cos; last_status long
lap_time off

#==============================================================
# echo command
#==============================================================
echo "#========================================================"
echo "# Test echo command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [echo \"Test echo command context\"]'
echo "#----------------------------"
lap_time on
echo "Test echo command context"; last_status long
lap_time off

#==============================================================
# sleep command
#==============================================================
echo "#========================================================"
echo "# Test sleep command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [sleep 1]'
echo "#----------------------------"
lap_time on
sleep 1; last_status long
lap_time off

#==============================================================
# history_max command
#==============================================================
echo "#========================================================"
echo "# Test history_max command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [history_max 10000]'
echo "#----------------------------"
lap_time on
history_max 10000; last_status long
lap_time off

#==============================================================
# history command
#==============================================================
echo "#========================================================"
echo "# Test history command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [history]'
echo "#----------------------------"
lap_time on
history; last_status long
lap_time off

#==============================================================
# save_history command
#==============================================================
echo "#========================================================"
echo "# Test save_history command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [save_history /tmp/.k2hveclinetool_test_history_save.his]'
echo "#----------------------------"
lap_time on
save_history /tmp/.k2hveclinetool_test_history_save.his; last_status long
lap_time off

#==============================================================
# lap_time command
#==============================================================
echo "#========================================================"
echo "# Test lap_time command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [lap_time on message]'
echo "#----------------------------"
lap_time on message; last_status long

echo "#----------------------------"
echo '# Command: [lap_time off message]'
echo "#----------------------------"
lap_time off message; last_status long

#==============================================================
# library_version command
#==============================================================
echo "#========================================================"
echo "# Test library_version command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [library_version]'
echo "#----------------------------"
lap_time on
library_version; last_status long
lap_time off

#==============================================================
# debug_level command
#==============================================================
echo "#========================================================"
echo "# Test debug_level command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [debug_level dump]'
echo "#----------------------------"
lap_time on
debug_level dump; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_level message]'
echo "#----------------------------"
lap_time on
debug_level message; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_level warning]'
echo "#----------------------------"
lap_time on
debug_level warning; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_level silent]'
echo "#----------------------------"
lap_time on
debug_level silent; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_level error]'
echo "#----------------------------"
lap_time on
debug_level error; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_level]'
echo "#----------------------------"
lap_time on
debug_level; last_status long
lap_time off

#==============================================================
# debug_logfile command
#==============================================================
echo "#========================================================"
echo "# Test debug_logfile command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [debug_logfile /tmp/.k2hveclinetool_test_debug_logfile]'
echo "#----------------------------"
lap_time on
debug_logfile /tmp/.k2hveclinetool_test_debug_logfile; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [debug_logfile]'
echo "#----------------------------"
lap_time on
debug_logfile; last_status long
lap_time off

#==============================================================
# last_status command
#==============================================================
echo "#========================================================"
echo "# Test last_status command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [last_status]'
echo "#----------------------------"
lap_time on
last_status
lap_time off

echo "#----------------------------"
echo '# Command: [last_status long]'
echo "#----------------------------"
lap_time on
last_status long
lap_time off

#==============================================================
# common_attribute command
#==============================================================
echo "#========================================================"
echo "# Test common_attribute command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [common_attribute]'
echo "#----------------------------"
lap_time on
common_attribute; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [common_attribute expire=100 pass=passphrase]'
echo "#----------------------------"
lap_time on
common_attribute expire=100 pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [common_attribute expire=unset pass=unset]'
echo "#----------------------------"
lap_time on
common_attribute expire=unset pass=unset; last_status long
lap_time off

#==============================================================
# common_config command
#==============================================================
echo "#========================================================"
echo "# Test common_config command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [common_config]'
echo "#----------------------------"
lap_time on
common_config; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [common_config cal_bit_size=48 neighbor_key_limit=4000 content_direct_limit=1024]'
echo "#----------------------------"
lap_time on
common_config cal_bit_size=48 neighbor_key_limit=4000 content_direct_limit=1024; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [common_config cal_bit_size=default neighbor_key_limit=default content_direct_limit=default]'
echo "#----------------------------"
lap_time on
common_config cal_bit_size=default neighbor_key_limit=default content_direct_limit=default; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [common_config cal_bit_size=4 neighbor_key_limit=2000 content_direct_limit=32]'
echo "#----------------------------"
lap_time on
common_config cal_bit_size=4 neighbor_key_limit=2000 content_direct_limit=32; last_status long
lap_time off

#==============================================================
# create_projection_matrix command
#==============================================================
echo "#========================================================"
echo "# Test create_projection_matrix command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [create_projection_matrix dimension=512 bitsize=16 name=test_create_projection_matrix_min version=1.0 output=/tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo "#----------------------------"
lap_time on
create_projection_matrix dimension=512 bitsize=16 name=test_create_projection_matrix_min version=1.0 output=/tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_projection_matrix dimension=8192 bitsize=512 name=test_create_projection_matrix_max version=1.0 output=/tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo "#----------------------------"
lap_time on
create_projection_matrix dimension=8192 bitsize=512 name=test_create_projection_matrix_max version=1.0 output=/tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_projection_matrix dimension=default bitsize=default name=default version=default output=/tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo "#----------------------------"
lap_time on
create_projection_matrix dimension=default bitsize=default name=default version=default output=/tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_projection_matrix]'
echo "#----------------------------"
lap_time on
create_projection_matrix; last_status long
lap_time off

#==============================================================
# create_embedding command
#==============================================================
echo "#========================================================"
echo "# Test create_embedding command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [create_embedding dimension=512 output=/tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo "#----------------------------"
lap_time on
create_embedding dimension=512 output=/tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_embedding dimension=8192 output=/tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo "#----------------------------"
lap_time on
create_embedding dimension=8192 output=/tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_embedding dimension=default output=/tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo "#----------------------------"
lap_time on
create_embedding dimension=default output=/tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_embedding]'
echo "#----------------------------"
lap_time on
create_embedding; last_status long
lap_time off

#==============================================================
# load_projection_matrix / load_embedding / information / dump command
#==============================================================
echo "#========================================================"
echo "# Test load_projection_matrix / load_embedding / information / dump command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo '#          [information pm]'
echo '#          [information em]'
echo '#          [dump pm output=/tmp/.k2hveclinetool_test_dump_min.pm]'
echo '#          [dump em output=/tmp/.k2hveclinetool_test_dump_min.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
information pm; last_status long
information em; last_status long
dump pm output=/tmp/.k2hveclinetool_test_dump_min.pm; last_status long
dump em output=/tmp/.k2hveclinetool_test_dump_min.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo '#          [information pm]'
echo '#          [information em]'
echo '#          [dump pm output=/tmp/.k2hveclinetool_test_dump_max.pm]'
echo '#          [dump em output=/tmp/.k2hveclinetool_test_dump_max.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
information pm; last_status long
information em; last_status long
dump pm output=/tmp/.k2hveclinetool_test_dump_max.pm; last_status long
dump em output=/tmp/.k2hveclinetool_test_dump_max.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo '#          [information pm]'
echo '#          [information em]'
echo '#          [dump pm output=/tmp/.k2hveclinetool_test_dump_def.pm]'
echo '#          [dump em output=/tmp/.k2hveclinetool_test_dump_def.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
information pm; last_status long
information em; last_status long
dump pm output=/tmp/.k2hveclinetool_test_dump_def.pm; last_status long
dump em output=/tmp/.k2hveclinetool_test_dump_def.em; last_status long
lap_time off

#==============================================================
# create_k2hash_key command
#==============================================================
echo "#========================================================"
echo "# Test create_k2hash_key command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
create_k2hash_key /tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [create_k2hash_key]'
echo "#----------------------------"
lap_time on
create_k2hash_key; last_status long
lap_time off

#==============================================================
# set/get/remove command
#==============================================================
echo "#========================================================"
echo "# Test set/get/remove command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=ABCDEFGHIJKLMNOPQRSTUVWXYZ]'
echo '#          [get pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_pass_def.em]'
echo '#          [remove emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo "#----------------------------"
lap_time on
set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=ABCDEFGHIJKLMNOPQRSTUVWXYZ; last_status long
get pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_pass_def.em; last_status long
remove emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [set expire=900 pass=unset emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=ABCDEFGHIJKLMNOPQRSTUVWXYZ]'
echo '#          [get pass=unset emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_nopass_def.em]'
echo '#          [get noattrcheck emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_noattr_nopass_def.em]'
echo '#          [remove emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo "#----------------------------"
lap_time on
set expire=900 pass=unset emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=ABCDEFGHIJKLMNOPQRSTUVWXYZ; last_status long
get pass=unset emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_nopass_def.em; last_status long
get noattrcheck emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em output=/tmp/.k2hveclinetool_test_get_noattr_nopass_def.em; last_status long
remove emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [set expire=900 pass=passphrase value=ABCDEFGHIJKLMNOPQRSTUVWXYZ]'
echo '#          [get pass=passphrase key=em output=/tmp/.k2hveclinetool_test_get_pass_em.em]'
echo '#          [remove key=em]'
echo "#----------------------------"
lap_time on
set expire=900 pass=passphrase value=ABCDEFGHIJKLMNOPQRSTUVWXYZ; last_status long
get pass=passphrase key=em output=/tmp/.k2hveclinetool_test_get_pass_em.em; last_status long
remove key=em; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [set expire=900 pass=unset value=ABCDEFGHIJKLMNOPQRSTUVWXYZ]'
echo '#          [get pass=unset key=em output=/tmp/.k2hveclinetool_test_get_nopass_em.em]'
echo '#          [get noattrcheck key=em output=/tmp/.k2hveclinetool_test_get_noattr_nopass_em.em]'
echo '#          [remove key=em]'
echo "#----------------------------"
lap_time on
set expire=900 pass=unset value=ABCDEFGHIJKLMNOPQRSTUVWXYZ; last_status long
get pass=unset key=em output=/tmp/.k2hveclinetool_test_get_nopass_em.em; last_status long
get noattrcheck key=em output=/tmp/.k2hveclinetool_test_get_noattr_nopass_em.em; last_status long
remove key=em; last_status long
lap_time off

#==============================================================
# calculate_binary_hash command
#==============================================================
echo "#========================================================"
echo "# Test calculate_binary_hash command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo "#----------------------------"
lap_time on
calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo "#----------------------------"
lap_time on
calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo "#----------------------------"
lap_time on
calculate_binary_hash emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em pmfile=/tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [calculate_binary_hash]'
echo "#----------------------------"
lap_time on
calculate_binary_hash; last_status long
lap_time off

#==============================================================
# search(same em) command
#==============================================================
echo "#========================================================"
echo "# Test search(same em) command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [set emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em noattr pass=unset]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
set emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em noattr pass=unset; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [set emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em noattr pass=unset]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
set emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em noattr pass=unset; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [set emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em noattr pass=unset]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
set emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em noattr pass=unset; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_min.em pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_max.em pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=0123456789]'
echo '#          [search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
set expire=900 pass=passphrase emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em value=0123456789; last_status long
search emfile=/tmp/.k2hveclinetool_test_create_embedding_def.em pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo '#          [set value=0123456789]'
echo '#          [search key=em]'
echo '#          [search key=em noattr pass=unset]'
echo '#          [search key=em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
set value=0123456789; last_status long
search key=em; last_status long
search key=em noattr pass=unset; last_status long
search key=em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo '#          [set value=0123456789]'
echo '#          [search key=em]'
echo '#          [search key=em noattr pass=unset]'
echo '#          [search key=em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
set value=0123456789; last_status long
search key=em; last_status long
search key=em noattr pass=unset; last_status long
search key=em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo '#          [set value=0123456789]'
echo '#          [search key=em]'
echo '#          [search key=em noattr pass=unset]'
echo '#          [search key=em noattr]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
set value=0123456789; last_status long
search key=em; last_status long
search key=em noattr pass=unset; last_status long
search key=em noattr; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em]'
echo '#          [set expire=900 pass=passphrase value=0123456789]'
echo '#          [search key=em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_min.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_min.em; last_status long
set expire=900 pass=passphrase value=0123456789; last_status long
search key=em pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em]'
echo '#          [set expire=900 pass=passphrase value=0123456789]'
echo '#          [search key=em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_max.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_max.em; last_status long
set expire=900 pass=passphrase value=0123456789; last_status long
search key=em pass=passphrase; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm]'
echo '#          [load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em]'
echo '#          [set expire=900 pass=passphrase value=0123456789]'
echo '#          [search key=em pass=passphrase]'
echo "#----------------------------"
lap_time on
load_projection_matrix /tmp/.k2hveclinetool_test_create_projection_matrix_def.pm; last_status long
load_embedding /tmp/.k2hveclinetool_test_create_embedding_def.em; last_status long
set expire=900 pass=passphrase value=0123456789; last_status long
search key=em pass=passphrase; last_status long
lap_time off

#==============================================================
# Finish
#==============================================================
exit

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
