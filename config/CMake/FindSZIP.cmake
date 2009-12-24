# - Find szip
# Find the native SZIP includes and library
#
#  SZIP_INCLUDE_DIRS - where to find szip.h, etc.
#  SZIP_LIBRARIES    - List of libraries when using szip.
#  SZIP_DLL          - List of DLLs when using szip.
#  SZIP_FOUND        - True if szip found.

IF (SZIP_INCLUDE_DIR)
  # Already in cache, be silent
  SET(SZIP_FIND_QUIETLY TRUE)
ENDIF (SZIP_INCLUDE_DIR)

FIND_PATH(SZIP_INCLUDE_DIR szlib.h PATHS "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/opt/include")

SET(SZIP_NAMES szip)
FIND_LIBRARY(SZIP_LIBRARY NAMES ${SZIP_NAMES} PATHS "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/opt/lib")

FIND_FILE(SZIP_DLL szip.dll "$ENV{HOMEDRIVE}$ENV{HOMEPATH}/opt/bin")

MARK_AS_ADVANCED( SZIP_LIBRARY SZIP_INCLUDE_DIR SZIP_DLL )

# Per-recommendation
SET(SZIP_INCLUDE_DIRS "${SZIP_INCLUDE_DIR}")
SET(SZIP_LIBRARIES    "${SZIP_LIBRARY}")
SET(SZIP_DLLS         "${SZIP_DLL}")

# handle the QUIETLY and REQUIRED arguments and set SZIP_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SZIP DEFAULT_MSG SZIP_LIBRARIES SZIP_INCLUDE_DIRS)
