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


include_directories(${HDF5_INCLUDE_DIR}
                    ${QT_INCLUDE_DIR}/..
                    ${SIMPLView_SOURCE_DIR}
                    ${SIMPLView_BINARY_DIR}
                    ${SIMPLView_BINARY_DIR}/SIMPLView
                    ${PROJECT_CODE_DIR}
                    ${SIMPLViewProj_BINARY_DIR}/Applications
                    ${SIMPLProj_SOURCE_DIR}/Source
                    ${SIMPLProj_BINARY_DIR}
                    
                    ${SIMPLViewProj_SOURCE_DIR}/Source
										${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib
                    ${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib/Core
                    ${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib/Dialogs
                    ${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib/FilterParameterWidgets
                    ${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib/QtSupport
                    ${SIMPLProj_SOURCE_DIR}/Source/SVWidgetsLib/Widgets

										${SIMPLProj_BINARY_DIR}/SVWidgetsLib
                    ${SIMPLProj_BINARY_DIR}/SVWidgetsLib/Core
                    ${SIMPLProj_BINARY_DIR}/SVWidgetsLib/Dialogs
                    ${SIMPLProj_BINARY_DIR}/SVWidgetsLib/FilterParameterWidgets
                    ${SIMPLProj_BINARY_DIR}/SVWidgetsLib/QtSupport
                    ${SIMPLProj_BINARY_DIR}/SVWidgetsLib/Widgets
)


#------------------------------------------------------------------------------
# Include this file from the CMP Project
include(${CMP_SOURCE_DIR}/cmpCMakeMacros.cmake)



set(TEST_TEMP_DIR ${SIMPLViewTest_BINARY_DIR}/Temp)
# Make sure the directory is created during CMake time
file(MAKE_DIRECTORY ${TEST_TEMP_DIR})


configure_file(${SIMPLViewTest_SOURCE_DIR}/TestFileLocations.h.in
               ${SIMPLViewTest_BINARY_DIR}/SIMPLViewTestFileLocations.h @ONLY IMMEDIATE)


configure_file(${SIMPLProj_SOURCE_DIR}/Resources/UnitTestSupport.hpp
          ${SIMPLViewTest_BINARY_DIR}/UnitTestSupport.hpp COPYONLY IMMEDIATE)


#------------------------------------------------------------------------------
# Example Test code
# AddSIMPLUnitTest(TESTNAME ConditionalSetValueTest
#   SOURCES ${SIMPLViewTest_SOURCE_DIR}/ConditionalSetValueTest.cpp
#   FOLDER "SIMPLibProj/Test"
#   LINK_LIBRARIES Qt5::Core H5Support SIMPLib)



