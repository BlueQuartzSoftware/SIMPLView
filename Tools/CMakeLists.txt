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
# set project's name
PROJECT( SIMPLViewTools )
cmake_minimum_required(VERSION 3.1.0)

# --------------------------------------------------------------------
# Setup the install rules for the various platforms
set(install_dir "bin")
if(WIN32)
    set(install_dir ".")
endif()

# --------------------------------------------------------------------------
macro(ConvertPathToHeaderCompatible INPUT)
    if(WIN32)
      STRING(REPLACE "\\" "\\\\" ${INPUT} ${INPUT} )
      STRING(REPLACE "/" "\\\\" ${INPUT} ${INPUT}  )
    endif()
endmacro()


set(SIMPLViewProj_SOURCE_DIR_NORM ${SIMPLViewProj_SOURCE_DIR})
ConvertPathToHeaderCompatible(${SIMPLViewProj_SOURCE_DIR_NORM})
set(SIMPLView_SOURCE_DIR_NORM ${SIMPLView_SOURCE_DIR})
ConvertPathToHeaderCompatible(${SIMPLView_SOURCE_DIR_NORM})
set(SIMPLView_PLUGIN_DIR_NORM "${SIMPLViewProj_SOURCE_DIR}/Source/Plugins")
ConvertPathToHeaderCompatible(${SIMPLView_PLUGIN_DIR_NORM})

get_filename_component(SIMPLViewProj_PARENT_DIR_NORM ${SIMPLViewProj_SOURCE_DIR} PATH)
ConvertPathToHeaderCompatible(${SIMPLViewProj_PARENT_DIR_NORM})


configure_file( ${SIMPLViewTools_SOURCE_DIR}/ToolConfiguration.h.in
                ${SIMPLViewTools_BINARY_DIR}/ToolConfiguration.h)

#-------------------------------------------------------------------------------
# Function COMPILE_TOOL to help alleviate lots of extra code below for adding
# simple command line tools that just need one or two source files
#
function(COMPILE_TOOL)
    set(options)
    set(oneValueArgs TARGET DEBUG_EXTENSION BINARY_DIR COMPONENT INSTALL_DEST DEFINITION)
    set(multiValueArgs SOURCES LINK_LIBRARIES)
    cmake_parse_arguments(D3DTOOL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if( ${D3DTOOL_DEFINITION} )
        add_definitions(-D${DEFINITION})
    endif()

    BuildToolBundle(
        TARGET ${D3DTOOL_TARGET}
        SOURCES ${D3DTOOL_SOURCES}
        DEBUG_EXTENSION ${D3DTOOL_DEBUG_EXTENSION}
        VERSION_MAJOR ${D3DTOOL_SIMPL_VER_MAJOR}
        VERSION_MINOR ${D3DTOOL_SIMPL_VER_MINOR}
        VERSION_PATCH ${D3DTOOL_SIMPL_VER_PATCH}
        BINARY_DIR    ${D3DTOOL_BINARY_DIR}
        LINK_LIBRARIES Qt5::Core "${D3DTOOL_LINK_LIBRARIES}"
        LIB_SEARCH_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMPONENT     ${D3DTOOL_COMPONENT}
        INSTALL_DEST  "${D3DTOOL_INSTALL_DEST}"
        SOLUTION_FOLDER "Applications"
    )

endfunction()


