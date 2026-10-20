#!/bin/sh
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

#--------------------------------------------------------------
# Common Variables
#--------------------------------------------------------------
#
# Instead of pipefail(for shells not support "set -o pipefail")
#
PIPEFAILURE_FILE="/tmp/.pipefailure.$(od -An -tu4 -N4 /dev/random | tr -d ' \n')"

PRGNAME=$(basename "${0}")
SCRIPTDIR=$(dirname "${0}")
SCRIPTDIR=$(cd "${SCRIPTDIR}" || exit 1; pwd)
SRCTOP=$(cd "${SCRIPTDIR}/.." || exit 1; pwd)
LIBOBJDIR="${SRCTOP}/lib/.libs"
TESTDIR="${SRCTOP}/tests"

#--------------------------------------------------------------
# Common Variables and Utility functions
#--------------------------------------------------------------
#
# Escape sequence
#
set_escape()
{
	CESC=$(printf '\033')

	if [ -t 1 ]; then
#		CBLD=$(printf '\033[1m')
		CREV=$(printf '\033[7m')
		CRED=$(printf '\033[31m')
		CYEL=$(printf '\033[33m')
		CGRN=$(printf '\033[32m')
		CDEF=$(printf '\033[0m')
	else
#		CBLD=""
		CREV=""
		CRED=""
		CYEL=""
		CGRN=""
		CDEF=""
	fi
}

#
# Immediately after startup, it operates with terminal attributes.
#
set_escape

#--------------------------------------------------------------
# Message functions
#--------------------------------------------------------------
PRNTITLE()
{
	echo "${CGRN}-----------------------------------------------------------------${CDEF}"
	echo "${CGRN}[TITLE]${CDEF} ${CGRN}$*${CDEF}"
	echo "${CGRN}-----------------------------------------------------------------${CDEF}"
}

PRNSUCCESS()
{
	echo "${CGRN}[SUCCEED]${CDEF} $*"
	echo ""
}

PRNFAILURE()
{
	echo "${CRED}[FAILED]${CDEF} $*"
	echo ""
}

PRNERR()
{
	echo "${CRED}[ERROR]${CDEF} ${CRED}$*${CDEF}"
}

PRNWARN()
{
	echo "${CYEL}[WARNING]${CDEF} $*"
}

PRNMSG()
{
	echo "${CYEL}[MSG]${CDEF} ${CYEL}$*${CDEF}"
}

PRNINFO()
{
	echo "    ${CREV}[INFO]${CDEF} $*"
}

#--------------------------------------------------------------
# Usage
#--------------------------------------------------------------
func_usage()
{
	#
	# Print all
	#
	echo ""
	echo "Usage: $1 [--help(-h)] [--debug(-d)] [--update_log(-u)] [<output file>]"
	echo ""
	echo "Option/Parameter:"
	echo " --help(-h)                Print usage."
	echo " --debug(-d)               Run test with debug flags."
	echo " --update_log(-u)          Update master log file after test success."
	echo " <output file>             Outputs the program's output to a specified file."
	echo ""
}

#--------------------------------------------------------------
# Parse options
#--------------------------------------------------------------
OUTPUT_FILE=""
DEBUG_MODE=0
UPDATE_LOG_FILE=0

#
# Parse options
#
while [ $# -ne 0 ]; do
	if [ -z "$1" ]; then
		break;

	elif echo "$1" | grep -q -i -e "^-h$" -e "^--help$"; then
		func_usage "${PRGNAME}"
		exit 0

	elif echo "$1" | grep -q -i -e "^-d$" -e "^--dbg$" -e "^--debug$"; then
		if [ "${DEBUG_MODE}" -ne 0 ]; then
			PRNERR "Already specified --debug(-d) option."
			exit 1
		fi
		DEBUG_MODE=1

	elif echo "$1" | grep -q -i -e "^-u$" -e "^--update$" -e "^--update_log$"; then
		if [ "${UPDATE_LOG_FILE}" -ne 0 ]; then
			PRNERR "Already specified --update_log(-u) option."
			exit 1
		fi
		UPDATE_LOG_FILE=1

	else
		if [ -n "${OUTPUT_FILE}" ]; then
			PRNERR "Already specified output file: ${OPT_OUTPUT_FILE}"
			exit 1
		fi
		OUTPUT_FILE="$1"
	fi
	shift
done

if [ -z "${OUTPUT_FILE}" ]; then
	OUTPUT_FILE="/dev/null"
else
	if [ -e "${OUTPUT_FILE}" ]; then
		PRNERR "${OUTPUT_FILE} already exists. Please specify a non-existent file, or delete the file and run again."
		exit 1
	fi
fi

if [ "${DEBUG_MODE}" -eq 0 ]; then
	PMGEN_TOOL_DBG_OPT=""
	PMTEST_TOOL_DBG_OPT=""
	COSTEST_TOOL_DBG_OPT=""
else
	PMGEN_TOOL_DBG_OPT="--debug"
	PMTEST_TOOL_DBG_OPT="--debug"
	COSTEST_TOOL_DBG_OPT="--debug"
fi

#--------------------------------------------------------------
# Environments
#--------------------------------------------------------------
#
# LD_LIBRARY_PATH / TESTPROGDIR
#
LD_LIBRARY_PATH="${LIBOBJDIR}"
export LD_LIBRARY_PATH

#--------------------------------------------------------------
# Utilities
#--------------------------------------------------------------
#
# Cut 'CREATE: <date>' in the file
#
CutCreateDateInFile()
{
	if [ -z "$1" ] || [ ! -f "$1" ]; then
		return 1
	fi
	if ! sed -i -e 's|CREATE:.*$||gi' "$1" >/dev/null 2>&1; then
		return 1
	fi
	return 0
}

#
# Remove runtime-variable values from the test_linetool.cmd result file
#
ConvertValiabelsInTestLineToolResult()
{
	if [ -z "$1" ] || [ ! -f "$1" ]; then
		return 1
	fi
	if [ -z "$2" ]; then
		return 1
	fi
	if ! sed -e 's|Execution time -.*$|Execution time - 000ms 000us 000ns(000000000ns)|g'											\
			-e 's|Checksum[[:space:]]*:.*$|Checksum : 0.0f|gi'																		\
			-e 's|2nd(FNV-1a) hash[[:space:]]*:.*$|2nd(FNV-1a) hash : 0000000000000000|gi'											\
			-e 's|FNV-1a hash[[:space:]]*:.*$|FNV-1a hash : 0000000000000000|gi'													\
			-e 's|K2hash main key[[:space:]]*:.*$|K2hash main key : xxx@0.0/0000-0000000000000000|gi'								\
			-e 's|2nd hash[[:space:]]*:.*$|2nd hash : 0000000000000000|gi'															\
			-e 's|Cosine Similarity value[[:space:]]*:.*$|Cosine Similarity value : 0.0f|gi'										\
			-e 's|[[:space:]]*Binary hash[[:space:]][[:space:]]*:[[:space:]][[:space:]]*.*$|    Binary hash : 0000000000000000|gi'	\
			-e 's|custom@1.0/[0-9][0-9][0-9][0-9]-[0-9a-f]\{16\}|custom@1.0/0000-0000000000000000|gi'								\
			-e 's|custom@1.0/[0-9][0-9][0-9][0-9]|custom@1.0/0000|gi'																\
			-e 's|custom@1.0/[0-9][0-9][0-9]|custom@1.0/000|gi'																		\
			-e 's|custom@1.0/[0-9][0-9]|custom@1.0/00|gi'																			\
			-e 's|custom@1.0/[0-9]|custom@1.0/0|gi'																					\
			-e 's|K2hvec Version .* (commit: .*).*$|K2hvec Version X.X.X (commit: xxxxxxx)|gi'										\
			-e 's|K2HASH library Version .* (commit: .*).*$|K2HASH library Version X.X.X (commit: xxxxxxx)|gi'						\
			-e 's|FULLOCK Version .* (commit: .*).*$|FULLOCK Version X.X.X (commit: xxxxxxx)|gi'									\
			"$1" > "$2" 2>/dev/null; then
		return 1
	fi
	return 0
}

#
# Remove runtime-variable values from the test_linetool_keyops.cmd result file
#
ConvertValiabelsInTestLineToolKeyOpsResult()
{
	if [ -z "$1" ] || [ ! -f "$1" ]; then
		return 1
	fi
	if [ -z "$2" ]; then
		return 1
	fi
	if ! sed -e 's|Execution time -.*$|Execution time - 000ms 000us 000ns(000000000ns)|g'				\
			-e 's|Checksum[[:space:]]*:.*$|Checksum : 0.0f|gi'											\
			-e 's|2nd(FNV-1a) hash[[:space:]]*:.*$|2nd(FNV-1a) hash : 0000000000000000|gi'				\
			-e 's|FNV-1a hash[[:space:]]*:.*$|FNV-1a hash : 0000000000000000|gi'						\
			-e 's|Value FNV-1a hash[[:space:]]*:.*$|Value FNV-1a hash : 0000000000000000|gi'			\
			-e 's|Value Content prefix[[:space:]]*:.*$|Value Content prefix : 0000000000000000|gi'		\
			-e 's|K2hash main key[[:space:]]*:.*$|K2hash main key : xxx@0.0/0000-0000000000000000|gi'	\
			-e 's|K2hash key[[:space:]]*:.*$|K2hash key : xxx@0.0/0000-0000000000000000|gi'				\
			-e 's|2nd hash[[:space:]]*:.*$|2nd hash : 0000000000000000|gi'								\
			-e 's|Cosine Similarity value[[:space:]]*:.*$|Cosine Similarity value : 0.0f|gi'			\
			-e 's|Result[[:space:]]*:.*$|Result : 0.0f|gi'												\
			-e 's|builtin@1.0/[0-9][0-9][0-9][0-9]-.*|builtin@1.0/0000-0000000000000000|gi'				\
			-e 's|builtin@1.0/[0-9][0-9][0-9][0-9]|builtin@1.0/0000|gi'									\
			-e 's|builtin@1.0/[0-9][0-9][0-9]|builtin@1.0/000|gi'										\
			-e 's|builtin@1.0/[0-9][0-9]|builtin@1.0/00|gi'												\
			-e 's|builtin@1.0/[0-9]|builtin@1.0/0|gi'													\
			-e 's| Load(.*) :| Load(xxxx) :|gi'															\
			"$1" > "$2" 2>/dev/null; then
		return 1
	fi
	return 0
}

#
# Create a file with slightly different embedding value.
#
# $1	: Input embedding file
# $2	: Output slightly different embedding file
# $3	: Modify line count in embedding file(default: 64 -> changed about 12.5%(64/512) data)
#
CreateSlightlyDifferentEmbeddingFile()
{
	if [ -z "$1" ] || [ ! -f "$1" ]; then
		return 1
	fi
	if [ -z "$2" ]; then
		return 1
	fi
	INPUT_FILE="$1"
	OUTPUT_FILE="$2"

	if [ -z "$3" ]; then
		LINE_COUNT=64
	else
		LINE_COUNT="$3"
	fi

	_IS_FOUND_EMBEDDING_LINE=0
	_IS_CONVERT_FIRST_LINE=0
	while IFS= read -r _oneline || [ -n "${_oneline}" ]; do
		if [ "${_IS_FOUND_EMBEDDING_LINE}" -eq 0 ]; then
			if echo "${_oneline}" | grep -q 'EMBEDDING[[:space:]]*=[[:space:]]*{'; then
				# found start line
				_IS_FOUND_EMBEDDING_LINE=1
			fi
		elif [ "${_IS_CONVERT_FIRST_LINE}" -lt "${LINE_COUNT}" ]; then
			if echo "${_oneline}" | grep -q '^[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,[[:space:]]*.*f,'; then
				# found first data line
				echo "     0.0f,             0.0f,             0.0f,             0.0f,                 0.0f,             0.0f,             0.0f,             0.0f,"
				_IS_CONVERT_FIRST_LINE=$((_IS_CONVERT_FIRST_LINE + 1))
				continue
			fi
		fi
		echo "${_oneline}"

	done < "${INPUT_FILE}" > "${OUTPUT_FILE}"

	if [ "${_IS_FOUND_EMBEDDING_LINE}" -eq 0 ] || [ "${_IS_CONVERT_FIRST_LINE}" -eq 0 ]; then
		# Not found
		return 1
	fi

	return 0
}

#==============================================================
# Main
#==============================================================
#
# Variables
#
PROCID=$$
TEST_NAME_BASE=$(echo "${PRGNAME}" | sed -e 's#\.sh.*$##g' | tr -d '\n')

#
# Logging with FIFO file
#
# [NOTE]
# This script outputs standard output and standard error to a log file.
# When running in a terminal, stdout is printed in color, but ANSI escape
# sequences are stripped from the saved log file.
#
# Due to Bourne Shell(/bin/sh) limitations, a Named Pipe(FIFO) is used to
# handle the removal of escape sequences.
#
# Execution flow:
# 1) Output is passed to "tee", which writes to stdout and the FIFO file simultaneously.
# 2) A background process reads from the FIFO file, strips escape sequences via "sed",
#    and saves the clean output to the log file.
#
# The code below implements this logic.
#
LOG_FIFO_FILE="/tmp/.${TEST_NAME_BASE}_fifo_$$"

# Trap for removing FIFO file
trap 'rm -f "${LOG_FIFO_FILE}"' EXIT INT TERM

# Create FIFO file
mkfifo "${LOG_FIFO_FILE}"

# Run background process writing log file
sed -E "s/${CESC}\[[0-9;]*[mK]//g" < "${LOG_FIFO_FILE}" > "${OUTPUT_FILE}" &

#
# Error count file
#
ERROR_COUNT_FILE="/tmp/.${TEST_NAME_BASE}_errcount_$$"

#
# Test programs
#
PMGEN_TOOL="${TESTDIR}/k2hvecpmtool"
PMTEST_TOOL="${TESTDIR}/k2hvec_pmtest"
BHTEST_TOOL="${TESTDIR}/k2hvec_bhtest"
COSTEST_TOOL="${TESTDIR}/k2hvec_costest"
K2HVECCLI_TOOL="${TESTDIR}/k2hveclinetool"

#
# Common test parameters
#
BITSIZE_MAX="512"
BITSIZE_NORM="48"
BITSIZE_MIN="16"

DIMENSION_MAX="8192"
DIMENSION_NORM="4096"
DIMENSION_MIN="512"

#
# For k2hvecpmtool options
#
# PM name		: test_<bitsize>_<dimension>
# PM version	: 1.<bitsize>.<dimension>
# PM file name	: test_<bitsize>_<dimension>.pm
#
PM_CUSTOM_NAME_BASE="${TEST_NAME_BASE}"
PM_CUSTOM_FILE_BASE="/tmp/.k2hvecpmtool_${PM_CUSTOM_NAME_BASE}_custom_${PROCID}"

PM_CUSTOM_NAME_MAX="${PM_CUSTOM_NAME_BASE}_${BITSIZE_MAX}_${DIMENSION_MAX}"
PM_CUSTOM_NAME_NORM="${PM_CUSTOM_NAME_BASE}_${BITSIZE_NORM}_${DIMENSION_NORM}"
PM_CUSTOM_NAME_MIN="${PM_CUSTOM_NAME_BASE}_${BITSIZE_MIN}_${DIMENSION_MIN}"

PM_CUSTOM_VERSION_MAX="1.${BITSIZE_MAX}.${DIMENSION_MAX}"
PM_CUSTOM_VERSION_NORM="1.${BITSIZE_NORM}.${DIMENSION_NORM}"
PM_CUSTOM_VERSION_MIN="1.${BITSIZE_MIN}.${DIMENSION_MIN}"

PM_CUSTOM_FILE_MAX="${PM_CUSTOM_FILE_BASE}_${BITSIZE_MAX}_${DIMENSION_MAX}.pm"
PM_CUSTOM_FILE_NORM="${PM_CUSTOM_FILE_BASE}_${BITSIZE_NORM}_${DIMENSION_NORM}.pm"
PM_CUSTOM_FILE_MIN="${PM_CUSTOM_FILE_BASE}_${BITSIZE_MIN}_${DIMENSION_MIN}.pm"

#
# For k2hvec_bhtest options
#
# EM file name	: test_<bitsize>_<dimension>.em
#
EM_TEST_NAME_BASE="${TEST_NAME_BASE}"
EM_TEST_FILE_BASE="/tmp/.k2hvec_bhtest_${EM_TEST_NAME_BASE}_${PROCID}"

EM_TEST_FILE_MAX="${EM_TEST_FILE_BASE}_${BITSIZE_MAX}_${DIMENSION_MAX}.em"
EM_TEST_FILE_NORM="${EM_TEST_FILE_BASE}_${BITSIZE_NORM}_${DIMENSION_NORM}.em"
EM_TEST_FILE_MIN="${EM_TEST_FILE_BASE}_${BITSIZE_MIN}_${DIMENSION_MIN}.em"

#
# For k2hveclinetool options
#
K2HVECCLI_CMD_FILE="${TESTDIR}/test_linetool.cmd"
K2HVECCLI_CMD_MASTER_LOG_FILE="${TESTDIR}/test_linetool.log"

K2HVECCLI_TEST_NAME_BASE="${TEST_NAME_BASE}"
K2HVECCLI_TEST_FILE_BASE="/tmp/.k2hveclinetool_${K2HVECCLI_TEST_NAME_BASE}"

# file: k2hveclinetool log
K2HVECCLI_CMD_LOG_FILE="${K2HVECCLI_TEST_FILE_BASE}.log"
K2HVECCLI_CMD_CVT_LOG_FILE="${K2HVECCLI_TEST_FILE_BASE}_normalized.log"

# file: k2hveclinetool common
K2HVECCLI_CMD_TMP_FILE_HISTORY="${K2HVECCLI_TEST_FILE_BASE}_history_save.his"
K2HVECCLI_CMD_TMP_FILE_DEBUG="${K2HVECCLI_TEST_FILE_BASE}_debug_logfile"

# file: create_projection_matrix
K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM="${K2HVECCLI_TEST_FILE_BASE}_create_projection_matrix_min.pm"
K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM="${K2HVECCLI_TEST_FILE_BASE}_create_projection_matrix_max.pm"
K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM="${K2HVECCLI_TEST_FILE_BASE}_create_projection_matrix_def.pm"

# file: create_embedding
K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM="${K2HVECCLI_TEST_FILE_BASE}_create_embedding_min.em"
K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM="${K2HVECCLI_TEST_FILE_BASE}_create_embedding_max.em"
K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM="${K2HVECCLI_TEST_FILE_BASE}_create_embedding_def.em"

# file: dump pm
K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM="${K2HVECCLI_TEST_FILE_BASE}_dump_min.pm"
K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM="${K2HVECCLI_TEST_FILE_BASE}_dump_max.pm"
K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM="${K2HVECCLI_TEST_FILE_BASE}_dump_def.pm"

# file: dump em
K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM="${K2HVECCLI_TEST_FILE_BASE}_dump_min.em"
K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM="${K2HVECCLI_TEST_FILE_BASE}_dump_max.em"
K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM="${K2HVECCLI_TEST_FILE_BASE}_dump_def.em"

# file: get embedding file
K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM="${K2HVECCLI_TEST_FILE_BASE}_get_pass_def.em"
K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM="${K2HVECCLI_TEST_FILE_BASE}_get_nopass_def.em"
K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM="${K2HVECCLI_TEST_FILE_BASE}_get_noattr_nopass_def.em"

# file: get em
K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM="${K2HVECCLI_TEST_FILE_BASE}_get_pass_em.em"
K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM="${K2HVECCLI_TEST_FILE_BASE}_get_nopass_em.em"
K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM="${K2HVECCLI_TEST_FILE_BASE}_get_noattr_nopass_em.em"

#
# For k2hveclinetool <key opration> options
#
K2HVECCLI_KEYOPS_CMD_FILENAME="test_linetool_keyops.cmd"
K2HVECCLI_KEYOPS_CMD_FILE="${TESTDIR}/${K2HVECCLI_KEYOPS_CMD_FILENAME}"
K2HVECCLI_KEYOPS_CMD_TMP_FILE="/tmp/.${K2HVECCLI_KEYOPS_CMD_FILENAME}"
K2HVECCLI_KEYOPS_CMD_MASTER_LOG_FILE="${TESTDIR}/test_linetool_keyops.log"

K2HVECCLI_KEYOPS_NAME_BASE="keyops"
K2HVECCLI_KEYOPS_FILE_BASE="/tmp/.k2hveclinetool_${K2HVECCLI_KEYOPS_NAME_BASE}"

# file: k2hveclinetool log
K2HVECCLI_KEYOPS_CMD_LOG_FILE="${K2HVECCLI_KEYOPS_FILE_BASE}.log"
K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE="${K2HVECCLI_KEYOPS_FILE_BASE}_normalized.log"

# file: keyops
K2HVECCLI_KEYOPS_FILE_BASE_EM="${K2HVECCLI_KEYOPS_FILE_BASE}_base.em"
K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM="${K2HVECCLI_KEYOPS_FILE_BASE}_slightly.em"
K2HVECCLI_KEYOPS_TMP_FILE="${K2HVECCLI_KEYOPS_FILE_BASE}_keyname.$$"

K2HVECCLI_CMD_TMP_FILES="
	${K2HVECCLI_CMD_LOG_FILE}
	${K2HVECCLI_CMD_CVT_LOG_FILE}
	${K2HVECCLI_CMD_TMP_FILE_HISTORY}
	${K2HVECCLI_CMD_TMP_FILE_DEBUG}
	${K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM}
	${K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM}
	${K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM}
	${K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM}
	${K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM}
	${K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM}
	${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM}
	${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM}
	${K2HVECCLI_KEYOPS_CMD_TMP_FILE}
	${K2HVECCLI_KEYOPS_CMD_LOG_FILE}
	${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}
	${K2HVECCLI_KEYOPS_FILE_BASE_EM}
	${K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM}
	${K2HVECCLI_KEYOPS_TMP_FILE}
"

#--------------------------------------------------------------
# Run
#--------------------------------------------------------------
START_DATE=$(date)
{
	echo "================================================================="
	echo "${CGRN}[START TEST]${CDEF} : ${START_DATE}"
	echo "================================================================="
	LOCAL_ERROR_COUNT=0

	#----------------------------------------------------------
	# Projection Matrix Generate : k2hvecpmtool
	#----------------------------------------------------------
	PRNTITLE "Test Projection Matrix Generate : k2hvecpmtool"

	PRNMSG "Test Projection Matrix Generate Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	if ({ "${PMGEN_TOOL}" --bit_size "${BITSIZE_MAX}" --dimension_size "${DIMENSION_MAX}" --name "${PM_CUSTOM_NAME_MAX}" --version "${PM_CUSTOM_VERSION_MAX}" --output "${PM_CUSTOM_FILE_MAX}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Test Projection Matrix Generate Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Test Projection Matrix Generate Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	fi

	PRNMSG "Test Projection Matrix Generate Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	if ({ "${PMGEN_TOOL}" --bit_size "${BITSIZE_NORM}" --dimension_size "${DIMENSION_NORM}" --name "${PM_CUSTOM_NAME_NORM}" --version "${PM_CUSTOM_VERSION_NORM}" --output "${PM_CUSTOM_FILE_NORM}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Test Projection Matrix Generate Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Test Projection Matrix Generate Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	fi

	PRNMSG "Test Projection Matrix Generate Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	if ({ "${PMGEN_TOOL}" --bit_size "${BITSIZE_MIN}" --dimension_size "${DIMENSION_MIN}" --name "${PM_CUSTOM_NAME_MIN}" --version "${PM_CUSTOM_VERSION_MIN}" --output "${PM_CUSTOM_FILE_MIN}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Test Projection Matrix Generate Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Test Projection Matrix Generate Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	fi

	#----------------------------------------------------------
	# Projection Matrix Save/Load/Verify : k2hvec_pmtest
	#----------------------------------------------------------
	PRNTITLE "Projection Matrix Save/Load/Verify : k2hvec_pmtest"

	PRNMSG "Test Builtin Project Matrix(Save/Load/Verify)"
	if ({ "${PMTEST_TOOL}" builtin "${PMTEST_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Test Builtin Project Matrix"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Test Builtin Project Matrix"
	fi

	PRNMSG "Test Custom Project Matrix(Save/Load/Verify)"
	if ({ "${PMTEST_TOOL}" custom "${PMTEST_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Test Custom Project Matrix"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Test Custom Project Matrix"
	fi

	#----------------------------------------------------------
	# Binary Hash and Embedding: k2hvec_bhtest
	#----------------------------------------------------------
	PRNTITLE "Binary Hash and Embedding : k2hvec_bhtest"

	# Create
	PRNMSG "Create Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	if ({ "${BHTEST_TOOL}" create --projection_matrix "${PM_CUSTOM_FILE_MAX}" --output "${EM_TEST_FILE_MAX}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Create Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Create Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	fi

	PRNMSG "Create Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	if ({ "${BHTEST_TOOL}" create --projection_matrix "${PM_CUSTOM_FILE_NORM}" --output "${EM_TEST_FILE_NORM}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Create Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Create Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	fi

	PRNMSG "Create Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	if ({ "${BHTEST_TOOL}" create --projection_matrix "${PM_CUSTOM_FILE_MIN}" --output "${EM_TEST_FILE_MIN}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Create Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Create Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	fi

	# Verify
	PRNMSG "Verify Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	if ({ "${BHTEST_TOOL}" verify --projection_matrix "${PM_CUSTOM_FILE_MAX}" --input "${EM_TEST_FILE_MAX}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Verify Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Verify Embedding/Binary Hash Max(Bitsize=${BITSIZE_MAX}, Dimension=${DIMENSION_MAX})"
	fi

	PRNMSG "Verify Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	if ({ "${BHTEST_TOOL}" verify --projection_matrix "${PM_CUSTOM_FILE_NORM}" --input "${EM_TEST_FILE_NORM}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Verify Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Verify Embedding/Binary Hash Norm(Bitsize=${BITSIZE_NORM}, Dimension=${DIMENSION_NORM})"
	fi

	PRNMSG "Verify Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	if ({ "${BHTEST_TOOL}" verify --projection_matrix "${PM_CUSTOM_FILE_MIN}" --input "${EM_TEST_FILE_MIN}" "${PMGEN_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Verify Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Verify Embedding/Binary Hash Min(Bitsize=${BITSIZE_MIN}, Dimension=${DIMENSION_MIN})"
	fi

	#----------------------------------------------------------
	# Calculation of Cosine Similarity : k2hvec_costest
	#----------------------------------------------------------
	PRNTITLE "Calculation of Cosine Similarity : k2hvec_costest"

	PRNMSG "Calculation of Cosine Similarity (create 2 unnormalized embedding with 4096 dimention)"
	if ({ "${COSTEST_TOOL}" "${COSTEST_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Calculation of Cosine Similarity (create 2 unnormalized embedding with 4096 dimention)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Calculation of Cosine Similarity (create 2 unnormalized embedding with 4096 dimention)"
	fi

	PRNMSG "Calculation of Cosine Similarity (create 2 normalized embedding with 4096 dimention)"
	if ({ "${COSTEST_TOOL}" --simple "${COSTEST_TOOL_DBG_OPT}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Calculation of Cosine Similarity (create 2 normalized embedding with 4096 dimention)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Calculation of Cosine Similarity (create 2 normalized embedding with 4096 dimention)"
	fi

	#----------------------------------------------------------
	# Run test commands : k2hveclinetool
	#----------------------------------------------------------
	#
	# Use k2hveclinetool to execute command tests using a command file.
	#
	PRNTITLE "Run test commands : k2hveclinetool"

	#
	# Run options:	--memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping
	#				--binary_hash_bit_size 4
	#				--neighbor_key_limit 2000
	#				--content_direct_limit 32
	#				--debug_level wan
	#				--history_max 10000
	#				--run_file "${K2HVECCLI_CMD_FILE}"
	#
	PRNMSG "Run test standard commands (1)"
	if ({ "${K2HVECCLI_TOOL}" --memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping --binary_hash_bit_size 4 --neighbor_key_limit 2000 --content_direct_limit 32 --debug_level wan --history_max 10000 --run_file "${K2HVECCLI_CMD_FILE}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | tee "${K2HVECCLI_CMD_LOG_FILE}" | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Run test standard commands (1)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		#
		# Compare files
		#
		CMP_LOCAL_ERROR_COUNT=0

		# file: log file
		ConvertValiabelsInTestLineToolResult "${K2HVECCLI_CMD_LOG_FILE}" "${K2HVECCLI_CMD_CVT_LOG_FILE}"

		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_MASTER_LOG_FILE}" "${K2HVECCLI_CMD_CVT_LOG_FILE}" 2>&1); then
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'

			PRNERR "Different: ${K2HVECCLI_CMD_MASTER_LOG_FILE} vs ${K2HVECCLI_CMD_CVT_LOG_FILE}"
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_MASTER_LOG_FILE} vs ${K2HVECCLI_CMD_CVT_LOG_FILE}"
		fi

		# file: others
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM}"
		CutCreateDateInFile "${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM}"

		# file: create_projection_matrix
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CPM_MIN_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_PM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CPM_MAX_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_PM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CPM_DEF_PM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_PM}"
		fi

		# file: create_embedding
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CEM_MIN_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MIN_EM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CEM_MAX_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_MAX_EM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM}" "${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_CEM_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_DUMP_DEF_EM}"
		fi

		# file: get embedding file
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM}" "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM}" "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM}"
		fi

		# file: get em
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM}" "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_GET_NOPASS_DEF_EM_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}"
		fi
		if ! DIFF_RESULT=$(diff "${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM}" "${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}" 2>&1); then
			PRNERR "Different: ${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}"
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_CMD_TMP_FILE_GET_NOATTR_NOPASS_DEF_EM_EM} vs ${K2HVECCLI_CMD_TMP_FILE_GET_PASS_DEF_EM_EM}"
		fi

		#
		# Check Result
		#
		echo ""
		if [ "${CMP_LOCAL_ERROR_COUNT}" -ne 0 ]; then
			PRNFAILURE "Run test standard commands (1)"
			LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
		else
			PRNSUCCESS "Run test standard commands (1)"
		fi
	fi

	#----------------------------------------------------------
	# Run test commands using key : k2hveclinetool
	#----------------------------------------------------------
	#
	# Use k2hveclinetool to execute command tests using a command file.
	# This test is for using k2hash key.
	#
	PRNTITLE "Run test commands using key : k2hveclinetool"

	#
	# Prepare two embedding files
	#
	PRNMSG "Preparation: Creating Embedding Data File(1)"
	if ({ "${K2HVECCLI_TOOL}" --memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping --binary_hash_bit_size 4 --neighbor_key_limit 2000 --content_direct_limit 32 --debug_level wan --history_max 10000 -- create_embedding output="${K2HVECCLI_KEYOPS_FILE_BASE_EM}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Preparation: Creating Embedding Data File(1)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Preparation: Creating Embedding Data File(1)"
	fi

	PRNMSG "Preparation: Creating Slightly Embedding Data File(2)"
	if ! CreateSlightlyDifferentEmbeddingFile "${K2HVECCLI_KEYOPS_FILE_BASE_EM}" "${K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM}" 64; then
		PRNFAILURE "Preparation: Creating Slightly Embedding Data File(2)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Preparation: Creating Slightly Embedding Data File(2)"
	fi

	#
	# Get main key name for base embedding data(file)
	#
	PRNMSG "Preparation: Get main key name for base embedding data(file)"
	K2HVECCLI_KEYOPS_KEYNAME=""
	if ({ "${K2HVECCLI_TOOL}" --memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping --binary_hash_bit_size 4 --neighbor_key_limit 2000 --content_direct_limit 32 --debug_level wan --history_max 10000 -- create_k2hash_key "${K2HVECCLI_KEYOPS_FILE_BASE_EM}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | grep 'K2hash main key' | awk '{print $5}' > "${K2HVECCLI_KEYOPS_TMP_FILE}") && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Preparation: Get main key name for base embedding data(file)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		if [ ! -f "${K2HVECCLI_KEYOPS_TMP_FILE}" ]; then
			PRNFAILURE "Preparation: Get main key name for base embedding data(file): not found main key name"
			LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
		else
			K2HVECCLI_KEYOPS_KEYNAME=$(tr -d '\n' < "${K2HVECCLI_KEYOPS_TMP_FILE}")
			rm -f "${K2HVECCLI_KEYOPS_TMP_FILE}" >/dev/null 2>&1

			if [ -z "${K2HVECCLI_KEYOPS_KEYNAME}" ]; then
				PRNFAILURE "Preparation: Get main key name for base embedding data(file): not found main key name"
				LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
			else
				PRNSUCCESS "Preparation: Get main key name for base embedding data(file)"
			fi
		fi
	fi

	#
	# Generate command file from template
	#
	PRNMSG "Preparation: Generate command file from template"
	if ! sed -e "s|__K2HVECCLI_KEYOPS_FILE_BASE_EM__|${K2HVECCLI_KEYOPS_FILE_BASE_EM}|g" -e "s|__K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM__|${K2HVECCLI_KEYOPS_FILE_SLIGHTLY_EM}|g" -e "s|__K2HVECCLI_KEYOPS_KEYNAME__|${K2HVECCLI_KEYOPS_KEYNAME}|g" "${K2HVECCLI_KEYOPS_CMD_FILE}" > "${K2HVECCLI_KEYOPS_CMD_TMP_FILE}" 2>&1; then
		PRNFAILURE "Preparation: Generate command file from template"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		PRNSUCCESS "Preparation: Generate command file from template"
	fi

	#
	# Run k2hveclinetool for key oprations
	#
	# Run options:	--memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping
	#				--binary_hash_bit_size 4
	#				--neighbor_key_limit 2000
	#				--content_direct_limit 32
	#				--debug_level wan
	#				--history_max 10000
	#				--run_file "${K2HVECCLI_KEYOPS_CMD_TMP_FILE}"
	#
	PRNMSG "Run test key opration commands (2)"
	if ({ "${K2HVECCLI_TOOL}" --memory --mask 4 --collision_mask 2 --element_count 4 --page_size 128 --full_mapping --binary_hash_bit_size 4 --neighbor_key_limit 2000 --content_direct_limit 32 --debug_level wan --history_max 10000 --run_file "${K2HVECCLI_KEYOPS_CMD_TMP_FILE}" 2>&1 || echo > "${PIPEFAILURE_FILE}"; } | tee "${K2HVECCLI_KEYOPS_CMD_LOG_FILE}" | sed -e 's/^/    /g') && rm "${PIPEFAILURE_FILE}" >/dev/null 2>&1; then
		PRNFAILURE "Run test key opration commands (2)"
		LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
	else
		#
		# Compare files
		#
		CMP_LOCAL_ERROR_COUNT=0

		# file: log file
		ConvertValiabelsInTestLineToolKeyOpsResult "${K2HVECCLI_KEYOPS_CMD_LOG_FILE}" "${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}"

		if ! DIFF_RESULT=$(diff "${K2HVECCLI_KEYOPS_CMD_MASTER_LOG_FILE}" "${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}" 2>&1); then
			echo "${DIFF_RESULT}" | sed -e 's/^/    /g'

			PRNERR "Different: ${K2HVECCLI_KEYOPS_CMD_MASTER_LOG_FILE} vs ${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}"
			CMP_LOCAL_ERROR_COUNT=$((CMP_LOCAL_ERROR_COUNT + 1))
		else
			PRNINFO "No difference: ${K2HVECCLI_KEYOPS_CMD_MASTER_LOG_FILE} vs ${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}"
		fi

		#
		# Check Result
		#
		echo ""
		if [ "${CMP_LOCAL_ERROR_COUNT}" -ne 0 ]; then
			PRNFAILURE "Run test key opration commands (2)"
			LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
		else
			PRNSUCCESS "Run test key opration commands (2)"
		fi
	fi

	#----------------------------------------------------------
	# Update master log files
	#----------------------------------------------------------
	if [ "${UPDATE_LOG_FILE}" -eq 1 ]; then
		PRNMSG "Update master log file"
		if ! cp "${K2HVECCLI_CMD_CVT_LOG_FILE}" "${K2HVECCLI_CMD_MASTER_LOG_FILE}" >/dev/null 2>&1; then
			PRNFAILURE "Update master log file"
			LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
		else
			PRNSUCCESS "Update master log file"
		fi

		PRNMSG "Update master keyops log file"
		if ! cp "${K2HVECCLI_KEYOPS_CMD_CVT_LOG_FILE}" "${K2HVECCLI_KEYOPS_CMD_MASTER_LOG_FILE}" >/dev/null 2>&1; then
			PRNFAILURE "Update master keyops log file"
			LOCAL_ERROR_COUNT=$((LOCAL_ERROR_COUNT + 1))
		else
			PRNSUCCESS "Update master keyops log file"
		fi
	fi

	#----------------------------------------------------------
	# Remove files
	#----------------------------------------------------------
	if [ "${DEBUG_MODE}" -eq 0 ] && [ "${LOCAL_ERROR_COUNT}" -eq 0 ]; then
		PRNMSG "Delete the file containing the work results"

		for _cmd_tmp_file in ${K2HVECCLI_CMD_TMP_FILES}; do
			rm -f "${_cmd_tmp_file}" > /dev/null 2>&1
		done

		rm -f "${PM_CUSTOM_FILE_MAX}" "${PM_CUSTOM_FILE_NORM}" "${PM_CUSTOM_FILE_MIN}" "${EM_TEST_FILE_MAX}" "${EM_TEST_FILE_NORM}" "${EM_TEST_FILE_MIN}" >/dev/null 2>&1

		PRNSUCCESS "Delete the file containing the work results"

	elif [ "${LOCAL_ERROR_COUNT}" -eq 0 ]; then
		PRNWARN "Errors occurred during execution, so the file containing the work results has been left without being deleted"

	else
		PRNINFO "Debug mode is enable, so the file containing the work results has been left without being deleted"
	fi

	#----------------------------------------------------------
	# Print Result
	#----------------------------------------------------------
	END_DATE=$(date)
	if [ "${LOCAL_ERROR_COUNT}" -eq 0 ]; then
		echo "================================================================="
		echo "${CGRN}[FINISH TEST]${CDEF} : ${START_DATE} - ${END_DATE}"
		echo "================================================================="
		echo "${CGRN}[SUCCEED]${CDEF} All tests completed successfully."
	else
		echo "================================================================="
		echo "${CRED}[FINISH TEST]${CDEF} : ${START_DATE} - ${END_DATE}"
		echo "================================================================="
		echo "${CRED}[FAILED]${CDEF} ${LOCAL_ERROR_COUNT} tests have failed."
	fi
	echo ""

	echo "${LOCAL_ERROR_COUNT}" > "${ERROR_COUNT_FILE}"
} 2>&1 | tee "${LOG_FIFO_FILE}"

#--------------------------------------------------------------
# Finish
#--------------------------------------------------------------
if [ -f "${ERROR_COUNT_FILE}" ]; then
	ERROR_COUNT=$(tr -d '\n' < "${ERROR_COUNT_FILE}")
	rm -f "${ERROR_COUNT_FILE}" >/dev/null 2>&1
else
	ERROR_COUNT=1
fi
if [ "${ERROR_COUNT}" -ne 0 ]; then
	exit 1
fi

exit 0

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
