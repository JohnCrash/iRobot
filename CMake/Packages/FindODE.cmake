# - Try to find ODE
# Once done, this will define
#
#  ODE_FOUND - system has ODE
#  ODE_INCLUDE_DIRS - the ODE include directories 
#  ODE_LIBRARIES - link these to use ODE

include(FindPkgMacros)
findpkg_begin(ODE)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(ODE_SOURCE)
getenv_path(ODE_BUILD)
getenv_path(iRobot_HOME)
getenv_path(iRobot_SOURCE)

# construct search paths
set(ODE_PREFIX_PATH 
${iRobot_DEPENDENCIES_DIR}/ode
${iRobot_DEPENDENCIES_DIR}/build/ode
${ODE_SOURCE} 
${ODE_BUILD}
${ODE_SOURCE}/trunk ${ODE_SOURCE}/trunk/include
${ODE_SOURCE}/trunk/lib
)

create_search_paths(ODE)
#set(ODE_INC_SEARCH_PATH ${ODE_PREFIX_PATH})
#set(ODE_LIB_SEARCH_PATH ${ODE_PREFIX_PATH})

#message(STATUS "FindLua ODE_SOURCE=${ODE_INC_SEARCH_PATH}")

#create_search_paths(ODE)
# redo search if prefix path changed
clear_if_changed(
  ODE_LIBRARY_FWK
  ODE_LIBRARY_REL
  ODE_LIBRARY_DBG
  ODE_INCLUDE_DIR
  ODE_OGRPLATFORM_REL
  ODE_OGRPLATFORM_DBG
  ODE_OGRPLATFORM_FWK
)

if(APPLE)
	set(ODE_LIBRARY_NAMES ode)
	set(ODE_LIBRARY_NAMES_DBG ode)
else()
	set(ODE_LIBRARY_NAMES ode)
	get_debug_names(ODE_LIBRARY_NAMES)
endif()

use_pkgconfig(ODE_PKGC ODE)

findpkg_framework(ODE)

#message(STATUS "library search path:"${ODE_LIB_SEARCH_PATH})
#foreach(dir ${ODE_LIB_SEARCH_PATH})
#	message(STATUS ${dir})
#endforeach(dir)
find_path(ODE_INCLUDE_DIR NAMES ode/ode.h HINTS ${ODE_INC_SEARCH_PATH} ${ODE_PKGC_INCLUDE_DIRS} PATH_SUFFIXES include)

find_library(ODE_LIBRARY_REL NAMES ${ODE_LIBRARY_NAMES} HINTS ${ODE_LIB_SEARCH_PATH} ${ODE_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Release
		lib/release lib/Release)
find_library(ODE_LIBRARY_DBG NAMES ${ODE_LIBRARY_NAMES_DBG} HINTS ${ODE_LIB_SEARCH_PATH} ${ODE_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Debug
		lib/Debug lib/debug)

make_library_set(ODE_LIBRARY)

findpkg_finish(ODE)
add_parent_dir(ODE_INCLUDE_DIRS ODE_INCLUDE_DIR)
set(ODE_LIBRARIES ${ODE_LIBRARIES})
set(ODE_INCLUDE_DIR ${ODE_INCLUDE_DIR})
