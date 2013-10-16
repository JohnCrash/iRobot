cmake_minimum_required(VERSION 2.8)

project(iRobot)

# Include necessary submodules
set(CMAKE_MODULE_PATH
	"${iRobot_SOURCE_DIR}/CMake"
	"${iRobot_SOURCE_DIR}/CMake/Utils"
	"${iRobot_SOURCE_DIR}/CMake/Packages"
	"${iRobot_SOURCE_DIR}/CMake/Templates"
)

set(HEADER_FILES 
	include/BaseManager.h
	include/Main.h
	include/PointerManager.h
	include/StatisticInfo.h
	include/InputFocusInfo.h
	include/Game.h
	include/Factory.h
	include/Input.h
	include/MainMenu.h
	include/SimpleUI.h
	include/Stage.h
	include/stdheader.h
	include/Log.h
	include/Rigid.h
	include/Geometry.h
	include/RigidManager.h
#	include/TestStage.h
	include/CameraControlet.h
	include/Physica.h
	include/RigidControlet.h
	include/RigidTool.h
	include/Math3d.h
	include/RigidToolManager.h
	include/Controlet.h
	include/InputFilter.h
	include/Joint.h
	include/VisualObject.h
	include/GeometryObject.h
	include/LuaBind.h
	include/LuaExport.h
	include/LuaManager.h
	include/utf8.h
	include/JointGroup.h
	include/LuaControlet.h
	include/HotkeyManager.h
	include/LuaHotkey.h
	include/Framework.h
	include/StringUtility.h
	include/ObjectFactory.h
	include/ClassInfo.h
	include/gettext.h
	include/Registry.h
	)

set(SOURCE_FILES 
	src/stdheader.cpp
	src/BaseManager.cpp
	src/Main.cpp
	src/PointerManager.cpp
	src/Factory.cpp
	src/Game.cpp
	src/MainMenu.cpp
	src/SimpleUI.cpp
	src/Console.cpp
	src/Geometry.cpp
	src/Rigid.cpp
	src/RigidManager.cpp
#	src/TestStage.cpp
	src/CameraControlet.cpp
	src/Physica.cpp
	src/RigidControlet.cpp
	src/Controlet.cpp
	src/RigidTool.cpp
	src/Math3d.cpp
	src/RigidToolManager.cpp
	src/InputFilter.cpp
	src/Joint.cpp
	src/VisualObject.cpp
	src/GeometryObject.cpp
	src/LuaGame.cpp
	src/LuaBind.cpp
	src/LuaGui.cpp
	src/LuaManager.cpp
	src/LuaRigid.cpp
	src/utf8.cpp
	src/LuaCamera.cpp
	src/JointGroup.cpp
	src/LuaControlet.cpp
	src/LuaMat.cpp
	src/HotkeyManager.cpp
	src/LuaHotkey.cpp
	src/Framework.cpp
	src/StringUtility.cpp
	src/ObjectFactory.cpp
	src/gettext.cpp
	src/LuaXml.cpp
	src/Registry.cpp
	src/LuaRegistry.cpp
	)

SOURCE_GROUP("Header Files\\LuaExport" FILES
	include/LuaBind.h
	include/LuaExport.h
	include/LuaManager.h
	include/LuaControlet.h
	include/LuaHotkey.h
)

SOURCE_GROUP("Source Files\\LuaExport" FILES
	src/LuaGame.cpp
	src/LuaBind.cpp
	src/LuaGui.cpp
	src/LuaManager.cpp
	src/LuaRigid.cpp
	src/LuaCamera.cpp
	src/LuaControlet.cpp
	src/LuaMat.cpp
	src/LuaHotkey.cpp
	src/LuaRegistry.cpp
	src/LuaXml.cpp	
)
SOURCE_GROUP("Header Files\\Singleton" FILES
	include/PointerManager.h
	include/Game.h
	include/Geometry.h
	include/RigidManager.h
	include/RigidToolManager.h
	include/InputFilter.h
	include/HotkeyManager.h
	include/ObjectFactory.h
	include/gettext.h
	include/Registry.h
)

SOURCE_GROUP("Source Files\\Singleton" FILES
	src/PointerManager.cpp
	src/Game.cpp
	src/RigidManager.cpp
	src/RigidToolManager.cpp
	src/InputFilter.cpp
	src/HotkeyManager.cpp
	src/ObjectFactory.cpp
	src/gettext.cpp
	src/Registry.cpp
)

# Specify build paths
set(iRobot_WORK_DIR ${iRobot_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${iRobot_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${iRobot_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${iRobot_BINARY_DIR}/bin)

set(Boost_USE_MULTITHREADED ON)

set(iRobot_DEPENDENCIES_DIR "" CACHE PATH "iRobot dependencies")
#告诉FindBoost模块boost位置
set(BOOST_ROOT ${iRobot_DEPENDENCIES_DIR}/boost)

#find_package(Freetype)	
find_package(OIS)
find_package(OGRE)	
find_package(MyGUI)
find_package(Boost)
find_package(ODE)
find_package(Lua)


include_directories(
${CMAKE_CURRENT_SOURCE_DIR}/include
${Boost_INCLUDE_DIR}
${OGRE_INCLUDE_DIR}
${OIS_INCLUDE_DIR}
${MyGUI_INCLUDE_DIR}
${ODE_INCLUDE_DIR}
${Lua_INCLUDE_DIR}
)

if(WIN32)
	set(SOURCE_FILES ${SOURCE_FILES} src/win32/InputManager.cpp)
	set(HEADER_FILES ${HEADER_FILES} include/win32/InputManager.h)

	if(MSVC)
	#VC++打开预编译头，可以显著加快编译时间
		add_definitions(/Yu"stdheader.h")
		add_definitions(/D_CRT_SECURE_NO_WARNINGS)
		add_definitions(/D_SCL_SECURE_NO_WARNINGS)
	#	add_definitions(/DdDOUBLE)
		set_source_files_properties(src/stdheader.cpp PPROPERTIES COMPILE_FLAGS "/Ycstdheader.h")
	endif()
	add_executable(iRobot WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
	target_link_libraries(iRobot ${BOOST_LIBRARIES} ${OGRE_LIBRARIES} ${OIS_LIBRARIES} 
		${MyGUI_LIBRARIES} ${ODE_LIBRARIES} ${Lua_LIBRARIES} Ws2_32)
elseif(APPLE)
	find_package(IOKit)
	find_package(Carbon)
	find_package(Cocoa)

	set(SOURCE_FILES ${SOURCE_FILES} src/mac/InputManager.mm)
	set(HEADER_FILES ${HEADER_FILES} include/mac/InputManager.h)

	#add_definitions(/DdDOUBLE)
	#这个苹果Ogre使用了boost，注意其他的未必有
	
	set_source_files_properties(src/Main.cpp PROPERTIES COMPILE_FLAGS "-x objective-c++")

	add_executable(iRobot MACOSX_BUNDLE ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})
	target_link_libraries(iRobot ${BOOST_LIBRARIES} 
		${OGRE_LIBRARIES} ${OIS_LIBRARIES} ${MyGUI_LIBRARIES} ${Cocoa_LIBRARIES} 
		${Carbon_LIBRARIES} ${IOKit_LIBRARIES} ${ODE_LIBRARIES} ${Lua_LIBRARIES})

	#xcode预编译头文件
	if(CMAKE_GENERATOR MATCHES Xcode)
		SET_TARGET_PROPERTIES(iRobot PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "src/stdheader.cpp")
            	SET_TARGET_PROPERTIES(iRobot PROPERTIES XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
	endif()
endif(WIN32)


# Install CMake modules
add_subdirectory(CMake)

# install resource files
include(InstallDLL)

set(iRobot_TEMPLATES_DIR "${iRobot_SOURCE_DIR}/CMake/Templates")
if(WIN32)
	set(iRobot_PLUGIN_DIR ".")
	set(iRobot_MEDIA_DIR "${iRobot_SOURCE_DIR}/Media")
elseif(APPLE)
	#set(iRobot_PLUGIN_DIR ".")
	set(iRobot_MEDIA_DIR "${iRobot_SOURCE_DIR}/Media")
endif()
set(iRobot_NO_PLUGIN_OGRE_DIRECTX "#")

if (WIN32)
	# create resources.xml
	configure_file(${iRobot_TEMPLATES_DIR}/resources.xml.in ${iRobot_BINARY_DIR}/bin/debug/resources.xml)
	configure_file(${iRobot_TEMPLATES_DIR}/resources.xml.in ${iRobot_BINARY_DIR}/bin/release/resources.xml)
	# create plugins.cfg
	configure_file(${iRobot_TEMPLATES_DIR}/plugins_d.cfg.in ${iRobot_BINARY_DIR}/bin/debug/plugins.cfg)
	configure_file(${iRobot_TEMPLATES_DIR}/plugins.cfg.in ${iRobot_BINARY_DIR}/bin/release/plugins.cfg)
	
	#查找Ogre动态库位置
	set(DLLS_SEARCH_PATH
	${OGRE_INCLUDE_DIR}
	${OGRE_LIB_DIR}/../bin
	${OGRE_LIB_DIR}
	${OGRE_BUILD}/bin
	${ODE_SOURCE}/lib
	)
	#将Ogre的动态库安装到iRobot的执行目录
	install_dll_file(OgreMain "${DLLS_SEARCH_PATH}")
	install_dll_file(RenderSystem_GL "${DLLS_SEARCH_PATH}")
	install_dll_file(RenderSystem_Direct3D9 "${DLLS_SEARCH_PATH}")
	#查找OIS的动态库位置
	set(DLLS_SEARCH_PATH
	${OIS_INCLUDE_DIR}/../../bin
	${OGRE_LIB_DIR}/../bin
	${OGRE_BUILD}/bin)
	#将OIS动态库安装到iRobot的执行目录
	install_dll_file(OIS "${DLLS_SEARCH_PATH}")
	#将MyGUI的动态库复制到iRobot执行目录
	set(DLLS_SEARCH_PATH
	${MyGUI_BUILD}/bin
	${MyGUI_SOURCE}/bin
	${OGRE_BUILD}/bin)
	install_dll_file(MyGUIEngine "${DLLS_SEARCH_PATH}")
	
	#安装ODE动态库
	#	我使用手动copy
##	set(DLLS_SEARCH_PATH
##	${ODE_SOURCE}/trunk/lib/DebugDoubleDLL
##	${ODE_SOURCE}/trunk/lib/ReleaseDoubleDLL)
##	install_dll_file(ode_double "${DLLS_SEARCH_PATH}")	
	
	create_vcproj_userfile( iRobot )
elseif(APPLE) 
# other OS only need one cfg file
	string(TOLOWER "${CMAKE_BUILD_TYPE}" MYGUI_BUILD_TYPE)
	if (MYGUI_BUILD_TYPE STREQUAL "debug" AND NOT APPLE)
		set(MYGUI_CFG_SUFFIX "_d")
	endif ()
	set(MACOSX_BUNDLE_ICON_FILE robot.icns)
	set_property(TARGET iRobot PROPERTY MACOSX_BUNDLE TRUE)
	set_property(TARGET iRobot PROPERTY MACOSX_BUNDLE_INFO_PLIST ${iRobot_SOURCE_DIR}/misc/Info.plist)
	#苹果应用目录
	set(APP_BUNDLE_DEBUG ${iRobot_BINARY_DIR}/bin/debug/iRobot.app/Contents)
	set(APP_BUNDLE_RELEASE ${iRobot_BINARY_DIR}/bin/release/iRobot.app/Contents)
	# create resources.xml
	add_custom_command(TARGET iRobot POST_BUILD COMMAND mkdir ARGS -p ${APP_BUNDLE_DEBUG}/Resources )
	add_custom_command(TARGET iRobot POST_BUILD COMMAND mkdir ARGS -p ${APP_BUNDLE_RELEASE}/Resources )
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND ln ARGS -s -f ${PROJECT_SOURCE_DIR}/Media/robot.icns ${APP_BUNDLE_DEBUG}/Resources
		)
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND ln ARGS -s -f ${PROJECT_SOURCE_DIR}/Media/robot.icns ${APP_BUNDLE_RELEASE}/Resources
		)		
	configure_file(${iRobot_TEMPLATES_DIR}/resources.xml.in ${APP_BUNDLE_DEBUG}/Resources/resources.xml)
	configure_file(${iRobot_TEMPLATES_DIR}/resources.xml.in ${APP_BUNDLE_RELEASE}/Resources/resources.xml)
	configure_file(${iRobot_TEMPLATES_DIR}/plugins.cfg.in ${APP_BUNDLE_DEBUG}/Resources/plugins.cfg)
	configure_file(${iRobot_TEMPLATES_DIR}/plugins.cfg.in ${APP_BUNDLE_RELEASE}/Resources/plugins.cfg)
#	configure_file(${iRobot_TEMPLATES_DIR}/builder.xml.in ${APP_BUNDLE_DEBUG}/Resources/builder.xml)
#	configure_file(${iRobot_TEMPLATES_DIR}/builder.xml.in ${APP_BUNDLE_RELEASE}/Resources/builder.xml)

	#简单的建立个连接Ogre.framework
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND mkdir ARGS -p ${APP_BUNDLE_DEBUG}/Frameworks
		COMMAND ln ARGS -s -f ${OGRE_BUILD}/lib/debug/Ogre.framework ${APP_BUNDLE_DEBUG}/Frameworks
		)
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND mkdir ARGS -p ${APP_BUNDLE_RELEASE}/Frameworks
		COMMAND ln ARGS -s -f ${OGRE_BUILD}/lib/release/Ogre.framework ${APP_BUNDLE_RELEASE}/Frameworks
		)		
	#和RenderSystem_GL建立连接		
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND mkdir ARGS -p ${APP_BUNDLE_DEBUG}/Plugins
		COMMAND ln ARGS -s -f ${OGRE_BUILD}/lib/debug/RenderSystem_GL.dylib ${APP_BUNDLE_DEBUG}/Plugins
	)	
	add_custom_command(TARGET iRobot POST_BUILD
		COMMAND mkdir ARGS -p ${APP_BUNDLE_RELEASE}/Plugins
		COMMAND ln ARGS -s -f ${OGRE_BUILD}/lib/release/RenderSystem_GL.dylib ${APP_BUNDLE_RELEASE}/Plugins
	)
endif ()
