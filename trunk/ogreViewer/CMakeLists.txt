# Configure ogreViewer which is based on Lighting-demo from the OGRE SDK

set( OGREVIEW_NAME ogreViewer )

FILE(GLOB_RECURSE ogreViewer_INCS 
	"inc/*.h"
)

FILE(GLOB_RECURSE ogreViewer_SRCS 
	"src/*.cpp"
)
SET(HEADER_FILES ${ogreViewer_INCS})
SET(SOURCE_FILES ${ogreViewer_SRCS})

###############################################################################
# source groupes, mainly used for VS-Folders and to structure coding
###############################################################################
# Headers and Source Files for ogreViewer
SOURCE_GROUP( "inc" FILES ${ogreViewer_INCS} )
SOURCE_GROUP( "src" FILES ${ogreViewer_SRCS} )


INCLUDE_DIRECTORIES(
	${mixedRealityGlasses_SOURCE_DIR}/ogreViewer/inc
	${OGRE_INCLUDE_DIRS}
	${OIS_INCLUDE_DIR}
	#${OGRE_RenderSystem_Direct3D9_INCLUDE_DIRS}
	#${OGRE_RenderSystem_Direct3D10_INCLUDE_DIRS}	
	${KinectSDK_INCLUDE_DIR}
	${KinectToolkit_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
)

LINK_DIRECTORIES( 
	${OGRE_LIBRARY_DIR}
	${KINECT_LIBRARY_DIR}
	${BOOST_LIBRARY_DIR}
	${OIS_LIBRARY_DIR}
	${KinectSDK_LIBRARY_DIR}
	${KinectToolkit_LIBRARY_DIR}
)

ADD_EXECUTABLE( ${OGREVIEW_NAME} 
	${HEADER_FILES}
	${SOURCE_FILES}
)

TARGET_LINK_LIBRARIES( ${OGREVIEW_NAME} 
	${OGRE_LIBRARIES}
	${KINECT_LIBRARIES}
	${OIS_LIBRARIES}
	${KinectSDK_LIBRARIES}
	${KinectToolkit_LIBRARIES}
)

SET( EXE_INSTALL_DIR ${mixedRealityGlasses_SOURCE_DIR}/bin )
INSTALL( TARGETS ${OGREVIEW_NAME} DESTINATION ${EXE_INSTALL_DIR} )
