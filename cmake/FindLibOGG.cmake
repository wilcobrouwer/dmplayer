find_package(ExtraLibDir)

FIND_LIB_IN_EXTRALIBS(LIBOGG *ogg* include lib)

SET(LIBOGG_FOUND FALSE)
SET(LIBOGG_DEBUG_FOUND FALSE)

Find_Path(LIBOGG_INCLUDE_DIR
	NAMES ogg/ogg.h
	PATHS /usr/include /usr/local/include ${LIBOGG_EXTRALIB_INCLUDE_PATHS}
)

Find_Library(LIBOGG_LIBRARY
	NAMES ogg
	PATHS /usr/lib /usr/local/lib ${LIBOGG_EXTRALIB_LIBRARY_PATHS}
)

Find_Library(LIBOGG_DEBUG_LIBRARY
	NAMES oggd
	PATHS /usr/lib /usr/local/lib ${LIBOGG_EXTRALIB_LIBRARY_PATHS}
)

IF(NOT MSVC) # Assume non-MSVC compilers don't care about different runtimes
	SET(LIBOGG_DEBUG_LIBRARY ${LIBOGG_LIBRARY})
ENDIF(NOT MSVC)

IF(LIBOGG_INCLUDE_DIR AND LIBOGG_LIBRARY AND LIBOGG_DEBUG_LIBRARY)
	SET(LIBOGG_FOUND TRUE)
	SET(LIBOGG_DEBUG_FOUND TRUE)
	MESSAGE(STATUS "Found ogg library")
ENDIF(LIBOGG_INCLUDE_DIR AND LIBOGG_LIBRARY AND LIBOGG_DEBUG_LIBRARY)
