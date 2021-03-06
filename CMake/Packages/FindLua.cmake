# - Try to find Lua
# Once done, this will define
#
#  Lua_FOUND - system has Lua
#  Lua_INCLUDE_DIRS - the Lua include directories 
#  Lua_LIBRARIES - link these to use Lua

include(FindPkgMacros)
findpkg_begin(Lua)

# Get path, convert backslashes as ${ENV_${var}}

# construct search paths
set(Lua_PREFIX_PATH 
${iRobot_DEPENDENCIES_DIR}/lua
${iRobot_DEPENDENCIES_DIR}/build/lua
${Lua_SOURCE} 
${Lua_BUILD}
${Lua_SOURCE}/trunk 
${Lua_SOURCE}/trunk/include
${Lua_SOURCE}/trunk/lib
)

#create_search_paths(Lua)
# redo search if prefix path changed
clear_if_changed(
  Lua_LIBRARY_FWK
  Lua_LIBRARY_REL
  Lua_LIBRARY_DBG
  Lua_INCLUDE_DIR
  Lua_OGRPLATFORM_REL
  Lua_OGRPLATFORM_DBG
  Lua_OGRPLATFORM_FWK
)
create_search_paths(Lua)

set(Lua_LIBRARY_NAMES_REL Lua)
set(Lua_LIBRARY_NAMES_DBG Lua)

use_pkgconfig(Lua_PKGC Lua)

findpkg_framework(Lua)

set(Lua_INC_SEARCH_PATH ${Lua_PREFIX_PATH})
set(Lua_LIB_SEARCH_PATH ${Lua_PREFIX_PATH})

#message(STATUS "library search path:"${Lua_INC_SEARCH_PATH})
#foreach(dir ${Lua_INC_SEARCH_PATH})
#	message(STATUS ${dir})
#endforeach(dir)
set(NO_DEFAULT_PATH 1)
find_path(Lua_INCLUDE_DIR NAMES lua.h HINTS ${Lua_INC_SEARCH_PATH} ${Lua_PKGC_INCLUDE_DIRS} PATH_SUFFIXES "" src)

find_library(Lua_LIBRARY_REL NAMES ${Lua_LIBRARY_NAMES_REL} HINTS ${Lua_LIB_SEARCH_PATH} ${Lua_PKGC_LIBRARY_DIRS} PATH_SUFFIXES release
		lib/release lib/Release    NO_CMAKE_SYSTEM_PATH )
find_library(Lua_LIBRARY_DBG NAMES ${Lua_LIBRARY_NAMES_DBG} HINTS ${Lua_LIB_SEARCH_PATH} ${Lua_PKGC_LIBRARY_DIRS} PATH_SUFFIXES debug
		lib/Debug lib/debug NO_CMAKE_SYSTEM_PATH)

make_library_set(Lua_LIBRARY)

findpkg_finish(Lua)
add_parent_dir(Lua_INCLUDE_DIRS Lua_INCLUDE_DIR)
set(Lua_LIBRARIES ${Lua_LIBRARIES})
set(Lua_INCLUDE_DIR ${Lua_INCLUDE_DIR})
