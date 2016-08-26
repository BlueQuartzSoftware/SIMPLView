# ============================================================================
# Copyright (c) 2009-2015 BlueQuartz Software, LLC
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice, this
# list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
# contributors may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The code contained herein was partially funded by the followig contracts:
#    United States Air Force Prime Contract FA8650-07-D-5800
#    United States Air Force Prime Contract FA8650-10-D-5210
#    United States Prime Contract Navy N00173-07-C-2068
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


if(NOT "${DREAM3D_SDK}" STREQUAL "")
  include("${DREAM3D_SDK}/DREAM3D_SDK.cmake")
  message(STATUS "DREAM3D_SDK Location: ${DREAM3D_SDK}")
endif()

if(${CMAKE_MAJOR_VERSION} STREQUAL "3")
  # CMake 3.0 introduces VERSION variables for each Project.
  cmake_policy(SET CMP0048 NEW)
  # CMake 3.0 introduces some OS X RPath policies
  cmake_policy(SET CMP0042 OLD)
  # CMake 3.1 introduces if() policies on dereferencing variables in quotes
  cmake_policy(SET CMP0054 NEW)
endif()

cmake_minimum_required(VERSION 3.1.0)


#------------------------------------------------------------------------------
# Create the Top level Project
project(SIMPLViewProj VERSION 1.0.0.0)

# ---------- Setup output Directories -------------------------
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${SIMPLViewProj_BINARY_DIR}/Bin
    CACHE PATH
    "Single Directory for all Libraries"
    )
endif()

# --------- Setup the Executable output Directory -------------
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${SIMPLViewProj_BINARY_DIR}/Bin
    CACHE PATH
    "Single Directory for all Executables."
    )
endif()

# --------- Setup the Executable output Directory -------------
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${SIMPLViewProj_BINARY_DIR}/Bin
    CACHE PATH
    "Single Directory for all static libraries."
    )
endif()



#------------------------------------------------------------------------------
# Find the Git Package for Versioning. It should be ok if Git is NOT found
Find_package(Git)

cmpRevisionString( GENERATED_HEADER_FILE_PATH "SIMPLViewFileNotUsed.h"
                        GENERATED_SOURCE_FILE_PATH "SIMPLViewFileNotUsed.cpp"
                        NAMESPACE "SIMPLViewProj"
                        PROJECT_NAME "${PROJECT_NAME}"
                        EXPORT_MACRO "SIMPLViewProj_EXPORT")

get_filename_component(SIMPLView_PARENT ${SIMPLViewProj_SOURCE_DIR} DIRECTORY CACHE)

#------------------------------------------------------------------------------
# If the developer has set another CMP directory then use that, otherwise look
# for the CMP directory at the same level as the SIMPL directory
if("${CMP_SOURCE_DIR}" STREQUAL "")
  set(CMP_SOURCE_DIR ${SIMPLView_PARENT}/CMP)
  message(STATUS "CMP_SOURCE_DIR: ${CMP_SOURCE_DIR}")
endif()

if(NOT EXISTS "${CMP_SOURCE_DIR}")
  if(Git_FOUND)
    message(STATUS "CMP_SOURCE_DIR does not exist at ${CMP_SOURCE_DIR}. Attempting to clone it from Github....")
    execute_process(COMMAND ${GIT_EXECUTABLE} clone -b develop git://github.com/bluequartzsoftware/CMP.git ${SIMPLView_PARENT}/CMP
                    OUTPUT_VARIABLE CLONE_OUTPUT
                    RESULT_VARIABLE did_run
                    ERROR_VARIABLE git_error
                    WORKING_DIRECTORY ${SIMPLView_PARENT} )
  endif()
  # One last check. If it does not exist at this point then we simply Abort CMake
  if(NOT EXISTS "${CMP_SOURCE_DIR}")
    message(FATAL_ERROR "CMP_SOURCE_DIR does not exist. We attempted to clone it from the GitHub repo but that also seemed\
      to fail. SIMPLView can not proceed without the CMP directory. Please ask a SIMPL developer for more\
      information about this error. Can you access http://github.com/bluequartzsoftware/CMP.git? is\
      a firewall blocking the 'git' protocol? ")
  endif()
endif()

#------------------------------------------------------------------------------
# If the developer has set another SIMPL directory then use that, otherwise look
# for the SIMPLView directory at the same level as the SIMPL directory
if("${SIMPLProj_SOURCE_DIR}" STREQUAL "")
  set(SIMPLProj_SOURCE_DIR ${SIMPLView_PARENT}/SIMPL)
  message(STATUS "SIMPLProj_SOURCE_DIR: ${SIMPLProj_SOURCE_DIR}")
endif()

if(NOT EXISTS "${SIMPLProj_SOURCE_DIR}")
  if(Git_FOUND)
    message(STATUS "SIMPLProj_SOURCE_DIR does not exist at ${SIMPLProj_SOURCE_DIR}. Attempting to clone it from Github....")
    execute_process(COMMAND ${GIT_EXECUTABLE} clone -b develop git://github.com/bluequartzsoftware/SIMPL.git ${SIMPLView_PARENT}/SIMPL
                    OUTPUT_VARIABLE CLONE_OUTPUT
                    RESULT_VARIABLE did_run
                    ERROR_VARIABLE git_error
                    WORKING_DIRECTORY ${SIMPLView_PARENT}/ )
    # message(STATUS "CLONE_OUTPUT: ${CLONE_OUTPUT}")
    # message(STATUS "did_run: ${did_run}")
    # message(STATUS "git_error: ${git_error}")
  endif()
  # One last check. If it does not exist at this point then we simply Abort CMake
  if(NOT EXISTS "${SIMPLProj_SOURCE_DIR}")
    message(FATAL_ERROR "SIMPLProj_SOURCE_DIR does not exist. We attempted to clone it from the GitHub repo but that also seemed\
      to fail. SIMPLView can not proceed without the SIMPLProj_SOURCE_DIR directory. Please ask a SIMPL developer for more\
      information about this error. Can you access http://github.com/bluequartzsoftware/SIMPLView.git? is\
      a firewall blocking the 'git' protocol? ")
  endif()
endif()


#------------------------------------------------------------------------------
# There are several different Release Types that we are going to use.
# Official: This is an official release from BlueQuartz Software
# Beta: This is a Beta release that has is getting ready to be officially released.
# Development: This is currently in development and probably has bugs.
get_property(SIMPLViewProj_RELEASE_TYPE GLOBAL PROPERTY PACKAGE_RELEASE_TYPE)
if("${SIMPLViewProj_RELEASE_TYPE}" STREQUAL "")
  set(SIMPLViewProj_RELEASE_TYPE "Development")
  set_property(GLOBAL PROPERTY PACKAGE_RELEASE_TYPE "${SIMPLViewProj_RELEASE_TYPE}")
endif()

include(CMakeParseArguments)

# -----------------------------------------------------------------------
# Setup a Global property that is used to gather Documentation Information
# into a single known location
get_property(DREAM3DDocRoot GLOBAL PROPERTY DREAM3DDocRoot)
if("${DREAM3DDocRoot}" STREQUAL "")
  message(STATUS "Setting GLOBAL PROPERTY DREAM3DDocRoot")
  set_property(GLOBAL PROPERTY DREAM3DDocRoot "${SIMPLViewProj_BINARY_DIR}")
  get_property(DREAM3DDocRoot GLOBAL PROPERTY DREAM3DDocRoot)
  file(WRITE "${SIMPLViewProj_BINARY_DIR}/DREAM3DDocGroupList" "")
endif()

get_property(SIMPLPluginList GLOBAL PROPERTY SIMPLPluginList)
if("${SIMPLPluginList}" STREQUAL "")
  message(STATUS "Setting GLOBAL PROPERTY SIMPLPluginList")
  set_property(GLOBAL PROPERTY SIMPLPluginList "${SIMPLViewProj_BINARY_DIR}/SIMPLPluginList.txt")
  get_property(SIMPLPluginList GLOBAL PROPERTY SIMPLPluginList)
  file(WRITE "${SIMPLViewProj_BINARY_DIR}/SIMPLPluginList.txt" "")
endif()

get_property(SIMPLLibSearchDirs GLOBAL PROPERTY SIMPLLibSearchDirs)
if("${SIMPLLibSearchDirs}" STREQUAL "")
  message(STATUS "Setting GLOBAL PROPERTY SIMPLLibSearchDirs")
  set_property(GLOBAL PROPERTY SIMPLLibSearchDirs "${SIMPLViewProj_BINARY_DIR}/LibSearchDirs.txt")
  get_property(SIMPLLibSearchDirs GLOBAL PROPERTY SIMPLLibSearchDirs)
  file(WRITE "${SIMPLViewProj_BINARY_DIR}/LibSearchDirs.txt" "")
endif()

# -----------------------------------------------------------------------
# The project generally does NOT want to install headers. See if another
# project has over ridden that property
get_property(CMP_PROJECT_INSTALL_HEADERS GLOBAL PROPERTY CMP_PROJECT_INSTALL_HEADERS)
if("${CMP_PROJECT_INSTALL_HEADERS}" STREQUAL "")
  set_property(GLOBAL PROPERTY CMP_PROJECT_INSTALL_HEADERS "0")
  set(PROJECT_INSTALL_HEADERS "0")
else()
  set(PROJECT_INSTALL_HEADERS "${CMP_PROJECT_INSTALL_HEADERS}")
endif()

# -----------------------------------------------------------------------
# Set this to ON because SIMPLView needs the Qt based widgets to be compiled.
set(SIMPL_BUILD_WIDGETS ON CACHE BOOL "Build the QtWidgets based library")

# -----------------------------------------------------------------------
# Bring in the SIMPL Project
# -----------------------------------------------------------------------
add_subdirectory( ${SIMPLProj_SOURCE_DIR} ${PROJECT_BINARY_DIR}/SIMPL)


# -----------------------------------------------------------------------
#Enable use of Solution Folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PROJECT_CODE_DIR ${PROJECT_SOURCE_DIR}/Source)
set(PROJECT_RESOURCES_DIR ${SIMPLViewProj_SOURCE_DIR}/Resources)
set(EXE_DEBUG_EXTENSION "_debug")
set(PROJECT_INSTALL_HEADERS "${CMP_PROJECT_INSTALL_HEADERS}")
set(PROJECT_INSTALL_EXECUTABLES "1")


set(CMP_CONFIGURED_FILES_SOURCE_DIR ${CMP_SOURCE_DIR}/ConfiguredFiles)
INCLUDE (${CMP_SOURCE_DIR}/cmpCMakeMacros.cmake )
set(CMP_ENABLE_PLUGINS "1")
set(CMP_PLUGIN_LIST_FILE "${SIMPLPluginList}")
set(CMP_PLUGIN_SEARCHDIR_FILE "${SIMPLLibSearchDirs}")
file(APPEND ${CMP_PLUGIN_SEARCHDIR_FILE} "${SIMPLViewProj_BINARY_DIR}/Bin/plugins;")
file(APPEND ${CMP_PLUGIN_SEARCHDIR_FILE} "${SIMPLViewProj_BINARY_DIR}/Bin;")
set(CMP_LIB_SEARCH_DIRS "")
set(CMP_MODULES_SOURCE_DIR ${CMP_SOURCE_DIR}/Modules CACHE INTERNAL "")

if(UNIX AND NOT APPLE)
    set(DREAM3D_LINUX_SYSTEM ${CMAKE_SYSTEM_NAME} CACHE STRING "The Name of the linux distrubution for packaging")
endif()

# --------------------------------------------------------------------
# Override CMake's built in module directory by prepending cmp's module
# directory first
set(CMAKE_MODULE_PATH ${CMP_MODULES_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# --------------------------------------------------------------------
# Find HDF5 Headers/Libraries
# HDF5 now comes with everything that is needed for CMake to load
# up the targets (Exported) that it needs. We just need to find where
# HDF5 is installed.
include(${CMP_SOURCE_DIR}/ExtLib/HDF5Support.cmake)

# --------------------------------------------------------------------
# Find Boost Headers/Libraries
#set(Boost_FIND_REQUIRED FALSE)
set(Boost_DEBUG FALSE)
set(Boost_USE_MULTITHREADED TRUE)
set(Boost_USE_STATIC_LIBS TRUE)
set(Boost_ADDITIONAL_VERSIONS 1.57 1.58 1.59 1.60)
set(Boost_FIND_COMPONENTS "")
set(Boost_NO_BOOST_CMAKE 1)
FIND_PACKAGE(Boost)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

# --------------------------------------------------------------------
# Should we use Intel Threading Building Blocks
# --------------------------------------------------------------------
set(SIMPLib_USE_PARALLEL_ALGORITHMS "")
option(SIMPL_USE_MULTITHREADED_ALGOS "Use MultiThreaded Algorithms" ON)
if(SIMPL_USE_MULTITHREADED_ALGOS)
    find_package(TBB)
    if(TBB_FOUND)
        include_directories(${TBB_INCLUDE_DIRS} )
        CMP_COPY_DEPENDENT_LIBRARIES("tbb;tbb_malloc")
        # Append the locations of the TBB libraries to our Library Search Paths
        list(APPEND CMP_LIB_SEARCH_DIRS ${TBB_BINARY_DIR} ${TBB_LIBRARY_DIR} )
        # Create an install rule to copy the dependent shared library over to the installed location
        CMP_LIBRARIES_INSTALL_RULES("tbb;tbb_malloc" ".")
    else()
        message(FATAL_ERROR "The Intel Threading Building Blocks library is needed to enable the multithreaded algorithms. Please make sure it is installed properly")
    endif()
    set(SIMPLib_USE_PARALLEL_ALGORITHMS "1")
endif()

# --------------------------------------------------------------------
# SIMPL needs the Eigen library for Least Squares fit and Eigen
# value/vector calculations.
set(SIMPLib_USE_EIGEN "")
Find_Package(Eigen REQUIRED)
if(EIGEN_FOUND)
  message(STATUS "Found Eigen Library")
  include_directories( ${EIGEN_INCLUDE_DIRS} )
  set(SIMPLib_USE_EIGEN "1")
else()
  message(WARNING "The Eigen Library is required for some algorithms to execute. These algorithms will be disabled.")
endif()


# --------------------------------------------------------------------
# Find the Doxygen package
FIND_PACKAGE(Doxygen)
if(DOXYGEN_FOUND)
  message(STATUS "Doxygen Found at ${DOXYGEN_EXECUTABLE}")
else()
  message(FATAL_ERROR "Doxygen is needed to build the documentation. Please install it correctly")
endif()

# --------------------------------------------------------------------
# Find and Use the Qt5 Libraries
include(${CMP_SOURCE_DIR}/ExtLib/Qt5Support.cmake)
set(SIMPLView_Qt5_Components Core Widgets Network Gui Concurrent Script Svg Xml OpenGL PrintSupport )
option(SIMPLView_USE_QtWebEngine "Use QWebEngine to display Help. OFF=Use default web browser" ON)
CMP_AddQt5Support( "${SIMPLView_Qt5_Components}"
                    "${SIMPLView_USE_QtWebEngine}"
                    "${SIMPLViewProj_BINARY_DIR}"
                    "SIMPLView")

# --------------------------------------------------------------------
# Now that we have found all of our required libraries and packages we can start the all
# the tests

# We do NOT want to install any of the headers
set(SIMPLViewProj_INSTALL_FILES "${PROJECT_INSTALL_HEADERS}")

# --------------------------------------------------------------------
# Generate our ReadMe and License Files
configure_file(${PROJECT_RESOURCES_DIR}/SIMPLView/SIMPLViewLicense.txt
                ${SIMPLViewProj_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt )
configure_file(${PROJECT_SOURCE_DIR}/ReadMe-SIMPLView.md
                ${SIMPLViewProj_BINARY_DIR}/ReadMe-SIMPLView.md )

# --------------------------------------------------------------------
# Generate install rules for the text files
install(FILES ${SIMPLViewProj_BINARY_DIR}/ReadMe-SIMPLView.md
              ${SIMPLViewProj_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt 
        DESTINATION .
        COMPONENT Applications)

# Include some directories for the compiler
include_directories(${SIMPLViewProj_BINARY_DIR})

# --------------------------------------------------------------------
# Add in some compiler definitions
# --------------------------------------------------------------------
if( CMAKE_BUILD_TYPE MATCHES Debug )
  ADD_DEFINITIONS(-DDEBUG)
ENDif( CMAKE_BUILD_TYPE MATCHES Debug )

if(NOT MSVC)
 # ADD_DEFINITIONS(-Wall)
endif()

# On linux we need to set this because some of the libraries are Static
# and some are shared.
if( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT MSVC )
  ADD_DEFINITIONS(-fPIC)
ENDif()

# --------------------------------------------------------------------
# If was are using GCC, make the compiler messages on a single line
if(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDif(CMAKE_COMPILER_IS_GNUCC)
if(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDif(CMAKE_COMPILER_IS_GNUCXX)

if(MSVC)
 add_definitions(-D_CRT_SECURE_NO_WARNINGS)
 add_definitions(-D_SCL_SECURE_NO_WARNINGS)
 option(SIMPL_DISABLE_MSVC_WARNINGS "Disable some MSVC Compiler warnings" OFF)
endif()


include_directories(${SIMPLViewProj_SOURCE_DIR}/Source)
include_directories(${SIMPLViewProj_BINARY_DIR})


#------------------------------------------------------------------------------
# If we have not defined a "BrandedSIMPLView_DIR" then define it to the default
# generic set of ICONs
if(NOT DEFINED BrandedSIMPLView_DIR)
  set(BrandedSIMPLView_DIR ${SIMPLViewProj_SOURCE_DIR}/Resources/OpenSourceEdition)
endif()

include_directories(${BrandedSIMPLView_DIR})

# -----------------------------------------------------------------------
# This needs to be set here as we are going to look for files in this directory
# -----------------------------------------------------------------------
if("${FilterWidgetsLib_BINARY_DIR}" STREQUAL "")
  set(FilterWidgetsLib_BINARY_DIR ${SIMPLViewProj_BINARY_DIR}/FilterWidgetsLib)
endif()

# --------------------------------------------------------------------
#- Add in the Main SIMPLView Application
add_subdirectory( ${SIMPLViewProj_SOURCE_DIR}/Source/Applications/SIMPLView ${PROJECT_BINARY_DIR}/Applications/SIMPLView)

# --------------------------------------------------------------------
#- Add in the Main SIMPLView Application
option(SIMPLView_BUILD_DevHelper "Build the DevHelper Application" OFF)
if(SIMPLView_BUILD_DevHelper)
  add_subdirectory( ${SIMPLViewProj_SOURCE_DIR}/Source/Applications/DevHelper  ${PROJECT_BINARY_DIR}/Applications/DevHelper)
endif()

set(SIMPView_DOCS_ROOT_DIR "${SIMPLViewProj_BINARY_DIR}/Bin/Help/SIMPLView")

# add the Command line utilities
# Figure out here if we are going to build the command line tools
option(SIMPLView_BUILD_CLI_TOOLS "Build the command line versions of the applications" OFF)
if( SIMPLView_BUILD_CLI_TOOLS )
    add_subdirectory(${SIMPLViewProj_SOURCE_DIR}/Tools ${SIMPLViewProj_BINARY_DIR}/Tools)
endif()

if(0)
  #-------------------------------------------------------------------------------
  # Should we build the documentation using DOxygen
  option(SIMPLViewProj_BUILD_DOCUMENTATION "Build the DREAM3D documentation" ON)
  if(SIMPLViewProj_BUILD_DOCUMENTATION)
    set(DOCUMENTATION_RESOURCES_DIR ${PROJECT_RESOURCES_DIR}/DocConfigFiles)

    set(DOX_GENERATE_RTF "NO")
    option(SIMPLViewProj_GENERATE_RTF "Generate RTF Documentation" OFF)
    if(SIMPLViewProj_GENERATE_RTF STREQUAL "ON")
      set(DOX_GENERATE_RTF "YES")
    endif()

    # By default we generate the Documentation in HTML format
    set(DOX_GENERATE_HTML "NO")
    option(SIMPLViewProj_GENERATE_HTML "Generate HTML Documentation" ON)
    if(SIMPLViewProj_GENERATE_HTML STREQUAL "ON")
      set(DOX_GENERATE_HTML "YES")
    endif()

    #most windows machines do NOT have LATEX built in so default this to OFF
    set(DOX_GENERATE_LATEX "NO")
    option(SIMPLViewProj_GENERATE_LATEX "Generate PDF Documentation" OFF)
    if(SIMPLViewProj_GENERATE_LATEX STREQUAL "ON")
      set(DOX_GENERATE_LATEX "YES")
      # Check for existence of LaTex executable files
      INCLUDE( ${DOCUMENTATION_RESOURCES_DIR}/CMake/UseLATEX.cmake)
      if(APPLE)
        option(AUTO_OPEN_PDF "Automatically open generated PDF files with default PDF reading application" ON)
      endif()
    endif()


    # Now add each subdirectory that holds our documentation files
    add_subdirectory(${PROJECT_SOURCE_DIR}/Documentation/ReferenceManual ${SIMPLViewProj_BINARY_DIR}/Documentation/ReferenceManual)
    add_subdirectory(${PROJECT_SOURCE_DIR}/Documentation/APIDocs ${SIMPLViewProj_BINARY_DIR}/Documentation/APIDocs)
  endif()

endif()


# This should be the last line in this file:
include(${PROJECT_RESOURCES_DIR}/CPack/PackageProject.cmake)

# ------- Enable the CTest testing. Use make test to run all tests ---------------
if(SIMPL_BUILD_TESTING)
    include(${SIMPLViewProj_SOURCE_DIR}/Test/CMakeLists.txt)
endif()
