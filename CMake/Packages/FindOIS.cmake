# - Try to find OIS
# Once done, this will define
#
#  OIS_FOUND - system has OIS
#  OIS_INCLUDE_DIRS - the OIS include directories 
#  OIS_LIBRARIES - link these to use OIS

include(FindPkgMacros)
findpkg_begin(OIS)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(OIS_HOME)
getenv_path(iRobot_HOME)
getenv_path(iRobot_SOURCE)

# construct search paths

set(OIS_PREFIX_PATH   
	${iRobot_DEPENDENCIES_DIR}/ogredeps/src/ois/includes
	${iRobot_DEPENDENCIES_DIR}/build/ogredeps
)

#create_search_paths(OIS)
#直接设置搜索位置
set(OIS_INC_SEARCH_PATH ${iRobot_DEPENDENCIES_DIR}/ogredeps/src/ois/includes)
set(OIS_LIB_SEARCH_PATH ${iRobot_DEPENDENCIES_DIR}/build/ogredeps/ogredeps/lib)

# redo search if prefix path changed
clear_if_changed(OIS_PREFIX_PATH
  OIS_LIBRARY_FWK
  OIS_LIBRARY_REL
  OIS_LIBRARY_DBG
  OIS_INCLUDE_DIR
)

set(OIS_LIBRARY_NAMES_REL OIS)
set(OIS_LIBRARY_NAMES_DBG OIS)

use_pkgconfig(OIS_PKGC OIS)

findpkg_framework(OIS)
find_path(OIS_INCLUDE_DIR NAMES OIS.h HINTS ${OIS_INC_SEARCH_PATH} ${OIS_PKGC_INCLUDE_DIRS})
find_library(OIS_LIBRARY_REL NAMES ${OIS_LIBRARY_NAMES_REL} HINTS ${OIS_LIB_SEARCH_PATH} ${OIS_PKGC_LIBRARY_DIRS} PATH_SUFFIXES release)
find_library(OIS_LIBRARY_DBG NAMES ${OIS_LIBRARY_NAMES_DBG} HINTS ${OIS_LIB_SEARCH_PATH} ${OIS_PKGC_LIBRARY_DIRS} PATH_SUFFIXES debug)
make_library_set(OIS_LIBRARY)

findpkg_finish(OIS)
add_parent_dir(OIS_INCLUDE_DIRS OIS_INCLUDE_DIR)

