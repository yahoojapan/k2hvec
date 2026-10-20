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

# [NOTE]
# This file is a template for a command file.
# You must replace the "__K2HVECCLI_KEYOPS_FILE_BASE_EM__",
# "__K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM__", "__K2HVECCLI_KEYOPS_KEYNAME__"
# variable within the file with the correct key name before using it.
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
# set/get/read_summary command
#==============================================================
echo "#========================================================"
echo "# Test set/get/read_summary command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [load_embedding <embedding base file>]'
echo '#          [set value=ABCDEFGHIJKLMNOPQRSTUVWXYZ]'
echo '#          [get key=<key name>]'
echo '#          [read_summary <key name>]'
echo "#----------------------------"
lap_time on
load_embedding __K2HVECCLI_KEYOPS_FILE_BASE_EM__ ; last_status long
set value=ABCDEFGHIJKLMNOPQRSTUVWXYZ ; last_status long
get key=__K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
read_summary __K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
lap_time off

echo "#----------------------------"
echo '# Command: [load_embedding <embedding base file>]'
echo '#          [set value=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789]'
echo '#          [get key=<key name>]'
echo '#          [read_summary <key name>]'
echo "#----------------------------"
lap_time on
load_embedding __K2HVECCLI_KEYOPS_FILE_BASE_EM__ ; last_status long
set value=ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ; last_status long
get key=__K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
read_summary __K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
lap_time off

#==============================================================
# calculate_cos command
#==============================================================
echo "#========================================================"
echo "# Test calculate_cos command : same embedding data(em)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [calculate_cos <key nme> key=em]'
echo "#----------------------------"
lap_time on
calculate_cos __K2HVECCLI_KEYOPS_KEYNAME__ key=em; last_status long
lap_time off

echo "#========================================================"
echo "# Test calculate_cos command : same embedding data(file)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [calculate_cos <key nme> emfile=<embedding base file>]'
echo "#----------------------------"
lap_time on
calculate_cos __K2HVECCLI_KEYOPS_KEYNAME__ emfile=__K2HVECCLI_KEYOPS_FILE_BASE_EM__; last_status long
lap_time off

echo "#========================================================"
echo "# Test calculate_cos command : slightly embedding data(file)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [calculate_cos <key nme> emfile=<embedding slightly file>]'
echo "#----------------------------"
lap_time on
calculate_cos __K2HVECCLI_KEYOPS_KEYNAME__ emfile=__K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM__; last_status long
lap_time off

#==============================================================
# search command
#==============================================================
echo "#========================================================"
echo "# Test search command : same embedding data(em)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [search key=em]'
echo "#----------------------------"
lap_time on
search key=em; last_status long
lap_time off

echo "#========================================================"
echo "# Test search command : same embedding data(file)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [search emfile=<embedding base file>]'
echo "#----------------------------"
lap_time on
search emfile=__K2HVECCLI_KEYOPS_FILE_BASE_EM__ ; last_status long
lap_time off

echo "#========================================================"
echo "# Test search command : not found slightly embedding data(file)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [search score=0.9999f emfile=<embedding slightly file>]'
echo "#----------------------------"
lap_time on
search score=0.9999f emfile=__K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM__ ; last_status long
lap_time off

echo "#========================================================"
echo "# Test search command : found slightly embedding data(file)"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [search score=0.1f emfile=<embedding slightly file>]'
echo "#----------------------------"
lap_time on
search score=0.1f emfile=__K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM__ ; last_status long
lap_time off

#==============================================================
# remove command
#==============================================================
echo "#========================================================"
echo "# Test remove command"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [remove key=<key nme>]'
echo "#----------------------------"
lap_time on
remove key=__K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
lap_time off

echo "#========================================================"
echo "# Test get command : not found"
echo "#========================================================"
echo "#----------------------------"
echo '# Command: [get key=<key nme>]'
echo "#----------------------------"
lap_time on
get key=__K2HVECCLI_KEYOPS_KEYNAME__ ; last_status long
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
