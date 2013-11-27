#SET(KinectToolkit_ROOT_DIR "c:/Program Files/Microsoft SDKs/Kinect/v1.0/")
SET(KinectToolkit_ROOT_DIR $ENV{FTSDK_DIR})
#message(${KinectToolkit_ROOT_DIR})
SET(KinectToolkit_LIB_DIR "${KinectToolkit_ROOT_DIR}Lib/x86")
SET(KinectToolkit_LIBRARY_DIR ${KinectToolkit_LIB_DIR}) #alias
SET(KinectToolkit_INCLUDE_DIR "${KinectToolkit_ROOT_DIR}inc")
#SET(KinectToolkit_DLLS ) #???
SET(KinectToolkit_LIBRARIES 
	"${KinectToolkit_LIB_DIR}/FaceTrackLib.lib"
)
SET(KinectToolkit_INCLUDES 
	"${KinectToolkit_INCLUDE_DIR}/FaceTrackLib.h"
)

####################   Macro   #######################
MACRO(CHECK_FILES _FILES _DIR)
	SET(_MISSING_FILES)
	FOREACH(_FILE ${${_FILES}})
		IF(NOT EXISTS "${_FILE}")
			SET(KinectToolkit_FOUND NO)
			get_filename_component(_FILE ${_FILE} NAME)
			SET(_MISSING_FILES "${_MISSING_FILES}${_FILE}, ")
		ENDIF()
	ENDFOREACH()
	IF(_MISSING_FILES)
		MESSAGE(STATUS "In folder \"${${_DIR}}\" not found files: ${_MISSING_FILES}")
		SET(KinectToolkit_FOUND NO)
	ENDIF()
ENDMACRO(CHECK_FILES)

MACRO(CHECK_DIR _DIR)
	IF(NOT EXISTS "${${_DIR}}")
		MESSAGE(STATUS "Folder \"${${_DIR}}\" not found.")
		SET(KinectToolkit_FOUND NO)
	ENDIF()
ENDMACRO(CHECK_DIR)

##################### Checking #######################
MESSAGE(STATUS "Searching KinectToolkit.")
SET(KinectToolkit_FOUND YES)

CHECK_DIR(KinectToolkit_ROOT_DIR)
IF(KinectToolkit_FOUND)
	CHECK_DIR(KinectToolkit_LIB_DIR)
	CHECK_DIR(KinectToolkit_INCLUDE_DIR)
	
	IF(KinectToolkit_FOUND)
		#CHECK_FILES(KinectToolkit_DLLS KinectToolkit_ROOT_DIR)
		CHECK_FILES(KinectToolkit_LIBRARIES KinectToolkit_LIB_DIR)
		CHECK_FILES(KinectToolkit_INCLUDES KinectToolkit_INCLUDE_DIR)
	ENDIF()
ENDIF()

MESSAGE(STATUS "KinectToolkit_FOUND - ${KinectToolkit_FOUND}.")
