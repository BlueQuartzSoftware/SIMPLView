#--////////////////////////////////////////////////////////////////////////////
#--
#--  Copyright (c) 2015, BlueQuartz Software
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800
#--
#--////////////////////////////////////////////////////////////////////////////

set(SIMPLViewTest_SOURCE_DIR ${SIMPLViewProj_SOURCE_DIR}/Test)
set(SIMPLViewTest_BINARY_DIR ${SIMPLViewProj_BINARY_DIR}/Test)


include_directories(${SIMPLViewTest_SOURCE_DIR})

#------------------------------------------------------------------------------
# Include this file from the SIMPL Project
include(${SIMPLProj_SOURCE_DIR}/Test/SIMPLTestFunctions.cmake)



set(TEST_TEMP_DIR ${SIMPLViewTest_BINARY_DIR}/Temp)
# Make sure the directory is created during CMake time
file(MAKE_DIRECTORY ${TEST_TEMP_DIR})


configure_file(${SIMPLViewTest_SOURCE_DIR}/TestFileLocations.h.in
               ${SIMPLViewTest_BINARY_DIR}/SIMPLViewTestFileLocations.h @ONLY IMMEDIATE)

include_directories(${SIMPLViewTest_BINARY_DIR})

#------------------------------------------------------------------------------
# Example Test code
# AddSIMPLUnitTest(TESTNAME ConditionalSetValueTest
#   SOURCES ${SIMPLViewTest_SOURCE_DIR}/ConditionalSetValueTest.cpp
#   FOLDER "SIMPLibProj/Test"
#   LINK_LIBRARIES Qt5::Core H5Support SIMPLib)



