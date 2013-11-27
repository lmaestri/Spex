######################################################################
# BUILD file for Brillenprojekt (mixed reality glasses)
# This is the main file where we prepare the general build environment
# and provide build configuration options.
# adapted from the OGRE CMakefile
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_GENERATOR_CXX 1)
PROJECT(mixedRealityGlasses)
#SET(dofBrowser_SOURCE_DIR ${CMAKE_SOURCE_DIR})
# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
IF(WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  #set(CMAKE_USE_RELATIVE_PATHS true)
  #set(CMAKE_SUPPRESS_REGENERATION true)
ENDIF()

if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
  # differentiation between debug and release builds.
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

# we want all libraries in one folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${mixedRealityGlasses_SOURCE_DIR}/lib ) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${mixedRealityGlasses_SOURCE_DIR}/lib )
# we want all executables in one folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${mixedRealityGlasses_SOURCE_DIR}/bin )

######################################################################
# External dependencies 
######################################################################
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake )

FIND_PACKAGE( OGRE ) 
if( NOT OGRE_FOUND )
	MESSAGE( ERROR "Please install Ogre" )
endif( NOT OGRE_FOUND )

FIND_PACKAGE( OIS )
if( NOT OIS_FOUND )
	MESSAGE( ERROR "Please install OIS" )
endif( NOT OIS_FOUND )

FIND_PACKAGE( BOOST) 
if( NOT BOOST_FOUND )
	SET( BOOST_ROOT $ENV{OGRE_HOME}/boost_1_47 )
	SET( BOOST_LIBRARY_DIR ${BOOST_ROOT}/lib )
	SET( BOOST_INCLUDE_DIR ${BOOST_ROOT} )
endif( NOT BOOST_FOUND )

FIND_PACKAGE( KinectSDK )
FIND_PACKAGE( KinectToolkit )

######################################################################
# Set up directories
######################################################################

# Setup ogreViewer
ADD_SUBDIRECTORY(ogreViewer)
