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

pkginclude_HEADERS		=	k2hveccommon.h		\
							k2hvecdbg.h			\
							k2hvectypes.h		\
							k2hvecutil.h		\
							k2hvec.h			\
							k2hvecdb.h			\
							k2hvecmath.h		\
							k2hvecvalue.h		\
							k2hvecbuiltinpm.h

pkgincludedir			=	$(includedir)/k2hvec
pkgconfigdir			=	$(libdir)/pkgconfig
pkgconfig_DATA			=	libk2hvec.pc

DISTCLEANFILES			=	$(pkgconfig_DATA)

lib_LTLIBRARIES			=	libk2hvec.la

libk2hvec_la_SOURCES	=	k2hvecdbg.cc		\
							k2hvecutil.cc		\
							k2hvec.cc			\
							k2hvecdb.cc			\
							k2hvecmath.cc		\
							k2hvecvalue.cc		\
							k2hvecbuiltinpm.cc	\
							k2hvecversion.cc

libk2hvec_la_LDFLAGS	=	-version-info $(LIB_VERSION_INFO)
libk2hvec_la_LIBADD		=	$(fullock_LIBS) $(k2hash_LIBS)

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
AM_CPPFLAGS				=	-DK2HVEC
AM_CFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math -Wno-address-of-packed-member
AM_CXXFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math -Wno-address-of-packed-member
AM_LDFLAGS				=	-fno-math-errno -freciprocal-math -fno-trapping-math

### version(commit hash)
.PHONY:	k2hvecversion

k2hvecversion.cc:	k2hvecversion
	@../buildutils/make_commit_hash_source.sh -dch @CURRENTREV@ k2hvecversion.cc k2hvec_commit_hash

#
# Local variables:
# tab-width: 4
# c-basic-offset: 4
# End:
# vim600: noexpandtab sw=4 ts=4 fdm=marker
# vim<600: noexpandtab sw=4 ts=4
#
