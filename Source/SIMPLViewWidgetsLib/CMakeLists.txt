#--////////////////////////////////////////////////////////////////////////////
#--
#--  Copyright (c) 2012, Michael A. Jackson. BlueQuartz Software
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--
#--////////////////////////////////////////////////////////////////////////////

#///////////////////////////////////////////////////////////////////////////////
#// This code was partly written under US Air Force Contract FA8650-07-D-5800
#///////////////////////////////////////////////////////////////////////////////

project(SIMPLViewWidgetsLib VERSION 2.0)


set(SIMPLViewWidgetsLib_BUILT_AS_DYNAMIC_LIB)
if(BUILD_SHARED_LIBS)
    set(SIMPLViewWidgetsLib_BUILT_AS_DYNAMIC_LIB 1)
    if(WIN32)
        ADD_DEFINITIONS("-DSIMPLViewWidgetsLib_BUILT_AS_DYNAMIC_LIB")
    endif(WIN32)
endif(BUILD_SHARED_LIBS)

set(GENERATE_GIT_VERSION 1)

if(NOT PROJECT_RESOURCES_DIR)
set(PROJECT_RESOURCES_DIR ${SIMPLViewWidgetsLib_SOURCE_DIR}/Resources)
endif()
if(NOT DEFINED CMP_SOURCE_DIR)
set(CMP_SOURCE_DIR ${SIMPLViewWidgetsLib_SOURCE_DIR}/Support/cmp)
endif()

set(PROJECT_PREFIX "SIMPLViewWidgetsLib" CACHE STRING "The Prefix to be used for Preprocessor definitions")
set(SIMPLViewWidgetsLib_INSTALL_FILES 0)
#-------------------------------------------------------------------------------
# Include the cmp project to perform all the basic configuration tests for the
# project.
#-------------------------------------------------------------------------------
set(CMP_HEADER_DIR ${SIMPLViewWidgetsLib_BINARY_DIR})
set(CMP_CONFIGURATION_FILE_NAME "SIMPLViewWidgetsLibConfiguration.h")
set(CMP_TYPES_FILE_NAME "SIMPLViewWidgetsLibTypes.h")
set(CMP_VERSION_HEADER_FILE_NAME "SIMPLViewWidgetsLibVersion.h")
set(CMP_VERSION_SOURCE_FILE_NAME "SIMPLViewWidgetsLibVersion.cpp")
set(CMP_TOP_HEADER_FILE "SIMPLViewWidgetsLib.h")
set(CMP_PROJECT_NAMESPACE "${PROJECT_NAME}")
set(CMP_PROJECT_NAME "${PROJECT_NAME}")
set(CMP_ENABLE_PLUGINS "0")
set(CMP_LIB_SEARCH_DIRS "")
set(CMP_GENERATE_VERSION_STRING ${GENERATE_GIT_VERSION})

# Extra Variables that need to be set before all the configured files are generated.
set(SIMPLViewWidgetsLib_HDF5_SUPPORT 1)
option(SIMPLViewWidgetsLib_ENABLE_HDF5 "Enable the use of HDF5 functionality" ON)
mark_as_advanced(SIMPLViewWidgetsLib_ENABLE_HDF5)

include(${CMP_SOURCE_DIR}/cmpProject.cmake)

configure_file(${SIMPLViewWidgetsLib_SOURCE_DIR}/SIMPLViewWidgetsLibConfiguration.h.in
               ${SIMPLViewWidgetsLib_BINARY_DIR}/${CMP_TOP_HEADER_FILE})

set(Project_SRCS "")

set(${PROJECT_NAME}_Generated_HDRS
        ${CMP_HEADER_DIR}/${CMP_CONFIGURATION_FILE_NAME}
        ${CMP_HEADER_DIR}/${CMP_TYPES_FILE_NAME}
        ${CMP_HEADER_DIR}/${CMP_VERSION_HEADER_FILE_NAME}
        ${CMP_HEADER_DIR}/${CMP_TOP_HEADER_FILE}
)

set(${PROJECT_NAME}_Generated_SRCS
        ${CMP_HEADER_DIR}/${CMP_VERSION_SOURCE_FILE_NAME}
)

cmp_IDE_GENERATED_PROPERTIES("Generated" "${${PROJECT_NAME}_Generated_HDRS}" "${${PROJECT_NAME}_Generated_SRCS}")

set(SIMPLViewWidgetsLib_HDRS
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/FilterWidgetManager.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/IFilterWidgetFactory.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/PipelineFilterWidgetFactory.hpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/SIMPLViewWidgetsLibDLLExport.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/SIMPLViewWidgetsLibConstants.h
)

set(SIMPLViewWidgetsLib_MOC_HDRS
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/UpdateCheck.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/UpdateCheckData.h
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/PipelineViewPtrMimeData.h
)

set(SIMPLViewWidgetsLib_SRCS
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/FilterWidgetManager.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/IFilterWidgetFactory.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/UpdateCheck.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/UpdateCheckData.cpp
  ${SIMPLViewWidgetsLib_SOURCE_DIR}/PipelineViewPtrMimeData.cpp
)

cmp_IDE_SOURCE_PROPERTIES( "SIMPLViewWidgetsLib" "${SIMPLViewWidgetsLib_HDRS};${SIMPLViewWidgetsLib_MOC_HDRS}" "${SIMPLViewWidgetsLib_SRCS}" "0")

# --------------------------------------------------------------------
# and finally this will run moc:
QT5_WRAP_CPP( SIMPLViewWidgetsLib_Generated_MOC_SRCS ${SIMPLViewWidgetsLib_MOC_HDRS} )
set_source_files_properties( ${SIMPLViewWidgetsLib_Generated_MOC_SRCS} PROPERTIES GENERATED TRUE)
set_source_files_properties( ${SIMPLViewWidgetsLib_Generated_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)


# -- Run MOC and UIC on the necessary files
QT5_ADD_RESOURCES( SIMPLViewWidgetsLib_Generated_RC_SRCS "${SIMPLViewWidgetsLib_SOURCE_DIR}/icons/images/Icons.qrc"  )

# --------------------------------------------------------------------
# Continue on with our Qt4 section
QT5_WRAP_UI( SIMPLViewWidgetsLib_Generated_UI_HDRS   )

# --------------------------------------------------------------------
#-- Put the Qt generated files into their own group for IDEs
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Moc" "" "${SIMPLViewWidgetsLib_Generated_MOC_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Uic" "${SIMPLViewWidgetsLib_Generated_UI_HDRS}" "" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${SIMPLViewWidgetsLib_Generated_RC_SRCS}" "" "0")


# --------------------------------------------------------------------
#-- Include all the generic Widgets that can be used to build up a GUI
include(${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets/SourceList.cmake)

# --------------------------------------------------------------------
# Include all the Filter Parameter Widgets
include(${SIMPLViewWidgetsLib_SOURCE_DIR}/FilterParameterWidgets/SourceList.cmake)


set(Project_SRCS
  ${SIMPLViewWidgetsLib_HDRS}
  ${SIMPLViewWidgetsLib_MOC_HDRS}
  ${SIMPLViewWidgetsLib_SRCS}
  ${${PROJECT_NAME}_Generated_HDRS}
  ${${PROJECT_NAME}_Generated_SRCS}
  ${SIMPLViewWidgetsLib_Generated_MOC_SRCS}
  ${SIMPLViewWidgetsLib_Generated_UI_HDRS}
  ${SIMPLViewWidgetsLib_Generated_RC_SRCS}
  #---------------
  ${SIMPLViewWidgetsLib_Widgets_HDRS}
  ${SIMPLViewWidgetsLib_Widgets_MOC_HDRS}
  ${SIMPLViewWidgetsLib_Widgets_SRCS}
  ${SIMPLViewWidgetsLib_Widgets_UIS}
  ${SIMPLViewWidgetsLib_Widgets_Generated_MOC_SRCS}
  ${SIMPLViewWidgetsLib_Widgets_Generated_UI_HDRS}
  #---------------
  ${SIMPLViewWidgetsLib_ParameterWidgets_HDRS}
  ${SIMPLViewWidgetsLib_ParameterWidgets_MOC_HDRS}
  ${SIMPLViewWidgetsLib_ParameterWidgets_SRCS}
  ${SIMPLViewWidgetsLib_ParameterWidgets_UIS}
  ${SIMPLViewWidgetsLib_ParameterWidgets_Generated_MOC_SRCS}
  ${SIMPLViewWidgetsLib_ParameterWidgets_Generated_UI_HDRS}

  # -- This file has all the MOC sources in it
  ${SIMPLViewWidgetsLib_Generated_MOC_SRCS}
)



# --------------------------------------------------------------------
# Set some Include directories
include_directories(${PROJECT_CODE_DIR}
                    ${PROJECT_CODE_DIR}/QtSupportLib
                    ${QtSupportLib_BINARY_DIR}
                    ${QtSupportLib_SOURCE_DIR}
                    ${SIMPLViewWidgetsLib_SOURCE_DIR}/..
                    ${SIMPLViewWidgetsLib_SOURCE_DIR}/Widgets
                    ${SIMPLViewWidgetsLib_SOURCE_DIR}/FilterParameterWidgets
                    ${SIMPLViewWidgetsLib_BINARY_DIR}
                    ${SIMPLViewWidgetsLib_BINARY_DIR}/Widgets
                    ${SIMPLViewWidgetsLib_BINARY_DIR}/FilterParameterWidgets
                    ${SIMPLProj_SOURCE_DIR}/Source
                    ${SIMPLProj_BINARY_DIR}
  )

if(UNIX AND NOT APPLE)
    set_source_files_properties(${Project_SRCS} PROPERTIES COMPILE_FLAGS -fPIC)
endif()

add_library(SIMPLViewWidgetsLib ${LIB_TYPE} ${Project_SRCS})

set(D3D_LINK_LIBRARIES
                      QtSupportLib
                      )

target_link_libraries(SIMPLViewWidgetsLib ${D3D_LINK_LIBRARIES} )

LibraryProperties( SIMPLViewWidgetsLib ${EXE_DEBUG_EXTENSION})



set(install_dir "tools")
set(lib_install_dir "lib")
if(WIN32)
    set(install_dir ".")
    set(lib_install_dir "")
endif()


if(BUILD_SHARED_LIBS AND NOT APPLE)
    INSTALL(TARGETS ${PROJECT_NAME}
        COMPONENT Applications
        RUNTIME DESTINATION ${install_dir}
        LIBRARY DESTINATION ${lib_install_dir}
        ARCHIVE DESTINATION ${lib_install_dir}
        BUNDLE DESTINATION "."
    )
endif()

