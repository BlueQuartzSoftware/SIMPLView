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

# --------------------------------------------------------------------
# create Resource files for the various license files that are used and
# also create a header file that lists all the License Files
set(LICENSE_FILES
                   ${PROJECT_RESOURCES_DIR}/ThirdParty/Boost.txt
                   ${PROJECT_RESOURCES_DIR}/ThirdParty/Qt.txt
                   ${PROJECT_RESOURCES_DIR}/ThirdParty/HDF5.txt
           )
set(SIMPLView_LICENSE_FILE ${PROJECT_RESOURCES_DIR}/SIMPLView/SIMPLViewLicense.txt)
set(QRC_LICENSE_FILES "")
set(LICENSE_HEADER_FILE  ${PROJECT_BINARY_DIR}/SIMPLView/License/${PROJECT_NAME}LicenseFiles.h_tmp)
file(WRITE ${LICENSE_HEADER_FILE} "#ifndef _LICENSE_FILES_H_\n")
file(APPEND ${LICENSE_HEADER_FILE} "#define _LICENSE_FILES_H_\n")
file(APPEND ${LICENSE_HEADER_FILE} "namespace ${PROJECT_PREFIX} {\n")
file(APPEND ${LICENSE_HEADER_FILE} "  QStringList LicenseList = (QStringList()  ")

set(THIRDPARTY_QRC_CONTENTS "<!DOCTYPE RCC>\n<RCC version=\"1.0\">\n<qresource>\n")

foreach(lf ${LICENSE_FILES})
  string(CONCAT THIRDPARTY_QRC_CONTENTS ${THIRDPARTY_QRC_CONTENTS} "<file>")
    get_filename_component(cmp_text_file_name ${lf} NAME_WE)

    set(cmp_text_file_name "ThirdParty/${cmp_text_file_name}.txt")
    string(CONCAT THIRDPARTY_QRC_CONTENTS ${THIRDPARTY_QRC_CONTENTS} ${cmp_text_file_name})

    get_filename_component(lf_fn ${lf} NAME_WE)
    # Copy the text file into the Build Directory
#    message(STATUS "lf: ${lf}")
#    message(STATUS "cmp_text_file_name: ${cmp_text_file_name}")
#    message(STATUS "lf_fn: ${lf_fn}")
    configure_file("${lf}" ${PROJECT_BINARY_DIR}/ThirdParty/${lf_fn}.txt   COPYONLY )

    # create the Qt Resource File
    set(CMP_RESOURCE_FILE_NAME ${lf_fn}.txt)
    
    file(APPEND ${LICENSE_HEADER_FILE} " << \":/ThirdParty/${lf_fn}.txt\"")
  string(CONCAT THIRDPARTY_QRC_CONTENTS ${THIRDPARTY_QRC_CONTENTS} "</file>\n")
endforeach(lf ${LICENSE_FILES})

string(CONCAT THIRDPARTY_QRC_CONTENTS ${THIRDPARTY_QRC_CONTENTS} "</qresource>\n</RCC>")
set(cmp_contents "${THIRDPARTY_QRC_CONTENTS}")
configure_file(${CMP_CONFIGURED_FILES_SOURCE_DIR}/QtResourceFile-All.qrc.in
                   ${PROJECT_RESOURCES_DIR}/ThirdParty.qrc)

set(QRC_LICENSE_FILES ${QRC_LICENSE_FILES} ${PROJECT_RESOURCES_DIR}/ThirdParty.qrc)

set(cmp_text_file_name "SIMPLView/SIMPLViewLicense.txt")

configure_file("${SIMPLView_LICENSE_FILE}" ${PROJECT_BINARY_DIR}/SIMPLView/SIMPLViewLicense.txt   COPYONLY )

# create the Qt Resource File
configure_file(${CMP_CONFIGURED_FILES_SOURCE_DIR}/QtResourceFile.qrc.in ${PROJECT_RESOURCES_DIR}/SIMPLView.qrc)

set(QRC_LICENSE_FILES ${QRC_LICENSE_FILES} ${PROJECT_RESOURCES_DIR}/SIMPLView.qrc)
file(APPEND ${LICENSE_HEADER_FILE} " << \":/SIMPLView/SIMPLViewLicense.txt\"")

cmp_IDE_GENERATED_PROPERTIES("Generated/qrc" "${QRC_LICENSE_FILES}" "")

file(APPEND ${LICENSE_HEADER_FILE}  ");\n")
file(APPEND ${LICENSE_HEADER_FILE}  "}\n#endif /* _LICENSE_FILES_H_ */ \n")

cmpReplaceFileIfDifferent(OLD_FILE_PATH ${PROJECT_BINARY_DIR}/SIMPLView/License/${PROJECT_NAME}LicenseFiles.h
                          NEW_FILE_PATH ${LICENSE_HEADER_FILE} )
