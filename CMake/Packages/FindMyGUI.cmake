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
	${iRobot_DEPENDENCIES_DIR}/mygui
	${iRobot_DEPENDENCIES_DIR}/build/mygui
	${iRobot_DEPENDENCIES_DIR}/mygui/MyGUIEngine
	${iRobot_DEPENDENCIES_DIR}/mygui/Platforms/Ogre/OgrePlatform
	${MyGUI_SOURCE} 
	${MyGUI_BUILD} 
	${MyGUI_SOURCE}/Platforms/Ogre/OgrePlatform
	${MyGUI_SOURCE}/MyGUIEngine)

create_search_paths(MyGUI)

#set(MyGUI_INC_SEARCH_PATH ${MyGUI_PREFIX_PATH})
#set(MyGUI_LIB_SEARCH_PATH ${MyGUI_PREFIX_PATH})
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

set(MyGUI_STATIC_LIBRARY OFF CACHE BOOL "Use mygui static library")
if(${MyGUI_STATIC_LIBRARY})
	set(MyGUI_LIBRARY_NAMES_REL MyGUIEngineStatic)
	set(MyGUI_LIBRARY_NAMES_DBG MyGUIEngineStatic)
else()
	set(MyGUI_LIBRARY_NAMES_REL MyGUIEngine)
	set(MyGUI_LIBRARY_NAMES_DBG MyGUIEngine)
endif()
set(MyGUI_OGRPLATFORMS_NAMES_REL MyGUI.OgrePlatform)
set(MyGUI_OGRPLATFORMS_NAMES_DBG MyGUI.OgrePlatform)

use_pkgconfig(MyGUI_PKGC MyGUI)

findpkg_framework(MyGUI)

#message(STATUS "library search path:"${MyGUI_LIB_SEARCH_PATH})
#foreach(dir ${MyGUI_LIB_SEARCH_PATH})
#	message(STATUS ${dir})
#endforeach(dir)
find_path(MyGUI_INCLUDE_DIR NAMES MyGUI.h HINTS ${MyGUI_INC_SEARCH_PATH} ${MyGUI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES MyGUIEngine MyGUIEngine/include)
find_path(MyGUI_OGRPLATFORM_DIR NAMES MyGUI_OgrePlatform.h HINTS ${MyGUI_INC_SEARCH_PATH} ${MyGUI_PKGC_INCLUDE_DIRS} PATH_SUFFIXES Ogre/OgrePlatform/include)
find_library(MyGUI_LIBRARY_REL NAMES ${MyGUI_LIBRARY_NAMES_REL} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Release lib/release lib/Release) 
find_library(MyGUI_LIBRARY_DBG NAMES ${MyGUI_LIBRARY_NAMES_DBG} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Debug lib/debug lib/Debug)
find_library(MyGUI_OGRPLATFORM_REL NAMES ${MyGUI_OGRPLATFORMS_NAMES_REL} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Release lib/release lib/Release)
find_library(MyGUI_OGRPLATFORM_DBG NAMES ${MyGUI_OGRPLATFORMS_NAMES_DBG} HINTS ${MyGUI_LIB_SEARCH_PATH} ${MyGUI_PKGC_LIBRARY_DIRS} PATH_SUFFIXES Debug lib/debug lib/Debug)

make_library_set(MyGUI_LIBRARY)
make_library_set(MyGUI_OGRPLATFORM)

findpkg_finish(MyGUI)
add_parent_dir(MyGUI_INCLUDE_DIRS MyGUI_INCLUDE_DIR)
set(MyGUI_LIBRARIES ${MyGUI_LIBRARIES} ${MyGUI_OGRPLATFORM})
set(MyGUI_INCLUDE_DIR ${MyGUI_INCLUDE_DIR} ${MyGUI_OGRPLATFORM_DIR})
