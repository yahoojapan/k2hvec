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

bin_PROGRAMS			=	k2hveclinetool			\
							k2hvecpmtool

noinst_PROGRAMS			=	k2hvec_bhtest			\
							k2hvec_costest			\
							k2hvec_pmtest

k2hveclinetool_SOURCES	=	k2hveclinetool.cc		\
							k2hvec_tools_common.h	\
							k2hvec_cli_input.cc		\
							k2hvec_cli_input.h		\
							k2hvec_cli_cmd.cc		\
							k2hvec_cli_cmd.h		\
							k2hvec_cli_promptcmd.cc	\
							k2hvec_cli_promptcmd.h
k2hveclinetool_LDADD	=	$(k2hash_LIBS) -L../lib/.libs -lk2hvec

k2hvecpmtool_SOURCES	=	k2hvecpmtool.cc		\
							k2hvec_tools_common.h
k2hvecpmtool_LDADD		=	$(k2hash_LIBS) -L../lib/.libs -lk2hvec

k2hvec_bhtest_SOURCES	=	k2hvec_bhtest.cc		\
							k2hvec_tools_common.h
k2hvec_bhtest_LDADD		=	$(k2hash_LIBS) -L../lib/.libs -lk2hvec

k2hvec_costest_SOURCES	=	k2hvec_costest.cc		\
							k2hvec_tools_common.h
k2hvec_costest_LDADD	=	$(k2hash_LIBS) -L../lib/.libs -lk2hvec

k2hvec_pmtest_SOURCES	=	k2hvec_pmtest.cc		\
							k2hvec_tools_common.h
k2hvec_pmtest_LDADD		=	$(k2hash_LIBS) -L../lib/.libs -lk2hvec

ACLOCAL_AMFLAGS			=	-I m4

# [NOTE]
# "-Waddress-of-packed-member" optsion was introduced by default
# from GCC 9.
# Knowing that packed structure is CPU architecture dependent,
# this program ignores this warning.
#
# The flags "-fno-math-errno", "-freciprocal-math", and
# "-fno-trapping-math" are specified to optimize and accelerate
# floating-point operations. These must be passed consistently
# not only during compilation but also during linking.
#
AM_CPPFLAGS				=	-DK2HVEC -I$(top_srcdir)/lib
AM_CFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math -Wno-address-of-packed-member
AM_CXXFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math -Wno-address-of-packed-member
AM_LDFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math

TESTS					=	test.sh

EXTRA_DIST				=	test.sh						\
							test_linetool.cmd			\
							test_linetool_keyops.cmd	\
							test_linetool.log			\
							test_linetool_keyops.log

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
