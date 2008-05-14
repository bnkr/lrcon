# Build recursively in src
SUBDIRS = src include 
EXTRA_DIST = docs Doxyfile.dev-dotty examples

# So that dox are built before distribution, as well as letting the user
# rebuild them.
dist-hook:	doxygen

# This is build by dist so it's a relative path.
distclean-local:
		rm -rfv docs/*

doxygen:	${srcdir}/Doxyfile.dev-dotty
		doxygen ${srcdir}/Doxyfile.dev-dotty > /dev/null
