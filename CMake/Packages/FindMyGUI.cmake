# - Try to find MyGUI
# Once done, this will define
#
#  MyGUI_FOUND - system has MyGUI
#  MyGUI_INCLUDE_DIRS - the MyGUI include directories 
#  MyGUI_LIBRARIES - link these to use MyGUI

include(FindPkgMacros)
findpkg_begin(MyGUI)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(MyGUI_SOURCE)
getenv_path(MyGUI_BUILD)
getenv_path(iRobot_HOME)
getenv_path(iRobot_SOURCE)

# construct search paths
set(MyGUI_PREFIX_PATH 
  ${MyGUI_SOURCE} 
  ${MyGUI_BUILD} 
  ${MyGUI_SOURCE}/Platforms/Ogre/OgrePlatform
  ${MyGUI_SOURCE}/MyGUIEngine)

set(MyGUI_INC_SEARCH_PATH ${MyGUI_PREFIX_PATH})
set(MyGUI_LIB_SEARCH_PATH ${MyGUI_PREFIX_PATH})
#create_search_paths(MyGUI)
# redo search if prefix path changed
clear_if_changed(
  MyGUI_LIBRARY_FWK
  MyGUI_LIBRARY_REL
  MyGUI_LIBRARY_DBG
  MyGUI_INCLUDE_DIR
  MyGUI_OGRPLATFORM_REL
  MyGUI_OGRPLATFORM_DBG
  MyGUI_OGRPLATFORM_FWK
)

if(APPLE)
	set(MyGUI_STATIC_LIBRARY ON CACHE BOOL "Use mygui static library")
	if(${MyGUI_STATIC_LIBRARY})
		set(MyGUI_LIBRARY_NAMES MyGUIEngineStatic)
	else()
		set(MyGUI_LIBRARY_NAMES MyGUIEngine)
	endif()
	set(MyGUI_LIBRARY_NAMES_DBG MyGUIEngine)
	set(MyGUI_OGRPLATFORMS_DBG MyGUI.OgrePlatform)
else()
	set(MyGUI_LIBRARY_NAMES MyGUIEngine)
	get_debug_names(MyGUI_LIBRARY_NAMES)
	get_debug_names(MyGUI_OGRPLATFORMS)
endif()
set(MyGUI_OGRPLATFORMS MyGUI.OgrePlatform)

use_pkgconfig(MyGUI_PKGC MyGUI)

findpkg_framework(MyGUI)

#message(STATUS "library search path:"${MyGUI_LIB_SEARCH_PATH})
#foreach(dir ${MyGUI_LIB_SEARCH_PATH})
#	message(STATUS ${dir})
#endforeach(dir)

find_path(MyGUI_INCLUDE_DIR NAMES MyGUI.h HINTS ${MyGUI_INC_SEARCH_PATH} ${MyGUI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES MyGUIEngine MyGUIEngine/include)
find_path(MyGUI_OGRPLATFORM_DIR NAMES MyGUI_OgrePlatform.h HINTS ${MyGUI_INC_SEARCH_PATH} ${MyGUI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES Platforms/Ogre/OgrePlatform/include)

find_library(MyGUI_LIBRARY_REL NAMES ${MyGUI_LIBRARY_NAMES} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" release lib/release lib/relwithdebinfo lib/minsizerel)
find_library(MyGUI_LIBRARY_DBG NAMES ${MyGUI_LIBRARY_NAMES_DBG} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES debug lib/Debug lib/debug)
find_library(MyGUI_OGRPLATFORM_REL NAMES ${MyGUI_OGRPLATFORMS} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES release lib/release lib/Releaserelwithdebinfo lib/minsizerel)
find_library(MyGUI_OGRPLATFORM_DBG NAMES ${MyGUI_OGRPLATFORMS_DBG} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES debug lib/Debug lib/debug)
make_library_set(MyGUI_LIBRARY)
make_library_set(MyGUI_OGRPLATFORM)

findpkg_finish(MyGUI)
add_parent_dir(MyGUI_INCLUDE_DIRS MyGUI_INCLUDE_DIR)
set(MyGUI_LIBRARIES ${MyGUI_LIBRARIES} ${MyGUI_OGRPLATFORM})
set(MyGUI_INCLUDE_DIR ${MyGUI_INCLUDE_DIR} ${MyGUI_OGRPLATFORM_DIR})
