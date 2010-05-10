# $Id$

# find the ANN library 
#
# Joerg F. Unger, Northwestern University Evanston IL, April 2010
#
# variables used by this module (can be also defined as environment variables):
#   ANN_ROOT - preferred installation prefix for searching for ANN
#   ANN_FIND_STATIC_LIBRARY - searches for static libraries (UNIX only)
#   ANN_DEBUG - print debug messages
#
# variables defined by this module
#   ANN_FOUND - defines whether metis was found or not
#   ANN_INCLUDE_DIR - ANN include directory
#   ANN_LIBRARIES   - ANN libraries


# initialize variables
MESSAGE(STATUS "Checking for ANN Library ...")
# check if ANN_ROOT is set
IF(NOT ANN_ROOT AND NOT $ENV{ANN_ROOT} STREQUAL "")
  SET(ANN_ROOT $ENV{ANN_ROOT})
ENDIF(NOT ANN_ROOT AND NOT $ENV{ANN_ROOT} STREQUAL "")

# convert path to unix style path and set search path
IF(ANN_ROOT)
  FILE(TO_CMAKE_PATH ${ANN_ROOT} ANN_ROOT)
  SET(_ANN_INCLUDE_SEARCH_DIRS ${ANN_ROOT}/include ${ANN_ROOT} ${_ANN_INCLUDE_SEARCH_DIRS})
  SET(_ANN_LIBRARIES_SEARCH_DIRS ${ANN_ROOT}/lib ${ANN_ROOT} ${_ANN_LIBRARIES_SEARCH_DIRS})
ENDIF(ANN_ROOT)

# search for header ANN.h
FIND_PATH(ANN_INCLUDE_DIR NAMES ANN/ANN.h HINTS ${_ANN_INCLUDE_SEARCH_DIRS})

# search for ann library
IF(UNIX AND ANN_FIND_STATIC_LIBRARY)
  SET(ANN_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
ENDIF(UNIX AND ANN_FIND_STATIC_LIBRARY)
FIND_LIBRARY(ANN_LIBRARIES NAMES ANN HINTS ${_ANN_LIBRARIES_SEARCH_DIRS})
IF(UNIX AND ANN_FIND_STATIC_LIBRARY)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ${ANN_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(UNIX AND ANN_FIND_STATIC_LIBRARY)

# handle the QUIETLY and REQUIRED arguments
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ANN DEFAULT_MSG ANN_LIBRARIES ANN_INCLUDE_DIR)

if(ANN_DEBUG)
  MESSAGE(STATUS "ANN_FOUND=${ANN_FOUND}")
  MESSAGE(STATUS "ANN_INCLUDE_DIR=${ANN_INCLUDE_DIR}")
  MESSAGE(STATUS "ANN_LIBRARIES=${ANN_LIBRARIES}")
endif(ANN_DEBUG)

