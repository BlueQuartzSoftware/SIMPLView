

if(NOT "${DREAM3D_SDK}" STREQUAL "")
  include("${DREAM3D_SDK}/DREAM3D_SDK.cmake")
  get_property(DREAM3D_SDK_STATUS_PRINTED GLOBAL PROPERTY DREAM3D_SDK_STATUS_PRINTED)
  if(NOT DREAM3D_SDK_STATUS_PRINTED)
    message(STATUS "DREAM3D_SDK Location: ${DREAM3D_SDK}")
    set_property(GLOBAL PROPERTY DREAM3D_SDK_STATUS_PRINTED TRUE)
  endif()
endif()

# Request C++11 standard, using new CMake variables.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

if(${CMAKE_MAJOR_VERSION} STREQUAL "3")
  # CMake 3.1 introduces if() policies on dereferencing variables in quotes
  cmake_policy(SET CMP0054 NEW)
endif()

cmake_minimum_required(VERSION 3.8.0)
set(SIMPLViewProjGenerateVersion 0)

#------------------------------------------------------------------------------
# Create the Top level Project
project(SIMPLViewProj VERSION 1.1.0.0)
get_property(SIMPLViewProj_VERSION_MAJOR GLOBAL PROPERTY SIMPLViewProj_VERSION_MAJOR)
if("${SIMPLViewProj_VERSION_MAJOR}" STREQUAL "")
  set(SIMPLViewProj_VERSION_MAJOR "1")
  set(SIMPLViewProjGenerateVersion 1)
endif()
get_property(SIMPLViewProj_VERSION_MINOR GLOBAL PROPERTY SIMPLViewProj_VERSION_MINOR)
if("${SIMPLViewProj_VERSION_MINOR}" STREQUAL "")
  set(SIMPLViewProj_VERSION_MINOR "0")
endif()
get_property(SIMPLViewProj_VERSION_PATCH GLOBAL PROPERTY SIMPLViewProj_VERSION_PATCH)
if("${SIMPLViewProj_VERSION_PATCH}" STREQUAL "")
  set(SIMPLViewProj_VERSION_PATCH "0")
endif()
get_property(SIMPLViewProj_VERSION_TWEAK GLOBAL PROPERTY SIMPLViewProj_VERSION_TWEAK)
if("${SIMPLViewProj_VERSION_TWEAK}" STREQUAL "")
  set(SIMPLViewProj_VERSION_TWEAK "0")
endif()


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

get_filename_component(SIMPLView_PARENT ${SIMPLViewProj_SOURCE_DIR} DIRECTORY CACHE)

#------------------------------------------------------------------------------
# If the developer has set another CMP directory then use that, otherwise look
# for the CMP directory at the same level as the SIMPL directory
if("${CMP_SOURCE_DIR}" STREQUAL "")
  set(CMP_SOURCE_DIR ${SIMPLView_PARENT}/CMP)
  message(STATUS "CMP_SOURCE_DIR: ${CMP_SOURCE_DIR}")
endif()

if(NOT EXISTS "${CMP_SOURCE_DIR}")
  message(FATAL_ERROR "CMP_SOURCE_DIR does not exist at ${CMP_SOURCE_DIR}.")
endif()

include(${CMP_SOURCE_DIR}/cmpProject.cmake)
include(${CMP_SOURCE_DIR}/cmpCMakeMacros.cmake )

# If SIMPLView is being built from another higher level project then this is should be set to 0
if(NOT SIMPLViewProjGenerateVersion)
  cmpRevisionString( GENERATED_HEADER_FILE_PATH "SIMPLViewFileNotUsed.h"
                      GENERATED_SOURCE_FILE_PATH "SIMPLViewFileNotUsed.cpp"
                      NAMESPACE "SIMPLViewProj"
                      PROJECT_NAME "${PROJECT_NAME}"
                      EXPORT_MACRO "SIMPLViewProj_EXPORT")
endif()

get_filename_component(SIMPLView_PARENT ${SIMPLViewProj_SOURCE_DIR} DIRECTORY CACHE)

#------------------------------------------------------------------------------
# If the developer has set another SIMPLVtkLib directory then use that, otherwise look
# for the SIMPLVtkLib directory at the same level as the SIMPLView directory
if("${SIMPLVtkLibProj_SOURCE_DIR}" STREQUAL "")
  set(SIMPLVtkLibProj_SOURCE_DIR ${SIMPLView_PARENT}/SIMPLVtkLib)
  message(STATUS "SIMPLVtkLibProj_SOURCE_DIR: ${SIMPLVtkLibProj_SOURCE_DIR}")
endif()

if(NOT EXISTS "${SIMPLVtkLibProj_SOURCE_DIR}")
  message(FATAL_ERROR "SIMPLVtkLibProj_SOURCE_DIR does not exist at ${SIMPLVtkLibProj_SOURCE_DIR}.")
endif()

#------------------------------------------------------------------------------
# If the developer has set another SIMPL directory then use that, otherwise look
# for the SIMPLView directory at the same level as the SIMPL directory
if("${SIMPLProj_SOURCE_DIR}" STREQUAL "")
  set(SIMPLProj_SOURCE_DIR ${SIMPLView_PARENT}/SIMPL)
  message(STATUS "SIMPLProj_SOURCE_DIR: ${SIMPLProj_SOURCE_DIR}")
endif()

if(NOT EXISTS "${SIMPLProj_SOURCE_DIR}")
  message(FATAL_ERROR "SIMPLProj_SOURCE_DIR does not exist at ${SIMPLProj_SOURCE_DIR}.")
endif()

set(LIB_TYPE STATIC)
if(BUILD_SHARED_LIBS)
    set(LIB_TYPE SHARED)
endif(BUILD_SHARED_LIBS)



#------------------------------------------------------------------------------
# There are several different Release Types that we are going to use.
# Official: This is an official release from BlueQuartz Software
# Beta: This is a Beta release that has is getting ready to be officially released.
# Development: This is currently in development and probably has bugs.
get_property(SIMPLViewProj_RELEASE_TYPE GLOBAL PROPERTY PACKAGE_RELEASE_TYPE)
if("${SIMPLViewProj_RELEASE_TYPE}" STREQUAL "")
  set(SIMPLViewProj_RELEASE_TYPE "Beta")
  set_property(GLOBAL PROPERTY PACKAGE_RELEASE_TYPE "${SIMPLViewProj_RELEASE_TYPE}")
endif()

include(CMakeParseArguments)

option(SIMPLView_BUILD_DOCUMENTATION "Build the SIMPLView documentation" ON)
set_property(GLOBAL PROPERTY SIMPLView_BUILD_DOCUMENTATION "${SIMPLView_BUILD_DOCUMENTATION}")

option(SIMPLView_USE_STYLESHEETEDITOR "Use the style sheet editor to apply custom styles" OFF)
set_property(GLOBAL PROPERTY SIMPLView_USE_STYLESHEETEDITOR "${SIMPLView_USE_STYLESHEETEDITOR}")

# -----------------------------------------------------------------------
# Setup a Global property that is used to gather Documentation Information
# into a single known location
get_property(DREAM3DDocRoot GLOBAL PROPERTY DREAM3DDocRoot)
if("${DREAM3DDocRoot}" STREQUAL "")
  message(STATUS "Setting GLOBAL PROPERTY DREAM3DDocRoot")
  set_property(GLOBAL PROPERTY DREAM3DDocRoot "${SIMPLViewProj_BINARY_DIR}")
  get_property(DREAM3DDocRoot GLOBAL PROPERTY DREAM3DDocRoot)
endif()

get_property(SIMPLPluginList GLOBAL PROPERTY SIMPLPluginList)
if("${SIMPLPluginList}" STREQUAL "")
  # message(STATUS "Setting GLOBAL PROPERTY SIMPLPluginList")
  set_property(GLOBAL PROPERTY SIMPLPluginList "${SIMPLViewProj_BINARY_DIR}/SIMPLPluginList.txt")
  get_property(SIMPLPluginList GLOBAL PROPERTY SIMPLPluginList)
  file(WRITE "${SIMPLViewProj_BINARY_DIR}/SIMPLPluginList.txt" "")
endif()

get_property(SIMPLibSearchDirs GLOBAL PROPERTY SIMPLibSearchDirs)
if("${SIMPLibSearchDirs}" STREQUAL "")
  # message(STATUS "Setting GLOBAL PROPERTY SIMPLibSearchDirs")
  set_property(GLOBAL PROPERTY SIMPLibSearchDirs "${SIMPLViewProj_BINARY_DIR}/LibSearchDirs.txt")
  get_property(SIMPLibSearchDirs GLOBAL PROPERTY SIMPLibSearchDirs)
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
get_property(CMP_PROJECT_INSTALL_HEADERS GLOBAL PROPERTY CMP_PROJECT_INSTALL_HEADERS)

# -----------------------------------------------------------------------
# This needs to be set here because it is used in the next section....
set(EXE_DEBUG_EXTENSION "_debug")
if( "${SIMPLView_APPLICATION_NAME}" STREQUAL "")
  set(SIMPLView_APPLICATION_NAME "SIMPLView")
endif()

# -----------------------------------------------------------------------
# Set a Global Prefix where everthing is going to get intalled. For OS X we are
# going to try to install everything into the .app package
# -----------------------------------------------------------------------
set_property(GLOBAL PROPERTY DREAM3D_PACKAGE_DEST_PREFIX ".")
if(APPLE)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_property(GLOBAL PROPERTY DREAM3D_PACKAGE_DEST_PREFIX "${SIMPLView_APPLICATION_NAME}${EXE_DEBUG_EXTENSION}.app/Contents/")
  else()
    set_property(GLOBAL PROPERTY DREAM3D_PACKAGE_DEST_PREFIX "${SIMPLView_APPLICATION_NAME}.app/Contents/")
  endif()
endif()
get_property(DREAM3D_PACKAGE_DEST_PREFIX GLOBAL PROPERTY DREAM3D_PACKAGE_DEST_PREFIX)

# -----------------------------------------------------------------------
# Set this to ON because SIMPLView needs the Qt based widgets to be compiled.
set(SIMPL_Group_WIDGETS ON CACHE BOOL "Build the QtWidgets based library")

# -----------------------------------------------------------------------
# Bring in the SIMPL Project
# -----------------------------------------------------------------------
add_subdirectory( ${SIMPLProj_SOURCE_DIR} ${PROJECT_BINARY_DIR}/SIMPL)

# -----------------------------------------------------------------------
# Bring in the SIMPLVtkLib Project
# -----------------------------------------------------------------------
add_subdirectory( ${SIMPLVtkLibProj_SOURCE_DIR} ${PROJECT_BINARY_DIR}/SIMPLVtkLib)

# -----------------------------------------------------------------------
#Enable use of Solution Folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(PROJECT_CODE_DIR ${PROJECT_SOURCE_DIR}/Source)
set(PROJECT_RESOURCES_DIR ${SIMPLViewProj_SOURCE_DIR}/Resources)
set(PROJECT_INSTALL_HEADERS "${CMP_PROJECT_INSTALL_HEADERS}")
set(PROJECT_INSTALL_EXECUTABLES "1")


set(CMP_CONFIGURED_FILES_SOURCE_DIR ${CMP_SOURCE_DIR}/ConfiguredFiles)
set(CMP_ENABLE_PLUGINS "1")
set(CMP_PLUGIN_LIST_FILE "${SIMPLPluginList}")
set(CMP_PLUGIN_SEARCHDIR_FILE "${SIMPLibSearchDirs}")
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
# Should we use Intel Threading Building Blocks
# --------------------------------------------------------------------
set(SIMPL_USE_PARALLEL_ALGORITHMS "")
option(SIMPL_USE_MULTITHREADED_ALGOS "Use MultiThreaded Algorithms" ON)
if(SIMPL_USE_MULTITHREADED_ALGOS)
    find_package(TBB)
    if(TBB_FOUND)
        CMP_COPY_DEPENDENT_LIBRARIES("tbb;tbb_malloc")
        # Append the locations of the TBB libraries to our Library Search Paths
        list(APPEND CMP_LIB_SEARCH_DIRS ${TBB_BINARY_DIR} ${TBB_LIBRARY_DIR} )
        # Create an install rule to copy the dependent shared library over to the installed location
        CMP_LIBRARIES_INSTALL_RULES("tbb;tbb_malloc" ".")
    else()
        message(FATAL_ERROR "The Intel Threading Building Blocks library is needed to enable the multithreaded algorithms. Please make sure it is installed properly")
    endif()
    set(SIMPL_USE_PARALLEL_ALGORITHMS "1")
endif()

# --------------------------------------------------------------------
# SIMPL needs the Eigen library for Least Squares fit and Eigen
# value/vector calculations.
set(SIMPL_USE_EIGEN "")
Find_Package(Eigen REQUIRED)
if(EIGEN_FOUND)
  message(STATUS "Found Eigen Library")
  set(SIMPL_USE_EIGEN "1")
else()
  message(WARNING "The Eigen Library is required for some algorithms to execute. These algorithms will be disabled.")
endif()


# --------------------------------------------------------------------
# Find and Use the Qt5 Libraries
include(${CMP_SOURCE_DIR}/ExtLib/Qt5Support.cmake)
set(SIMPLView_Qt5_Components Core Widgets Network Gui Concurrent Svg Xml OpenGL PrintSupport )
CMP_AddQt5Support( "${SIMPLView_Qt5_Components}"
                    "${SIMPL_USE_QtWebEngine}"
                    "${SIMPLViewProj_BINARY_DIR}"
                    "SIMPLView")

# --------------------------------------------------------------------
# Now that we have found all of our required libraries and packages we
# we can start finding targets to compile

# We do NOT want to install any of the headers
set(SIMPLViewProj_INSTALL_FILES "${PROJECT_INSTALL_HEADERS}")

# --------------------------------------------------------------------
# Generate our ReadMe and License Files
configure_file(${PROJECT_RESOURCES_DIR}/SIMPLView/SIMPLViewLicense.txt
                ${SIMPLViewProj_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt )

# --------------------------------------------------------------------
# Generate install rules for the text files
if(APPLE)
  install(FILES ${SIMPLViewProj_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt 
          DESTINATION "${DREAM3D_PACKAGE_DEST_PREFIX}/Resources"
          COMPONENT Applications)
else()
  install(FILES ${SIMPLViewProj_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt 
          DESTINATION "${DREAM3D_PACKAGE_DEST_PREFIX}"
          COMPONENT Applications)
endif()

#------------------------------------------------------------------------------
# If we have not defined a "BrandedSIMPLView_DIR" then define it to the default
# generic set of ICONs
if(NOT DEFINED BrandedSIMPLView_DIR)
  set(BrandedSIMPLView_DIR ${SIMPLViewProj_SOURCE_DIR}/Resources/OpenSourceEdition)
endif()

# -----------------------------------------------------------------------
# This needs to be set here as we are going to look for files in this directory
# -----------------------------------------------------------------------
if("${FilterWidgetsLib_BINARY_DIR}" STREQUAL "")
  set(FilterWidgetsLib_BINARY_DIR ${SIMPLViewProj_BINARY_DIR}/FilterWidgetsLib)
endif()

# --------------------------------------------------------------------
#- Add in the Main SIMPLView Application
add_subdirectory( ${SIMPLViewProj_SOURCE_DIR}/Source/SIMPLView ${PROJECT_BINARY_DIR}/SIMPLView)

# --------------------------------------------------------------------
#- Add in the Main SIMPLView Application
option(SIMPLView_BUILD_DevHelper "Build the DevHelper Application" OFF)
if(SIMPLView_BUILD_DevHelper)
  add_subdirectory( ${SIMPLViewProj_SOURCE_DIR}/Source/DevHelper  ${PROJECT_BINARY_DIR}/DevHelper)
endif()

set(SIMPView_DOCS_ROOT_DIR "${SIMPLViewProj_BINARY_DIR}/Bin/Help/SIMPLView")

# add the Command line utilities
# Figure out here if we are going to build the command line tools
option(SIMPLView_BUILD_CLI_TOOLS "Build the command line versions of the applications" OFF)
if( SIMPLView_BUILD_CLI_TOOLS )
    add_subdirectory(${SIMPLViewProj_SOURCE_DIR}/Tools ${SIMPLViewProj_BINARY_DIR}/Tools)
endif()

# This should be the last line in this file:
include(${PROJECT_RESOURCES_DIR}/CPack/PackageProject.cmake)

# ------- Enable the CTest testing. Use make test to run all tests ---------------
if(SIMPL_BUILD_TESTING)
    include(${SIMPLViewProj_SOURCE_DIR}/Test/CMakeLists.txt)
endif()
