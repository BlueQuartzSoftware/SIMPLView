

set(SVWidgetsLib_Workspace_HDRS

)

set(SVWidgetsLib_Workspace_MOC_HDRS
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/FilterInputWidget.h
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/PipelineFilterWidget.h
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/PipelineViewWidget.h
)

set(SVWidgetsLib_Workspace_SRCS
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/FilterInputWidget.cpp
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/PipelineFilterWidget.cpp
  ${SVWidgetsLib_SOURCE_DIR}/Workspace/PipelineViewWidget.cpp
)

cmp_IDE_SOURCE_PROPERTIES( "SVWidgetsLib" "${SVWidgetsLib_Workspace_HDRS};${SVWidgetsLib_Workspace_MOC_HDRS}" "${SVWidgetsLib_Workspace_SRCS}" "0")

# --------------------------------------------------------------------
# and finally this will run moc:
QT5_WRAP_CPP( SVWidgetsLib_Workspace_Generated_MOC_SRCS ${SVWidgetsLib_Workspace_MOC_HDRS} )
set_source_files_properties( ${SVWidgetsLib_Workspace_Generated_MOC_SRCS} PROPERTIES GENERATED TRUE)
set_source_files_properties( ${SVWidgetsLib_Workspace_Generated_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)



# --------------------------------------------------------------------
# Continue on with our Qt4 section
QT5_WRAP_UI( SVWidgetsLib_Workspace_Generated_UI_HDRS   
  ${SVWidgetsLib_SOURCE_DIR}/Dialogs/UI_Files/AboutPlugins.ui
  ${SVWidgetsLib_SOURCE_DIR}/Dialogs/UI_Files/PluginDetails.ui
)

# --------------------------------------------------------------------
#-- Put the Qt generated files into their own group for IDEs
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Moc" "" "${SVWidgetsLib_Workspace_Generated_MOC_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Uic" "${SVWidgetsLib_Workspace_Generated_UI_HDRS}" "" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${SVWidgetsLib_Workspace_Generated_RC_SRCS}" "" "0")

set(SVWidgets_Workspace_Files
  ${SVWidgetsLib_Workspace_HDRS}
  ${SVWidgetsLib_Workspace_MOC_HDRS}
  ${SVWidgetsLib_Workspace_SRCS}
  ${SVWidgetsLib_Workspace_Generated_MOC_SRCS}
  ${SVWidgetsLib_Workspace_Generated_UI_HDRS}
  ${SVWidgetsLib_Workspace_Generated_RC_SRCS}
)
