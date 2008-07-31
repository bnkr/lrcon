# Build recursively in src
SUBDIRS = src include 
EXTRA_DIST = doc ${DOXYFILE} examples version name README.documentation README.cross-compiling

# Necessary or it won't notice the extra scripts
ACLOCAL_AMFLAGS = -I build-aux

# For some reason it won't work if you define doc: doxygen, however this means that
# you must 'mkdir doc' before it will build properly.
dist-hook: 	doxygen

# This is build by dist so it's a relative path.
distclean-local:
		rm -rf doc/*

# To reconfigure doxygen dynamically (remember to suffix these with a semicolon)
DOXYGEN_INPUTS = "${top_srcdir}/include/ ${top_srcdir}/src/"
DOXYGEN_CONFS = cat ${DOXYFILE}; \
                echo "EXAMPLE_PATH = ${top_srcdir}/examples/"; \
                echo "INPUT = ${DOXYGEN_INPUTS}"; \
                echo "PROJECT_NUMBER = " `cat ${top_srcdir}/version`; \
                echo "PROJECT_NAME = " `cat ${top_srcdir}/name`

# Should be dynamic, and also while this does work, I think it would fail whenever
# there are subdirectories of html/, which there can be under some configurations.
#
#   cat Doxyfile.default  | grep 'HTML_OUTPUT\s*=' | awk '{ print $3 }'
# 
# btw, the docdir itself could be configured just as easily this way but it probably 
# shouldn't be.
html_DATA = ${top_srcdir}/doc/html/*

# Does doxygen and additionally ignores any undocumented parameter warnings.
# These pop up if you have only documented a subset of the parameters of a
# function.  Also, for some reason it seems to return an error most of the
# time so we ignore it (maybe grep of an empty string?).
doxygen:	${DOXYFILE}
		(( ${DOXYGEN_CONFS}; ) \
		| doxygen - > /dev/null) 2>&1 \
		| grep -v 'Warning: The following parameters of' | grep -v "^\s*parameter" || true

# Gives errors for anything which is undocumented.
doxygen-paranoid:	${DOXYFILE}
			( ${DOXYGEN_CONFS}; echo "WARN_NO_PARAMDOC = YES";  echo "WARN_IF_UNDOCUMENTED = YES") | doxygen - > /dev/null


