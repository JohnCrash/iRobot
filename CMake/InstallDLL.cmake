macro(install_dll_file FILENAME DLLSPATHS)
	set(DEBUG_DLLS_DIR "")
	set(RELEASE_DLLS_DIR "")
	foreach(dir ${DLLSPATHS})
		if (EXISTS ${dir}/debug/${FILENAME}_d.dll)
			set(DEBUG_DLLS_DIR ${dir}/debug/)
			set(RELEASE_DLLS_DIR ${dir}/release/)
			break()
		endif()
	endforeach(dir)
	if (EXISTS ${DEBUG_DLLS_DIR}${FILENAME}_d.dll)		
		configure_file(${DEBUG_DLLS_DIR}${FILENAME}_d.dll ${iRobot_BINARY_DIR}/bin/debug/${FILENAME}_d.dll COPYONLY)
		if (EXISTS ${RELEASE_DLLS_DIR}${FILENAME}.dll)		
			configure_file(${RELEASE_DLLS_DIR}${FILENAME}.dll ${iRobot_BINARY_DIR}/bin/release/${FILENAME}.dll COPYONLY)
		endif()
		message(STATUS "	Copy ${DEBUG_DLLS_DIR}${FILENAME}_d.dll")
	else()
		message(STATUS "	install_dll_file Can't copy ${FILENAME}.dll")
	endif ()
endmacro(install_dll_file)

# create vcproj.user file for Visual Studio to set debug working directory
macro(create_vcproj_userfile TARGETNAME)
	if (MSVC)
		# for VisualStudioUserFile.vcproj.user.in
		if(CMAKE_CL_64)
			set(iRobot_WIN_BUILD_CONFIGURATION "x64")
		else()
			set(iRobot_WIN_BUILD_CONFIGURATION "Win32")
		endif()
		
		if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 10" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 10 Win64")
			configure_file(
				${iRobot_TEMPLATES_DIR}/VisualStudio2010UserFile.vcxproj.user.in
				${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
				@ONLY
			)
		else ()
			configure_file(
				${iRobot_TEMPLATES_DIR}/VisualStudioUserFile.vcproj.user.in
				${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
				@ONLY
			)
		endif ()
		
	endif ()
endmacro(create_vcproj_userfile)